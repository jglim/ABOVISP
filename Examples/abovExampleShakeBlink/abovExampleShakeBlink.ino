void setup() 
{
  // Set LED pins as output, switch as input with pullup
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
  pinMode(PIN_SWITCH, INPUT_PULLUP);

  // All LEDs off
  digitalWrite(PIN_LED_RED, HIGH);
  digitalWrite(PIN_LED_GREEN, HIGH);
  digitalWrite(PIN_LED_BLUE, HIGH);
}

byte redLightPattern[] =    {0, 1, 1, 0, 1, 0, 0};
byte greenLightPattern[] =  {0, 0, 1, 1, 1, 0, 1};
byte blueLightPattern[] =   {0, 0, 1, 0, 0, 1, 1};

byte colorCounter = 0;

void loop() 
{
  // Wait for sufficient movement to trigger switch (LOW)
  if (digitalRead(PIN_SWITCH) == LOW) 
  {
    // Set color based on map
    digitalWrite(PIN_LED_RED, redLightPattern[colorCounter]);
    digitalWrite(PIN_LED_GREEN, greenLightPattern[colorCounter]);
    digitalWrite(PIN_LED_BLUE, blueLightPattern[colorCounter]);
    colorCounter++;
    if (colorCounter == 7)
    {
      colorCounter = 0;
    }
    delay(20);
  }
  
  // Turn LED off
  digitalWrite(PIN_LED_RED, HIGH);
  digitalWrite(PIN_LED_GREEN, HIGH);
  digitalWrite(PIN_LED_BLUE, HIGH);
}
