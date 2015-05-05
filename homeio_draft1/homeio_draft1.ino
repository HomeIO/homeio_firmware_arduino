const int ledPin = 13;

int photoSensorPin = A0;
int photoSensorValue = 0; 

int analogValue = 0; 

int soilPeriodicASensorValue = 0; 
int soilPeriodicBSensorValue = 0; 
int soilPeriodicCSensorValue = 0; 

unsigned long periodicI = 0;

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
      analogValue = analogRead(A1);  

      buffer = analogValue / 256;
      Serial.write(buffer);

      buffer = analogValue % 256;
      Serial.write(buffer);
    }

    if (command == '2') {
      analogValue = analogRead(A2);  

      buffer = analogValue / 256;
      Serial.write(buffer);

      buffer = analogValue % 256;
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
  if (periodicI > 500000) {
    periodicI = 0;
  }

}



