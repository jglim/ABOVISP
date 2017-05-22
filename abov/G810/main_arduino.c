// #define NOPULLUPS // define NOPULLUPS if short of space and not using pullups
#define NOTIMERS // define to disable all user timers. they are quite buggy right now

// "byte" is not recognized
typedef unsigned char byte;

// booleans
#define false 0 
#define true !false

// interrupt macros

#define INT0  __attribute__ ((interrupt ("0")))
#define INT1  __attribute__ ((interrupt ("1")))
#define INT2  __attribute__ ((interrupt ("2")))
#define INT3  __attribute__ ((interrupt ("3")))
#define INT4  __attribute__ ((interrupt ("4")))
#define INT5  __attribute__ ((interrupt ("5")))
#define INT6  __attribute__ ((interrupt ("6")))
#define INT7  __attribute__ ((interrupt ("7")))
#define INT8  __attribute__ ((interrupt ("8")))
#define INT9  __attribute__ ((interrupt ("9")))
#define INT10 __attribute__ ((interrupt ("10")))
#define INT11 __attribute__ ((interrupt ("11")))
#define INT12 __attribute__ ((interrupt ("12")))
#define INT13 __attribute__ ((interrupt ("13")))
#define INT14 __attribute__ ((interrupt ("14")))
#define INT15 __attribute__ ((interrupt ("15")))


// ABOV headers
#define HMS800_ENABLE_INTERRUPT   asm("EI");
#define HMS800_DISABLE_INTERRUPT  asm("DI");
#define HMS800_NOP          asm("NOP");
#define HMS800_STOP         asm("STOP");


// special function registers

sfr     T0SCR     = 0xB0;
sfr     T0DR      = 0xB1;
sfr     T0CR      = 0xB2;
sfr     T1SCR     = 0xB3;
sfr     T1DR      = 0xB4;
sfr     T1CR      = 0xB5;
sfr     T2SCR     = 0xB6;
sfr     T2DR      = 0xB7;
sfr     T2CR      = 0xB8;
sfr     ADMR      = 0xBD;
sfr     ADDRH     = 0xBE;
sfr     ADDRL     = 0xBF;
sfr     R0        = 0xC0;
sfr     R1        = 0xC1;
sfr     R3        = 0xC3;
sfr     R0CONH    = 0xC6;
sfr     R0CONM    = 0xC7;
sfr     R0CONL    = 0xC8;
sfr     PUR0      = 0xC9;
sfr     EINT0H    = 0xCA;
sfr     EINT0L    = 0xCB;
sfr     ERQ0      = 0xCC;
sfr     EINTF     = 0xCD;
sfr     PWMSCR    = 0xCE;
sfr     PWMPDR    = 0xCF;
sfr     PWM2DR    = 0xD0;
sfr     PWM3DR    = 0xD1;
sfr     R1CONH    = 0xD3;
sfr     R1CONM    = 0xD4;
sfr     R1CONL    = 0xD5;
sfr     PUR1      = 0xD6;
sfr     EINT1     = 0xD7;
sfr     ERQ1      = 0xD8;
sfr     R3CONH    = 0xDC;
sfr     R3CONL    = 0xDD;
sfr     RPR       = 0xE1;
sfr     BUZR      = 0xE5;
sfr     BUPDR     = 0xE6;
sfr     SIOCR     = 0xE7;
sfr     SIODAT    = 0xE8;
sfr     SIOPS     = 0xE9;
sfr     IENH      = 0xEA;
sfr     IENL      = 0xEB;
sfr     IRQH      = 0xEC;
sfr     IRQL      = 0xED;
sfr     INTFH     = 0xEE;
sfr     BTCR      = 0xF1;
sfr     CKCTLR    = 0xF2;
sfr     PORC      = 0xF3;
sfr     WDTR      = 0xF4;
sfr     SSCR      = 0xF5;
sfr     WDTSR     = 0xF6;
sfr     WDTCR     = 0xF7;

