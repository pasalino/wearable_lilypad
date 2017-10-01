
//Normalization Constants
#define ZERO -0.5
#define SLOPE 0.01
#define BUFFER_SIZE 100
#define CORRECTION 0

#define TEMP_SENSOR A2

const int numReadings = 10;
const int numReadings2 = 100;

float readings[numReadings];      // the readings from the analog input
float readings2[numReadings2];
int readIndex = 0;              // the index of the current reading
int readIndex2 = 0;
float total = 0;                  // the running total
float total2 = 0;
float average = 0;                // the average
float average2 = 0;

bool ledStatus=false;

void setup() {
  // initialize serial communication with computer:
  Serial.begin(57600);
  analogReference(INTERNAL1V1);
  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
  for (int thisReading = 0; thisReading < numReadings2; thisReading++) {
    readings2[thisReading] = 0;
  }
  //Define digital pin
  pinMode(13, OUTPUT);
}

void loop() {
  // subtract the last reading:
  total = total - readings[readIndex];
  total2 = total2 - readings2[readIndex2];
  // read from the sensor:

  float temp = readAnalogTemp();

  readings[readIndex] = temp;
  readings2[readIndex2] = temp;

  // add the reading to the total:
  total = total + readings[readIndex];
  total2 = total2 + readings2[readIndex2];
  // advance to the next position in the array:
  readIndex = readIndex + 1;
  readIndex2 = readIndex2 + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }
  if (readIndex2 >= numReadings2) {
    // ...wrap around to the beginning:
    readIndex2 = 0;
  }

  // calculate the average:
  average = total / numReadings;
  average2 = total2 / numReadings2;
  // send it to the computer as ASCII digits

  Serial.print(average);
  Serial.print("\t");
  Serial.print(average2);
  Serial.print("\t");
  Serial.print(temp);
  Serial.print("\r\n");

  blink(2000);
  delay(500);        // delay in between reads for stability
}


//Read temperature
float readAnalogTemp() {
  //Analog read
  int readValue = analogRead(TEMP_SENSOR);

  //Conversion in degrees Celsius *C
  float ref = readVcc() / 1000;
  float temp = readValue * 5 / 1024.0;
  temp = temp + ZERO;
  temp = temp / SLOPE;

  Serial.print(ledStatus);
  Serial.print("\t");
  Serial.print(ref);
  Serial.print("\t");
  Serial.print(readValue);
  Serial.print("\t");

  return temp + CORRECTION;
}



long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
  ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  ADMUX = _BV(MUX3) | _BV(MUX2);
#else
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both

  long result = (high << 8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}


void blink(int interval) {

  static unsigned long previousMillis = 0;
  static int ledState = LOW;

  if (interval == 0) {
    digitalWrite(13, HIGH);
  }

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
      ledStatus=true;
    } else {
      ledState = LOW;
      ledStatus=false;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(13, ledState);
  }
}



