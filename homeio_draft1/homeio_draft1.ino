//
//    FILE: dht.h
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.14
// PURPOSE: DHT Temperature & Humidity Sensor library for Arduino
//     URL: http://arduino.cc/playground/Main/DHTLib
//
// HISTORY:
// see dht.cpp file
//

#ifndef dht_h
#define dht_h

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#define DHT_LIB_VERSION "0.1.14"

#define DHTLIB_OK                0
#define DHTLIB_ERROR_CHECKSUM   -1
#define DHTLIB_ERROR_TIMEOUT    -2
#define DHTLIB_INVALID_VALUE    -999

#define DHTLIB_DHT11_WAKEUP     18
#define DHTLIB_DHT_WAKEUP       1

// max timeout is 100 usec.
// For a 16 Mhz proc 100 usec is 1600 clock cycles
// loops using DHTLIB_TIMEOUT use at least 4 clock cycli
// so 100 us takes max 400 loops
// so by dividing F_CPU by 40000 we "fail" as fast as possible
#define DHTLIB_TIMEOUT (F_CPU/40000)

class dht
{
public:
  // return values:
  // DHTLIB_OK
  // DHTLIB_ERROR_CHECKSUM
  // DHTLIB_ERROR_TIMEOUT
  int read11(uint8_t pin);
  int read(uint8_t pin);

  inline int read21(uint8_t pin) { 
    return read(pin); 
  };
  inline int read22(uint8_t pin) { 
    return read(pin); 
  };
  inline int read33(uint8_t pin) { 
    return read(pin); 
  };
  inline int read44(uint8_t pin) { 
    return read(pin); 
  };

  double humidity;
  double temperature;

private:
  uint8_t bits[5];  // buffer to receive data
  int _readSensor(uint8_t pin, uint8_t wakeupDelay);
};
#endif
//
// END OF FILE
//


//
//    FILE: dht.cpp
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.14
// PURPOSE: DHT Temperature & Humidity Sensor library for Arduino
//     URL: http://arduino.cc/playground/Main/DHTLib
//
// HISTORY:
// 0.1.14 replace digital read with faster (~3x) code => more robust low MHz machines.
// 0.1.13 fix negative temperature
// 0.1.12 support DHT33 and DHT44 initial version
// 0.1.11 renamed DHTLIB_TIMEOUT
// 0.1.10 optimized faster WAKEUP + TIMEOUT
// 0.1.09 optimize size: timeout check + use of mask
// 0.1.08 added formula for timeout based upon clockspeed
// 0.1.07 added support for DHT21
// 0.1.06 minimize footprint (2012-12-27)
// 0.1.05 fixed negative temperature bug (thanks to Roseman)
// 0.1.04 improved readability of code using DHTLIB_OK in code
// 0.1.03 added error values for temp and humidity when read failed
// 0.1.02 added error codes
// 0.1.01 added support for Arduino 1.0, fixed typos (31/12/2011)
// 0.1.00 by Rob Tillaart (01/04/2011)
//
// inspired by DHT11 library
//
// Released to the public domain
//

/////////////////////////////////////////////////////
//
// PUBLIC
//

// return values:
// DHTLIB_OK
// DHTLIB_ERROR_CHECKSUM
// DHTLIB_ERROR_TIMEOUT
int dht::read11(uint8_t pin)
{
  // READ VALUES
  int rv = _readSensor(pin, DHTLIB_DHT11_WAKEUP);
  if (rv != DHTLIB_OK)
  {
    humidity    = DHTLIB_INVALID_VALUE; // invalid value, or is NaN prefered?
    temperature = DHTLIB_INVALID_VALUE; // invalid value
    return rv;
  }

  // CONVERT AND STORE
  humidity    = bits[0];  // bits[1] == 0;
  temperature = bits[2];  // bits[3] == 0;

  // TEST CHECKSUM
  // bits[1] && bits[3] both 0
  uint8_t sum = bits[0] + bits[2];
  if (bits[4] != sum) return DHTLIB_ERROR_CHECKSUM;

  return DHTLIB_OK;
}