// arduino compatibility

#define HIGH 0x01
#define LOW 0x00

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105
#define EULER 2.718281828459045235360287471352

#define SERIAL  0x0
#define DISPLAY 0x1

#define LSBFIRST 0
#define MSBFIRST 1

#define CHANGE 1
#define FALLING 2
#define RISING 3

#define PIN_R04 1
#define PIN_R05 2
#define PIN_R06 3
#define PIN_R07 4
#define PIN_R10 6
#define PIN_R11 7
#define PIN_R12 8
#define PIN_R33 9
#define PIN_R34 10
#define PIN_R35 11
#define PIN_R00 13
#define PIN_R01 14
#define PIN_R02 15
#define PIN_R03 16


// interrupt controls arduino-style

void sei()
{
  asm("EI");
}

void cli()
{
  asm("DI");
}

// bit manipulation helpers

byte byteSet(byte value, byte bits)
{
  return value | bits;
}

byte byteClear(byte value, byte bits)
{
  return value & (~bits);
}

byte byteWrite(byte value, byte bits, byte mask)
{
  return byteSet(byteClear(value, mask), bits);
}

byte byteAnd(byte value, byte mask)
{
  return value & mask;
}

/*
// this breaks the compiler?
byte bitRead(byte value, byte bitIndex)
{
  return (value & (1 << bitIndex)) >> bitIndex;
}
*/


// gpio



#define PIN_LED_RED        PIN_R03
#define PIN_LED_GREEN      PIN_R11
#define PIN_LED_BLUE       PIN_R12
#define PIN_IR             PIN_R10
#define PIN_SWITCH         PIN_R02

