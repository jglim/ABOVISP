#include <digitalWriteFast.h>

#define ABOV_SDAT_PIN 9
#define ABOV_SCLK_PIN 8
#define ABOV_VDD_PIN 7
#define ABOV_VPP_PIN 6
#define ABOV_REG_PIN 5 // connected to boost regulator enable pin

#define MAX_FLASH_SIZE 4096;

#define FASTINTERVAL 5


byte inputBuffer = 0;
bool msbHasBeenReceived = false;

// 0: not listening, 1: load flash, 2: load boot
int entryMode = 0;

bool powerToggleIsOn = false;

byte trimC0 = 0x00;
byte trimC1 = 0x00;
byte trimC2 = 0x00;
byte trimC3 = 0x00;

byte configByte = 0x24; // 8MHz, 2.7V LVR, Unlocked

// if the erase process can't find a chip, the subsequent flash process shouldn't continue until the commands finish
byte shouldNotContinueProgramming = false;

void setupPins()
{
	pinModeFast(ABOV_SDAT_PIN, 		OUTPUT);
	pinModeFast(ABOV_SCLK_PIN, 		OUTPUT);
	pinModeFast(ABOV_VDD_PIN, 		OUTPUT);
	pinModeFast(ABOV_VPP_PIN, 		OUTPUT);
	pinModeFast(ABOV_REG_PIN, 		OUTPUT);

	digitalWriteFast(ABOV_SDAT_PIN, 	LOW);
	digitalWriteFast(ABOV_SCLK_PIN, 	LOW);
	digitalWriteFast(ABOV_VDD_PIN, 		LOW);
	digitalWriteFast(ABOV_VPP_PIN, 		HIGH);
	digitalWriteFast(ABOV_REG_PIN, 		HIGH);
}

void enterIsp()
{
	// assumes all pins are LOW

	digitalWriteFast(ABOV_VDD_PIN, HIGH);
	// delay by tVD.S (min 10ms, max 20ms)
	delay(11);
	digitalWriteFast(ABOV_VPP_PIN, LOW);
	// delay by tVP.S (min 40ms)
	delay(41);
	// after this, SCLK can rise,
	digitalWriteFast(ABOV_SCLK_PIN, HIGH);
	// delay by tBG.S (min 1us) / tVC.S (min 1us)
	delayMicroseconds(1);
	// SDAT rises
	digitalWriteFast(ABOV_SDAT_PIN, HIGH);
	// tBG.H (min 1us)
	delayMicroseconds(1);

	// turn off SCLK, then SDAT
	// arbitrary 20us * 2 "just in case" - because they ask for 40us 'stable prog'
	// this is probably a first of the dummy clocks
	delayMicroseconds(20);
	digitalWriteFast(ABOV_SCLK_PIN, LOW);
	delayMicroseconds(20);
	digitalWriteFast(ABOV_SDAT_PIN, LOW);
	delayMicroseconds(20);

}

void exitIsp()
{
	// SDAT should never be an input here bc commands end by sending a dummy byte

	// assumes all pins are HIGH
	delayMicroseconds(20); // tED.S (min 1us)
	digitalWriteFast(ABOV_SDAT_PIN, LOW);
	delayMicroseconds(1); // tDIF (min 1us) OR tVC.H (min 1us)
	digitalWriteFast(ABOV_SCLK_PIN, LOW);
	delay(41); // tVP.H (min 40ms)
	// digitalWriteFast(ABOV_VPP_PIN, HIGH);
	delay(11); // tVD.H (min 10ms)
	// digitalWriteFast(ABOV_VDD_PIN, LOW);
}

