
const int CTRL_PIN = 13;
const int IN = 12;
int buttonState = 0;  
void setup() {
  // put your setup code here, to run once:
  pinMode(CTRL_PIN, OUTPUT);
  pinMode(IN, INPUT);
  digitalWrite(CTRL_PIN, HIGH);
  digitalWrite(IN, LOW);
  int buttonState = 0; 
  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
    // turn LED on:
    delay(5000);
    digitalWrite(CTRL_PIN, LOW);
    delay(3000);
    digitalWrite(CTRL_PIN, HIGH);
    delay(5000);
    return;
}