void pinMode(byte pinName, byte desiredPinMode)
{
  if (desiredPinMode == OUTPUT)
  {
    // output usually depends on the MSB of the mask being 1
    if        (pinName == PIN_R04) { R0CONM = byteSet(R0CONM, 0x38); }
    else if   (pinName == PIN_R05) { R0CONM = byteSet(R0CONM, 0xC0); R0CONH = byteSet(R0CONH, 0x01); }
    else if   (pinName == PIN_R06) { R0CONH = byteSet(R0CONH, 0x1C); }
    else if   (pinName == PIN_R07) { R0CONH = byteSet(R0CONH, 0xE0); }
    else if   (pinName == PIN_R10) { R1CONL = byteSet(R1CONL, 0x03); }
    else if   (pinName == PIN_R11) { R1CONL = byteSet(R1CONL, 0x1C); }
    else if   (pinName == PIN_R12) { R1CONM = byteSet(R1CONM, 0x1C); }
    else if   (pinName == PIN_R33) { R3CONH = byteSet(R3CONH, 0x03); }
    else if   (pinName == PIN_R34) { R3CONH = byteSet(R3CONH, 0x0C); }
    else if   (pinName == PIN_R35) { R3CONH = byteSet(R3CONH, 0x30); } // invalid (reset)
    else if   (pinName == PIN_R00) { R0CONL = byteSet(R0CONL, 0x03); }
    else if   (pinName == PIN_R01) { R0CONL = byteSet(R0CONL, 0x0C); }
    else if   (pinName == PIN_R02) { R0CONL = byteSet(R0CONL, 0x30); }
    else if   (pinName == PIN_R03) { R0CONM = byteSet(R0CONM, 0x07); }

  }
  else if ((desiredPinMode == INPUT) || (desiredPinMode == INPUT_PULLUP))
  {
    // inputs (push/pull) depends on masked bits being 0
    if        (pinName == PIN_R04) { R0CONM = byteClear(R0CONM, 0x38); }
    else if   (pinName == PIN_R05) { R0CONM = byteClear(R0CONM, 0xC0); R0CONH = byteClear(R0CONH, 0x01); }
    else if   (pinName == PIN_R06) { R0CONH = byteClear(R0CONH, 0x1C); }
    else if   (pinName == PIN_R07) { R0CONH = byteClear(R0CONH, 0xE0); }
    else if   (pinName == PIN_R10) { R1CONL = byteClear(R1CONL, 0x03); }
    else if   (pinName == PIN_R11) { R1CONL = byteClear(R1CONL, 0x1C); }
    else if   (pinName == PIN_R12) { R1CONM = byteClear(R1CONM, 0x1C); }
    else if   (pinName == PIN_R33) { R3CONH = byteClear(R3CONH, 0x03); }
    else if   (pinName == PIN_R34) { R3CONH = byteClear(R3CONH, 0x0C); }
    else if   (pinName == PIN_R35) { R3CONH = byteClear(R3CONH, 0x30); }
    else if   (pinName == PIN_R00) { R0CONL = byteClear(R0CONL, 0x03); }
    else if   (pinName == PIN_R01) { R0CONL = byteClear(R0CONL, 0x0C); }
    else if   (pinName == PIN_R02) { R0CONL = byteClear(R0CONL, 0x30); }
    else if   (pinName == PIN_R03) { R0CONM = byteClear(R0CONM, 0x07); }

    #ifndef NOPULLUPS
    if (desiredPinMode == INPUT_PULLUP)
    {        
        if        (pinName == PIN_R04) { PUR0 = byteSet(PUR0, 0x10); }
        else if   (pinName == PIN_R05) { PUR0 = byteSet(PUR0, 0x20); }
        else if   (pinName == PIN_R06) { PUR0 = byteSet(PUR0, 0x40); }
        else if   (pinName == PIN_R07) { PUR0 = byteSet(PUR0, 0x80); }
        else if   (pinName == PIN_R10) { PUR1 = byteSet(PUR1, 0x01); }
        else if   (pinName == PIN_R11) { PUR1 = byteSet(PUR1, 0x02); }
        else if   (pinName == PIN_R12) { PUR1 = byteSet(PUR1, 0x04); }
        else if   (pinName == PIN_R00) { PUR0 = byteSet(PUR0, 0x01); }
        else if   (pinName == PIN_R01) { PUR0 = byteSet(PUR0, 0x02); }
        else if   (pinName == PIN_R02) { PUR0 = byteSet(PUR0, 0x04); }
        else if   (pinName == PIN_R03) { PUR0 = byteSet(PUR0, 0x08); }
    }
    else 
    {
      if        (pinName == PIN_R04) { PUR0 = byteClear(PUR0, 0x10); }
      else if   (pinName == PIN_R05) { PUR0 = byteClear(PUR0, 0x20); }
      else if   (pinName == PIN_R06) { PUR0 = byteClear(PUR0, 0x40); }
      else if   (pinName == PIN_R07) { PUR0 = byteClear(PUR0, 0x80); }
      else if   (pinName == PIN_R10) { PUR1 = byteClear(PUR1, 0x01); }
      else if   (pinName == PIN_R11) { PUR1 = byteClear(PUR1, 0x02); }
      else if   (pinName == PIN_R12) { PUR1 = byteClear(PUR1, 0x04); }
      else if   (pinName == PIN_R00) { PUR0 = byteClear(PUR0, 0x01); }
      else if   (pinName == PIN_R01) { PUR0 = byteClear(PUR0, 0x02); }
      else if   (pinName == PIN_R02) { PUR0 = byteClear(PUR0, 0x04); }
      else if   (pinName == PIN_R03) { PUR0 = byteClear(PUR0, 0x08); }
    }
    #endif

  }
}