void bootNormally()
{
	// this might break existing stuff. reset device or call setupPins(), enterIsp() again after using

	delayMicroseconds(20); // tED.S (min 1us)
	digitalWriteFast(ABOV_SDAT_PIN, LOW);
	delayMicroseconds(1); // tDIF (min 1us) OR tVC.H (min 1us)
	digitalWriteFast(ABOV_SCLK_PIN, LOW);
	delay(41); // tVP.H (min 40ms)
	digitalWriteFast(ABOV_VPP_PIN, HIGH);
	delay(11); // tVD.H (min 10ms)
	digitalWriteFast(ABOV_VDD_PIN, LOW);
	delay(300);
	digitalWriteFast(ABOV_VDD_PIN, HIGH);

	pinModeFast(ABOV_SDAT_PIN, INPUT);
	pinModeFast(ABOV_SCLK_PIN, INPUT);
}

void sendByte(byte inByte)
{
	// assumes sdat & sclk are LOW

	// this is really slow but it probably conforms to the best minimum timings for safety
	// msb first!
	for (int i = 7; i >= 0; i--)
	{
		byte bitToSend = (inByte & (1 << i)) >> i;
		digitalWriteFast(ABOV_SDAT_PIN, bitToSend);
		delayMicroseconds(20);
		digitalWriteFast(ABOV_SCLK_PIN, HIGH);
		delayMicroseconds(20);
		digitalWriteFast(ABOV_SCLK_PIN, LOW);
		delayMicroseconds(20);
		digitalWriteFast(ABOV_SDAT_PIN, LOW);
		delayMicroseconds(20);
	}

	// send dummy bit
	digitalWriteFast(ABOV_SDAT_PIN, HIGH);
	delayMicroseconds(20);
	digitalWriteFast(ABOV_SCLK_PIN, HIGH);
	delayMicroseconds(20);
	digitalWriteFast(ABOV_SCLK_PIN, LOW);
	delayMicroseconds(20);
	digitalWriteFast(ABOV_SDAT_PIN, LOW);
	delayMicroseconds(20);

}

byte readByte()
{
	pinModeFast(ABOV_SDAT_PIN, INPUT);
	byte byteResult = 0;
	for (int i = 0; i < 8; i++)
	{
		digitalWriteFast(ABOV_SCLK_PIN, HIGH);
		delayMicroseconds(20);

		byteResult = byteResult << 1;
		if (digitalReadFast(ABOV_SDAT_PIN) != 0)
		{
			byteResult = byteResult | 1;
		}
		delayMicroseconds(40);
		digitalWriteFast(ABOV_SCLK_PIN, LOW);
		delayMicroseconds(20);
	}

	pinModeFast(ABOV_SDAT_PIN, OUTPUT);

	// send dummy bit
	digitalWriteFast(ABOV_SDAT_PIN, HIGH);
	delayMicroseconds(20);
	digitalWriteFast(ABOV_SCLK_PIN, HIGH);
	delayMicroseconds(20);
	digitalWriteFast(ABOV_SCLK_PIN, LOW);
	delayMicroseconds(20);
	digitalWriteFast(ABOV_SDAT_PIN, LOW);
	delayMicroseconds(20);

	return byteResult;
}

byte readByteFast()
{

	pinModeFast(ABOV_SDAT_PIN, INPUT);
	byte byteResult = 0;
	for (int i = 0; i < 8; i++)
	{
		digitalWriteFast(ABOV_SCLK_PIN, HIGH);
		delayMicroseconds(FASTINTERVAL);

		byteResult = byteResult << 1;
		if (digitalReadFast(ABOV_SDAT_PIN) != 0)
		{
			byteResult = byteResult | 1;
		}
		delayMicroseconds(FASTINTERVAL);
		digitalWriteFast(ABOV_SCLK_PIN, LOW);
		delayMicroseconds(FASTINTERVAL);
	}

	pinModeFast(ABOV_SDAT_PIN, OUTPUT);

	// send dummy bit
	digitalWriteFast(ABOV_SDAT_PIN, HIGH);
	delayMicroseconds(FASTINTERVAL);
	digitalWriteFast(ABOV_SCLK_PIN, HIGH);
	delayMicroseconds(FASTINTERVAL);
	digitalWriteFast(ABOV_SCLK_PIN, LOW);
	delayMicroseconds(FASTINTERVAL);
	digitalWriteFast(ABOV_SDAT_PIN, LOW);
	delayMicroseconds(FASTINTERVAL);

	return byteResult;
}