// return values:
// DHTLIB_OK
// DHTLIB_ERROR_CHECKSUM
// DHTLIB_ERROR_TIMEOUT
int dht::read(uint8_t pin)
{
  // READ VALUES
  int rv = _readSensor(pin, DHTLIB_DHT_WAKEUP);
  if (rv != DHTLIB_OK)
  {
    humidity    = DHTLIB_INVALID_VALUE;  // invalid value, or is NaN prefered?
    temperature = DHTLIB_INVALID_VALUE;  // invalid value
    return rv; // propagate error value
  }

  // CONVERT AND STORE
  humidity = word(bits[0], bits[1]) * 0.1;
  temperature = word(bits[2] & 0x7F, bits[3]) * 0.1;
  if (bits[2] & 0x80)  // negative temperature
  {
    temperature = -temperature;
  }

  // TEST CHECKSUM
  uint8_t sum = bits[0] + bits[1] + bits[2] + bits[3];
  if (bits[4] != sum)
  {
    return DHTLIB_ERROR_CHECKSUM;
  }
  return DHTLIB_OK;
}

/////////////////////////////////////////////////////
//
// PRIVATE
//

// return values:
// DHTLIB_OK
// DHTLIB_ERROR_TIMEOUT
int dht::_readSensor(uint8_t pin, uint8_t wakeupDelay)
{
  // INIT BUFFERVAR TO RECEIVE DATA
  uint8_t mask = 128;
  uint8_t idx = 0;

  // replace digitalRead() with Direct Port Reads.
  // reduces footprint ~100 bytes => portability issue?
  // direct port read is about 3x faster
  uint8_t bit = digitalPinToBitMask(pin);
  uint8_t port = digitalPinToPort(pin);
  volatile uint8_t *PIR = portInputRegister(port);

  // EMPTY BUFFER
  for (uint8_t i = 0; i < 5; i++) bits[i] = 0;

  // REQUEST SAMPLE
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW); // T-be 
  delay(wakeupDelay);
  digitalWrite(pin, HIGH);   // T-go
  delayMicroseconds(40);
  pinMode(pin, INPUT);

  // GET ACKNOWLEDGE or TIMEOUT
  uint16_t loopCntLOW = DHTLIB_TIMEOUT;
  while ((*PIR & bit) == LOW )  // T-rel
  {
    if (--loopCntLOW == 0) return DHTLIB_ERROR_TIMEOUT;
  }

  uint16_t loopCntHIGH = DHTLIB_TIMEOUT;
  while ((*PIR & bit) != LOW )  // T-reh
  {
    if (--loopCntHIGH == 0) return DHTLIB_ERROR_TIMEOUT;
  }

  // READ THE OUTPUT - 40 BITS => 5 BYTES
  for (uint8_t i = 40; i != 0; i--)
  {
    loopCntLOW = DHTLIB_TIMEOUT;
    while ((*PIR & bit) == LOW )
    {
      if (--loopCntLOW == 0) return DHTLIB_ERROR_TIMEOUT;
    }

    uint32_t t = micros();

    loopCntHIGH = DHTLIB_TIMEOUT;
    while ((*PIR & bit) != LOW )
    {
      if (--loopCntHIGH == 0) return DHTLIB_ERROR_TIMEOUT;
    }

    if ((micros() - t) > 40)
    { 
      bits[idx] |= mask;
    }
    mask >>= 1;
    if (mask == 0)   // next byte?
    {
      mask = 128;
      idx++;
    }
  }
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);

  return DHTLIB_OK;
}
//
// END OF FILE
//





















// I had some problems including other files so I put stuff above
// My code is below of that line

#define DHT22_INTERNAL_PIN 0
#define DHT21_EXTERNAL_PIN 1

const int ledPin = 13;

int photoSensorPin = A0;
int photoSensorValue = 0; 

int analogValue = 0; 
int tempValue = 0;
int ignoreCount = 0;

int soilPeriodicASensorValue = 0; 
int soilPeriodicBSensorValue = 0; 
int soilPeriodicCSensorValue = 0; 

unsigned long periodicI = 0;
unsigned long periodicT = 0; // its not pretty, but faster

unsigned int testValue = 12345; 
byte buffer = 0;

dht DHT;
int internalChk = DHTLIB_ERROR_CHECKSUM;
int internalHumidity = 0;
int internalTemp = 0;
int externalChk = DHTLIB_ERROR_CHECKSUM;
int externalHumidity = 0;
int externalTemp = 0;

void setup()
{
  // initialize the serial communication:
  Serial.begin(38400);
  while (!Serial) ;
}