void digitalWrite(byte pinName, byte pinState)
{

  if (pinState == LOW)
  {
    if        (pinName == PIN_R04) { R0 = byteClear(R0, 0x10); }
    else if   (pinName == PIN_R05) { R0 = byteClear(R0, 0x20); }
    else if   (pinName == PIN_R06) { R0 = byteClear(R0, 0x40); }
    else if   (pinName == PIN_R07) { R0 = byteClear(R0, 0x80); }
    else if   (pinName == PIN_R10) { R1 = byteClear(R1, 0x01); }
    else if   (pinName == PIN_R11) { R1 = byteClear(R1, 0x02); }
    else if   (pinName == PIN_R12) { R1 = byteClear(R1, 0x04); }
    else if   (pinName == PIN_R33) { R3 = byteClear(R3, 0x08); }
    else if   (pinName == PIN_R34) { R3 = byteClear(R3, 0x10); }
    else if   (pinName == PIN_R35) { R3 = byteClear(R3, 0x20); } // invalid (reset)
    else if   (pinName == PIN_R00) { R0 = byteClear(R0, 0x01); }
    else if   (pinName == PIN_R01) { R0 = byteClear(R0, 0x02); }
    else if   (pinName == PIN_R02) { R0 = byteClear(R0, 0x04); }
    else if   (pinName == PIN_R03) { R0 = byteClear(R0, 0x08); }
  }
  else if (pinState == HIGH)
  {
    if        (pinName == PIN_R04) { R0 = byteSet(R0, 0x10); }
    else if   (pinName == PIN_R05) { R0 = byteSet(R0, 0x20); }
    else if   (pinName == PIN_R06) { R0 = byteSet(R0, 0x40); }
    else if   (pinName == PIN_R07) { R0 = byteSet(R0, 0x80); }
    else if   (pinName == PIN_R10) { R1 = byteSet(R1, 0x01); }
    else if   (pinName == PIN_R11) { R1 = byteSet(R1, 0x02); }
    else if   (pinName == PIN_R12) { R1 = byteSet(R1, 0x04); }
    else if   (pinName == PIN_R33) { R3 = byteSet(R3, 0x08); }
    else if   (pinName == PIN_R34) { R3 = byteSet(R3, 0x10); }
    else if   (pinName == PIN_R35) { R3 = byteSet(R3, 0x20); } // invalid (reset)
    else if   (pinName == PIN_R00) { R0 = byteSet(R0, 0x01); }
    else if   (pinName == PIN_R01) { R0 = byteSet(R0, 0x02); }
    else if   (pinName == PIN_R02) { R0 = byteSet(R0, 0x04); }
    else if   (pinName == PIN_R03) { R0 = byteSet(R0, 0x08); }
  }
}

byte digitalRead(byte pinName)
{
  byte result = 0;
  if        (pinName == PIN_R04) { result = byteAnd(R0, 0x10); }
  else if   (pinName == PIN_R05) { result = byteAnd(R0, 0x20); }
  else if   (pinName == PIN_R06) { result = byteAnd(R0, 0x40); }
  else if   (pinName == PIN_R07) { result = byteAnd(R0, 0x80); }
  else if   (pinName == PIN_R10) { result = byteAnd(R1, 0x01); }
  else if   (pinName == PIN_R11) { result = byteAnd(R1, 0x02); }
  else if   (pinName == PIN_R12) { result = byteAnd(R1, 0x04); }
  else if   (pinName == PIN_R33) { result = byteAnd(R3, 0x08); }
  else if   (pinName == PIN_R34) { result = byteAnd(R3, 0x10); }
  else if   (pinName == PIN_R35) { result = byteAnd(R3, 0x20); }
  else if   (pinName == PIN_R00) { result = byteAnd(R0, 0x01); }
  else if   (pinName == PIN_R01) { result = byteAnd(R0, 0x02); }
  else if   (pinName == PIN_R02) { result = byteAnd(R0, 0x04); }
  else if   (pinName == PIN_R03) { result = byteAnd(R0, 0x08); }
  if (result != 0)
  {
    return HIGH;
  }
  return LOW;
}


// delays


byte waste_i = 0;
byte waste_j = 0;

void wasteTime()
{
  for (waste_i = 0; waste_i < 0xFF; waste_i++)
  {
    for (waste_j = 0; waste_j < 0xFF; waste_j++)
    {
      asm("NOP");
    }
  }
}

