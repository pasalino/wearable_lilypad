float temp;

void setup() {
  Serial.begin(57600);
  analogReference(INTERNAL1V1);
};

void loop () {
 
  temp = analogRead(0)*1.1/1024.0;
  temp = temp - 0.5;
  temp = temp / 0.01;
  Serial.println(temp);
  delay(10);
};
