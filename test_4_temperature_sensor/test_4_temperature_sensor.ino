#include <SimpleDHT.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#define THERMISTORE_LED 3
#define DHT11_PIN 2
#define ONE_WIRE_BUS 3

//Normalization Constants
#define ZERO -0.5
#define SLOPE 0.01
#define CORRECTION 0

#define TEMP_SENSOR A2

int thermistoreValue = 0;

SimpleDHT11 dht11;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(57600);

  pinMode(THERMISTORE_LED, OUTPUT);
  sensors.begin();

}

void loop() {
  thermistore_read();
  dth11_read();
  ds18b20_read();
  mcp9701a_read();
  Serial.println("");
}


void thermistore_read() {

  int value = analogRead(A3);
  if (thermistoreValue < value) {
    digitalWrite(THERMISTORE_LED, HIGH);
  } else {
    digitalWrite(THERMISTORE_LED, LOW);
  }
  thermistoreValue = value;
  float ref = readVcc() / 1000;
  float temp = analogRead(A3) * ref / 1024.0;
  String stringValue = String(thermistoreValue);
  Serial.print("Thermistore value " + stringValue);
  Serial.print("@");
  Serial.print(temp);
  Serial.println("*C");
}

void dth11_read() {
  // read with raw sample data.
  byte temperature = 0;
  byte humidity = 0;
  byte data[40] = {0};
  if (dht11.read(DHT11_PIN, &temperature, &humidity, data)) {
    Serial.println("Read DHT11 failed");
    return;
  }

  Serial.print("DTF11 temperature: ");
  Serial.print((int)temperature); Serial.print(" *C, ");
  Serial.print((int)humidity); Serial.println(" %");
}

void ds18b20_read() {
  sensors.requestTemperatures();
  Serial.print("Temperature DS18B20 for the device 1 (index 0) is: ");
  Serial.print(sensors.getTempCByIndex(0));
  Serial.println("*C");
}


void mcp9701a_read() {
    //Analog read
  int readValue = analogRead(A2);

  //Conversion in degrees Celsius *C
  float ref = readVcc() / 1000;
  float temp = readValue *5/ 1024.0;
  temp = temp + ZERO;
  temp = temp / SLOPE;

  temp= temp + CORRECTION;


  Serial.print("Reference Vcc ");
  Serial.println(ref);
  Serial.print("Temperature MCP9701A ");
  Serial.print(readValue);
  Serial.print("@ ");
  Serial.print(temp);
  Serial.println("*C");
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
