const int ledPin = 13;

int photoSensorPin = A0;
int photoSensorValue = 0; 

int soilSensorPin = A1;
int soilSensorValue = 0; 

int tempSensorPin = A2;
int tempSensorValue = 0; 

unsigned int testValue = 12345; 
byte buffer = 0;

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
      soilSensorValue = analogRead(soilSensorPin);  

      buffer = soilSensorValue / 256;
      Serial.write(buffer);

      buffer = soilSensorValue % 256;
      Serial.write(buffer);
    }

    if (command == '2') {
      tempSensorValue = analogRead(tempSensorPin);  

      buffer = tempSensorValue / 256;
      Serial.write(buffer);

      buffer = tempSensorValue % 256;
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

    delay(1);
    digitalWrite(ledPin, LOW);
  }
}


