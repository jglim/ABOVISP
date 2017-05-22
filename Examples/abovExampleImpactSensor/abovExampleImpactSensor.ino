void setup() 
{
  // Set LED pins as output, switch as input with pullup
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
  pinMode(PIN_SWITCH, INPUT_PULLUP);

  // Turn green LED on and keep red LED off
  digitalWrite(PIN_LED_RED, HIGH);
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_BLUE, HIGH);
}

void loop() 
{
  // Switch will remain HIGH until impact, where it momentarily goes LOW
  if (digitalRead(PIN_SWITCH) == LOW) 
  {
    // Impact has occurred, toggle to show red LED only and remain until poweroff
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_LED_GREEN, HIGH);
    digitalWrite(PIN_LED_BLUE, HIGH);
    while(1);
  }
}