void loop() {
  byte command;

  // check if data has been sent from the computer:
  if (Serial.available()) {
    // read the most recent byte (which will be from 0 to 255):
    digitalWrite(ledPin, HIGH);   // turn the LED on (HIGH is the voltage level)

    command = Serial.read();
    if (command == '0') {
      photoSensorValue = analogRead(photoSensorPin);  

      buffer = photoSensorValue / 256;
      Serial.write(buffer);

      buffer = photoSensorValue % 256;
      Serial.write(buffer);
    }

    if (command == '1') {
      analogValue = analogRead(A1);  

      buffer = analogValue / 256;
      Serial.write(buffer);

      buffer = analogValue % 256;
      Serial.write(buffer);
    }

    // temperature
    // temporary sensor is not connected
    if (command == '2') {
      buffer = tempValue / 256;
      Serial.write(buffer);

      buffer = tempValue % 256;
      Serial.write(buffer);
    }

    // soil periodic
    if (command == '3') {
      buffer = soilPeriodicASensorValue / 256;
      Serial.write(buffer);

      buffer = soilPeriodicASensorValue % 256;
      Serial.write(buffer);
    }

    if (command == '4') {
      buffer = soilPeriodicBSensorValue / 256;
      Serial.write(buffer);

      buffer = soilPeriodicBSensorValue % 256;
      Serial.write(buffer);
    }

    if (command == '5') {
      buffer = soilPeriodicCSensorValue / 256;
      Serial.write(buffer);

      buffer = soilPeriodicCSensorValue % 256;
      Serial.write(buffer);
    }
    
    // internal - humidity
    if (command == 'h') {
      buffer = internalHumidity / 256;
      Serial.write(buffer);

      buffer = internalHumidity % 256;
      Serial.write(buffer);
    }
    
    // internal - digital temp
    if (command == 'd') {
      buffer = internalTemp / 256;
      Serial.write(buffer);

      buffer = internalTemp % 256;
      Serial.write(buffer);
    }
    
    // internal - digital sensor status
    if (command == 'e') {
      buffer = internalChk / 256;
      Serial.write(buffer);

      buffer = internalChk % 256;
      Serial.write(buffer);
    }    

    // external - humidity
    if (command == 'H') {
      buffer = externalHumidity / 256;
      Serial.write(buffer);

      buffer = externalHumidity % 256;
      Serial.write(buffer);
    }
    
    // external - digital temp
    if (command == 'D') {
      buffer = externalTemp / 256;
      Serial.write(buffer);

      buffer = externalTemp % 256;
      Serial.write(buffer);
    }
    
    // external - digital sensor status
    if (command == 'E') {
      buffer = externalChk / 256;
      Serial.write(buffer);

      buffer = externalChk % 256;
      Serial.write(buffer);
    }    


    if (command == 't') {
      buffer = testValue / 256;
      Serial.write(buffer);

      buffer = testValue % 256;
      Serial.write(buffer);
    }

    if (command == 's') {
      buffer = 0;
      Serial.write(buffer);
    }

    //delay(1);
    digitalWrite(ledPin, LOW);
  }

  if (periodicI == 0) {
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);

    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
  }
  else if (periodicI == 2400) {
    soilPeriodicASensorValue = analogRead(A3);      
    soilPeriodicBSensorValue = analogRead(A4);  
    soilPeriodicCSensorValue = analogRead(A5);  

    pinMode(3, INPUT);
    pinMode(4, INPUT);
    pinMode(5, INPUT);
  }


  periodicI++;
  periodicT++;

  if (periodicI > 500000) {
    periodicI = 0;
  }

  
  if (periodicT > 1000) {
    // temp noise fix
    periodicT = 0;

    analogValue = analogRead(A2);

    if ( (tempValue == 0) || ( abs(tempValue - analogValue) < 5 ) || ( ignoreCount > 10 ) ) {
      ignoreCount = 0;
      tempValue = analogValue;
    }
    else {
      ignoreCount++;
    }
    
    // digital humidity and temp
    internalChk = DHT.read22(DHT22_INTERNAL_PIN);
    if (internalChk == DHTLIB_OK) {
      internalHumidity = (int) ( DHT.humidity * 10.0 );
      internalTemp = (int) ( ( 50.0 + DHT.temperature ) * 10.0 );      
    }
    
    externalChk = DHT.read22(DHT21_EXTERNAL_PIN);
    if (externalChk == DHTLIB_OK) {
      externalHumidity = (int) ( DHT.humidity * 10.0 );
      externalTemp = (int) ( ( 50.0 + DHT.temperature ) * 10.0 );      
    }    
  }
  
  

}