void sendDummyByte()
{
	// 8 + 1 dummy bit
	for (int i = 0; i < 8; i++)
	{
		digitalWriteFast(ABOV_SDAT_PIN, HIGH);
		delayMicroseconds(20);
		digitalWriteFast(ABOV_SCLK_PIN, HIGH);
		delayMicroseconds(20);
		digitalWriteFast(ABOV_SCLK_PIN, LOW);
		delayMicroseconds(20);
		// digitalWriteFast(ABOV_SDAT_PIN, LOW); // dummy: leave it HIGH
		delayMicroseconds(20);
	}

	digitalWriteFast(ABOV_SCLK_PIN, HIGH);
}

void setup()
{
	Serial.begin(9600);
	setupPins();
}

byte readByteFromAddress(byte highAddr, byte lowAddr)
{
	enterIsp();
	sendByte(0xA1);
	sendByte(highAddr);
	sendByte(lowAddr);
	byte readResult = readByte();
	sendDummyByte();
	exitIsp();
	return readResult;
}

void writeByteAtAddress(byte highAddr, byte lowAddr, byte dataToWrite)
{
	enterIsp();
	sendByte(0x41);
	sendByte(highAddr);
	sendByte(lowAddr);
	sendByte(dataToWrite);
	sendDummyByte();
	exitIsp();
}

byte programErase(byte highAddr, byte lowAddr, byte command, int waitTimeMs)
{
	enterIsp();
	sendByte(command);
	sendByte(highAddr);
	sendByte(lowAddr);
	delay(waitTimeMs);


	for (int i = 0; i < 3; i++)
	{
		// send 3 dummy bits
		digitalWriteFast(ABOV_SCLK_PIN, HIGH);
		delayMicroseconds(20);
		digitalWriteFast(ABOV_SCLK_PIN, LOW);
		delayMicroseconds(20);
	}

	pinModeFast(ABOV_SDAT_PIN, INPUT);
	byte byteResult = 0;
	for (int i = 0; i < 8; i++)
	{
		digitalWriteFast(ABOV_SCLK_PIN, HIGH);
		delayMicroseconds(20);

		byteResult = byteResult << 1;
		if (digitalReadFast(ABOV_SDAT_PIN) != 0)
		{
			//result[i] = 1;
			byteResult = byteResult | 1;
		}
		delayMicroseconds(40);
		digitalWriteFast(ABOV_SCLK_PIN, LOW);
		delayMicroseconds(20);
	}

	for (int i = 0; i < 3; i++)
	{
		// send 3 dummy bits
		digitalWriteFast(ABOV_SCLK_PIN, HIGH);
		delayMicroseconds(20);
		digitalWriteFast(ABOV_SCLK_PIN, LOW);
		delayMicroseconds(20);

		if (i == 1)
		{
			// switch SDAT to output on second dummy bit (according to datasheet)
			pinModeFast(ABOV_SDAT_PIN, OUTPUT);
		}
	}

	exitIsp();
	return byteResult;
} 

void printPaddedHex(byte printMe)
{
	if (printMe < 0x10)
	{
		Serial.print(0);
	}
	Serial.print(printMe, HEX);
}

void printDivider()
{
	Serial.println("=============================================");
}