byte innerWasteLoop = 0;

void delayMicroseconds(unsigned char duration)
{
  // 8mhz: 1 us = 8 cycles

  // resolution is ???us - any lower and assembly would be preferable
  // todo: calibrate

  // duration = duration / 4; // division and move
  while (duration > 0) // comparison and jumps
  {
    duration--; // decrement can be 2 - 5 cycles

      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");

    /*
    for (innerWasteLoop = 0; innerWasteLoop < 0x4; innerWasteLoop++)
    {
      asm("NOP");
    }
    */
  }
}

// delay (in milliseconds) - more forgiving i think
void delay(unsigned char duration)
{
  while (duration > 0)
  {
    duration--;
    delayMicroseconds(250);
    delayMicroseconds(250);
    delayMicroseconds(250);
    delayMicroseconds(250);
  }
}

void disableWatchdogTimer()
{
  // Disable WDT (CKCTLR 0x00F2, disable this bit 0b00010000)
  CKCTLR &= ~0x10;
  WDTR  = 0xFF; // clear the WDTR bit anyway in case
}


// timers

#define HOW_MANY_OF_THESE_TO_A_MILLISECOND 40


#ifndef NOTIMERS

static byte microsValue = 0;
static byte millisValue = 0;
static byte numberOfBytesBeforeIncrementingMillis = HOW_MANY_OF_THESE_TO_A_MILLISECOND; // WAS 384

static byte pwmPhase = 0;

static byte redDuty = 0x00;
static byte redDutyIsIncrementing = 1;
static byte greenDuty = 0x00;
static byte greenDutyIsIncrementing = 1;
static byte blueDuty = 0x00;
static byte blueDutyIsIncrementing = 1;

static byte pwmIsEnabled = 0;

unsigned int micros()
{
  return microsValue;
}

// this WILL drift and desynchronize from micros;
unsigned int millis()
{
  return millisValue;
}


void setupTimer2()
{
  T2SCR = 0x09; 
  T2DR = 0xFF;
  IENH = 0x08;
  asm("EI");
}

void INT3 TMR2_INT(void)
{

  microsValue++;
  numberOfBytesBeforeIncrementingMillis--;
  if (numberOfBytesBeforeIncrementingMillis == 0)
  {
    numberOfBytesBeforeIncrementingMillis = HOW_MANY_OF_THESE_TO_A_MILLISECOND;
    millisValue++;

  }

  if (pwmIsEnabled)
  {
    if (pwmPhase > redDuty)     { R0 |= 0x08; } else { R0 &= 0xF7; }
    if (pwmPhase > greenDuty)   { R1 |= 0x02; } else { R1 &= 0xFD; }
    if (pwmPhase > blueDuty)    { R1 |= 0x04; } else { R1 &= 0xFB; }
    pwmPhase++;  
  }
  
}

void enableLedPwm()
{
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
  pwmIsEnabled = 1;
}

void setLedColor(byte redValue, byte greenValue, byte blueValue)
{
  redDuty = redValue;
  greenDuty = greenValue;
  blueDuty = blueValue;
}

#endif

void setup();
void loop();

int main()
{

  disableWatchdogTimer();

  #ifndef NOTIMERS
    setupTimer2();
  #endif

  setup();
  
  while (1)
  {
    loop();
  }
  return 0;
}

// setup() and loop() don't exist yet and will be appended below before compiling

/*
// example: blink 

void setup()
{

  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
}

void loop()
{

    // red
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_LED_GREEN, HIGH);
    digitalWrite(PIN_LED_BLUE, HIGH);
    wasteTime();
    // green
    digitalWrite(PIN_LED_RED, HIGH);
    digitalWrite(PIN_LED_GREEN, LOW);
    wasteTime();
    // blue
    digitalWrite(PIN_LED_GREEN, HIGH);
    digitalWrite(PIN_LED_BLUE, LOW);
    wasteTime();wasteTime();
}
*/