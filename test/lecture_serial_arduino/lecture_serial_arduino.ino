int incomingByte = 0;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT); // Ne pas oublier de déclarer la LED en sortie !
}

void loop() {
  if (Serial.available() > 0) {
    incomingByte = Serial.read();

    // Comparaison avec les caractères ASCII '1' et '0'
    if (incomingByte == '1') {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println("LED allumee");
    } 
    else if (incomingByte == '0') {
      digitalWrite(LED_BUILTIN, LOW);
      Serial.println("LED eteinte");
    }
  }
}