void loop()
{
	if (Serial.available() > 0)
	{
		char inChar = Serial.read();

		// special case here to modify entry mode
		if (inChar == '}')
		{
			if (shouldNotContinueProgramming)
			{
				shouldNotContinueProgramming = false;
				return;
			}
			// exit entry mode, tidy up flashwrite
			entryMode = 0;

			// wrap up flash rw
			sendDummyByte();
			exitIsp();

			Serial.println("Exiting Flash r/w.. ");

			// Write 'smart' config
			Serial.print("Writing Configuration Byte: 0x");
			printPaddedHex(configByte);
			writeByteAtAddress(0x20, 0xC7, configByte); // 0x2c -> 0x24 or 0x6c (2c: locked, 24: unlocked)
			Serial.println(".. OK");

			// Restart board to flush changes

			Serial.println("Restarting board.. ");
			bootNormally();
			delay(500);

			// ignoring verify since i don't use it anyway..
			#ifdef VERIFY_DATA
			setupPins();

			Serial.println("OK");

			// Restart board one last time
			Serial.println("Booting normally ");
			bootNormally();
			delay(500);
			#endif


			printDivider();
			Serial.println("Flash has been written");
			printDivider();


			return;

		}
		else if (inChar == '{')
		{
			// prepare for flash
			setupPins();
			printDivider();
			Serial.println("Attempting to write flash");
			printDivider();

			// Check DeviceID
			Serial.println("Checking DeviceID");
			if (readByteFromAddress(0x21, 0x06) == 0xF0)
			{
				Serial.println("DeviceID is valid (0xF0)");
			}
			else 
			{
				Serial.println("DeviceID is not 0xF0 (MC81F4204) - exiting");
				shouldNotContinueProgramming = true;
				return;
			}

			// Save Trim values
			trimC0 = readByteFromAddress(0x20, 0xC0);
			trimC1 = readByteFromAddress(0x20, 0xC1);
			trimC2 = readByteFromAddress(0x20, 0xC2);
			trimC3 = readByteFromAddress(0x20, 0xC3);


			// Send preprogram command
			Serial.print("Sending preprogram command.. ");
			/*
				tWAIT.PRE_PGM = (Nbytes+64) x 27us + 50us
				In case of 4K Bytes ROM : tWAIT.PRE_PGM = (4096+64)x27us + 50us = 112,370us
			*/
			byte preprogramResult = programErase(0x20, 0xC0, 0xEA, 113);
			if (preprogramResult == 0x55)
			{
				Serial.println("OK");
			}
			else 
			{
				Serial.print("Preprogram failed with response 0x");
				printPaddedHex(preprogramResult);
				Serial.println(" - exiting");
				shouldNotContinueProgramming = true;
				return;
			}

			// 4 Stage Erase
			// erase1
			bool successfullyErased = false;

			while (!successfullyErased)
			{
				Serial.print("Sending Erase1 command.. ");
				for (int i = 0; i < 3; i++)
				{
					Serial.print(".");
					if (programErase(0x00, 0x00, 0x81, 3) == 0x55)
					{
						successfullyErased = true;
						Serial.println("OK");
						break;
					}
				}
				
				// attempt erase2
				if (!successfullyErased)
				{
					Serial.println();
					Serial.print("Sending Erase2 command.. ");
					for (int i = 0; i < 3; i++)
					{
						Serial.print(".");
						if (programErase(0x00, 0x00, 0x83, 5) == 0x55)
						{
							successfullyErased = true;
							Serial.println("OK");
							break;
						}
					}
					Serial.println();
				}
				
				// attempt erase3
				if (!successfullyErased)
				{
					Serial.println();
					Serial.print("Sending Erase3 command.. ");
					for (int i = 0; i < 3; i++)
					{
						Serial.print(".");
						if (programErase(0x00, 0x00, 0x85, 8) == 0x55)
						{
							successfullyErased = true;
							Serial.println("OK");
							break;
						}
					}
					Serial.println();
				}

				// attempt erase4
				if (!successfullyErased)
				{
					Serial.println();
					Serial.print("Sending Erase4 command.. ");
					for (int i = 0; i < 30; i++)
					{
						Serial.print(".");
						if (programErase(0x00, 0x00, 0x87, 15) == 0x55)
						{
							successfullyErased = true;
							Serial.println("OK");
							break;
						}
					}
					Serial.println();
				}

				if (!successfullyErased)
				{
					Serial.println("Failed to erase chip - exiting");
					shouldNotContinueProgramming = true;
					return;
				}

				// Verify that memory has been erased via chip command
				Serial.print("Running VerifyErase.. ");
				byte VerifyEraseResult = programErase(0x20, 0xC0, 0x80, 8);
				if (VerifyEraseResult != 0x55)
				{
					Serial.print("VerifyErase failed with result 0x");
					printPaddedHex(VerifyEraseResult);
					Serial.println("");
					successfullyErased = false;
				}
				else 
				{
					Serial.println("OK");
					successfullyErased = true;
				}
			}

			// Write back Trim values
			Serial.print("Restoring Trim values..");
			writeByteAtAddress(0x20, 0xC0, trimC0); // LVR Vref Trim (bits 7654: Vref Up, bits 3210 Vref Down) @ 0x00FE
			writeByteAtAddress(0x20, 0xC1, trimC1); // OSC Trim @ 0x00FF
			writeByteAtAddress(0x20, 0xC2, trimC2); // Op-amp Trim (bits 7: IDBLE, bits 6543210 Op-amp trim data) @ 0x00E2
			writeByteAtAddress(0x20, 0xC3, trimC3); // Comparator0 Trim (bits 7: IHALF, bits 654321: Comparator0 trim data) @ 0x00E3

			Serial.println("OK");
			Serial.println("Entering Flash r/w.. ");

			// subsequent loops draw data from serial and sends it straight to target
			entryMode = 3;
			
			enterIsp();
			sendByte(0x41);
			sendByte(0xF0);
			sendByte(0x00);

			return;

		}
		else if (inChar == '[')
		{
			entryMode = 4;
			return;
		}
		else if (inChar == ']')
		{
			entryMode = 0;
			return;
		}
		else if (inChar == 'p')
		{
			if (powerToggleIsOn)
			{
				digitalWriteFast(ABOV_VDD_PIN, LOW);
				powerToggleIsOn = false;
			}
			else 
			{
				digitalWriteFast(ABOV_VDD_PIN, HIGH);
				powerToggleIsOn = true;
			}
			return;
		}

		if (entryMode != 0)
		{
			byte decValue = 0;
			// lolol
			if (inChar == '0'){ decValue = 0;}
			else if (inChar == '0'){ decValue = 0;}
			else if (inChar == '1'){ decValue = 1;}
			else if (inChar == '2'){ decValue = 2;}
			else if (inChar == '3'){ decValue = 3;}
			else if (inChar == '4'){ decValue = 4;}
			else if (inChar == '5'){ decValue = 5;}
			else if (inChar == '6'){ decValue = 6;}
			else if (inChar == '7'){ decValue = 7;}
			else if (inChar == '8'){ decValue = 8;}
			else if (inChar == '9'){ decValue = 9;}
			else if (inChar == 'A'){ decValue = 10;}
			else if (inChar == 'B'){ decValue = 11;}
			else if (inChar == 'C'){ decValue = 12;}
			else if (inChar == 'D'){ decValue = 13;}
			else if (inChar == 'E'){ decValue = 14;}
			else if (inChar == 'F'){ decValue = 15;}

			if (!msbHasBeenReceived)
			{
				inputBuffer = decValue << 4;
				msbHasBeenReceived = true;
			}
			else 
			{
				inputBuffer |= decValue;
				msbHasBeenReceived = false;

				if ((entryMode == 3) && (!shouldNotContinueProgramming))
				{
					sendByte(inputBuffer);
					Serial.print("*");
				}
				if (entryMode == 4)
				{
					configByte = inputBuffer;
					Serial.print("Configuration Byte stored as 0x");
					printPaddedHex(configByte);
					Serial.println();
					entryMode = 0;
				}
			}
			return;
		}
	}
}
