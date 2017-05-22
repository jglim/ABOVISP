void setup() 
{
  // Set LED pins as output, infrared pin as input
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_BLUE, OUTPUT);
  pinMode(PIN_IR, INPUT);

  // All off
  digitalWrite(PIN_LED_RED,   HIGH);
  digitalWrite(PIN_LED_GREEN, HIGH);
  digitalWrite(PIN_LED_BLUE,  HIGH);
}

void loop() 
{
  // When IR pin is active (active LOW), turn on red LED (active LOW)
  digitalWrite(PIN_LED_RED, digitalRead(PIN_IR));
}
