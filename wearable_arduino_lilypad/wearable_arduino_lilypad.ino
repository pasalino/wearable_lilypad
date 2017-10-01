#include <EEPROM.h>

//Pins
#define TEMP_SENSOR A2
#define LED 13


//Normalization Conxsstants
#define ZERO -0.5
#define SLOPE 0.01
#define BUFFER_SIZE 200
#define CORRECTION 0

//Alarm
#define THRESHOLD 30.0
#define HIGH_THRESHOLD 34.0

#define NORMAL_RATE 2000
#define ALARM_RATE 500
#define HIGH_RATE 100

//Store Data
#define STORE_RATE_SECOND 1
#define STORE_MAX_ELEMENT 500
#define STORE_INDEX 0
#define START_STORAGE 10
#define STORAGE_INDEX_ADDR 0


//Command
#define RESET 'r'
#define START 's'
#define PAUSE 'p'
#define READ_DATA 'w'

//Flow
bool execute = true;

//Avg variables
const int avgValueNumber = BUFFER_SIZE;
float readings[avgValueNumber];
int readIndex = 0;
float total = 0;

void setup() {
  //Start serial
  Serial.begin(57600);
  

  //Define analog reference
  analogReference(INTERNAL);

  //Define digital pin
  pinMode(LED, OUTPUT);
  //digitalWrite(LED, HIGH);

  //Reset memory for avg value
  for (int thisReading = 0; thisReading < avgValueNumber; thisReading++) {
    float analogValue = readAnalogTemp();
    readings[thisReading] = analogValue;
    total = total + analogValue;
  }

}


void loop() {
  commandExecutor() ;
  float avg = readAnalogAvgTemp();

  int blink_rate = NORMAL_RATE;
  if (avg >= HIGH_THRESHOLD) {
    blink_rate = HIGH_RATE;
  } else if (avg >= THRESHOLD) {
    blink_rate = ALARM_RATE;
  }
  
  blink(blink_rate);

  if (execute) {
    storeData(avg);
  }
}


void commandExecutor() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    switch (command) {
      case START:
        execute = true;
        Serial.println("Start acquisition");
        break;
      case PAUSE:
        Serial.println("Stop acquisition");
        execute = false;
        break;
      case RESET:
        resetData();
        Serial.println("Reset acquisition");
        break;
      case READ_DATA:
        readData();
        Serial.println("Read Data");
    }
  }
}

void readData() {
  int storage_index = 0;
  EEPROM.get(STORAGE_INDEX_ADDR, storage_index);
  Serial.print("Stored: ");
  Serial.print(storage_index);
  Serial.println(" data");
  Serial.println("");
  Serial.println("--------------------------");
  for (int i = 0; i < storage_index; i++) {
    int address = (i + START_STORAGE) * sizeof(int);
    int value = 0;
    EEPROM.get(address, value);
    float float_value = (((float)value) / 100.0);

    Serial.print("DATA INDEX:");
    Serial.print(i);
    Serial.print("\tADDRESS:");
    Serial.print(address);
    Serial.print("\tVALUE: ");
    Serial.print(value);
    Serial.print("\tTEMPERATURE: ");
    Serial.print(float_value);
    Serial.println("*C");
  }
  Serial.println("--------------------------");
  Serial.println("");
}

void resetData() {
  EEPROM.put(STORAGE_INDEX_ADDR,(int) 0);
}

//Store temperature
void storeData(float data) {
  static unsigned long previousMillis = 0;
  static int interval = STORE_RATE_SECOND * 1000;

  int storage_index = 0;
  EEPROM.get(STORAGE_INDEX_ADDR, storage_index);

  //If Memory is Full
  if (storage_index > STORE_MAX_ELEMENT) {
    Serial.println("MEMORY IS FULL");
    return;
  }

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    //Add data in storage
    previousMillis = currentMillis;

    //Calculate address in eeprom
    int address = (storage_index + START_STORAGE) * sizeof(int);
    //Normalize data
    int storage_data = (int)(data * 100);
    //Storage data
    Serial.print("INDEX: ");
    Serial.print(storage_index);
    Serial.print("\tADDRESS: ");
    Serial.print(address);
    Serial.print("\tTEMPERATURE: ");
    Serial.print(data);
    Serial.print("*C");
    Serial.print("\tSTORE DATA: ");
    Serial.println(storage_data);

    EEPROM.put(address, storage_data);

    storage_index = storage_index + 1;
    EEPROM.put(STORAGE_INDEX_ADDR, storage_index);
  }

}



//Read temperature
float readAnalogTemp() {
  //Analog read
  int readValue = analogRead(TEMP_SENSOR);
  //Conversion in degrees Celsius *C
  float ref = readVcc()/1000; 
  float temp = ((float)readValue) *3*1.1/ 1024.0;
  temp = temp + ZERO;
  temp = temp / SLOPE;

  return temp + CORRECTION;
}


float readAnalogAvgTemp() {
  // subtract the last reading:
  total = total - readings[readIndex];
  float temp = readAnalogTemp();
  readings[readIndex] = temp;
  total = total + readings[readIndex];

  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= avgValueNumber) {
    readIndex = 0;
  }

  // calculate the average:
  float average = total / avgValueNumber;
  return average;
}

void blink(int interval) {

  static unsigned long previousMillis = 0;
  static int ledState = LOW;

  if (interval == 0) {
    digitalWrite(LED, HIGH);
  }

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(LED, ledState);
  }
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
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}
