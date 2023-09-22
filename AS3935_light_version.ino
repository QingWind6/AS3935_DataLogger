#include <Wire.h>

// #define OUTDOOR 0b00100100
#define OUTDOOR 0b00011100

int indevice = 3;
int L_struck = 0;
int Distance = 0;

int count = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(100000);

  Wire.beginTransmission(indevice);
  Wire.write(0x00);
  Wire.write(OUTDOOR);
  Wire.endTransmission(true);
  delay(200);

  Wire.beginTransmission(indevice);
  Wire.write(0x3A);
  Wire.endTransmission(false);
  delay(200);

  Wire.beginTransmission(indevice);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  delay(200);

  Wire.requestFrom(indevice, 1);
  if(Wire.available()) 
  {
    Serial.println("find AS3935!");         // Print the character
  }
}

void loop() {
  Wire.beginTransmission(indevice);
  Wire.write(0x03);
  Wire.endTransmission(false);
  Wire.requestFrom(indevice, 1);
  while (Wire.available()) {
    int INT_struck = Wire.read();
    L_struck = (INT_struck & (0b00001000)) / 8;
  }

  if (L_struck > 0) {
    Wire.beginTransmission(indevice);
    Wire.write(0x07);
    Wire.endTransmission(false);
    Wire.requestFrom(indevice, 1);
    count ++;
    while (Wire.available()) {
      Distance = Wire.read();
      Distance = Distance & (0b00111111);
    }

    if (Distance > 0) {
      Serial.println("Detected lightning!");
      Serial.print("Estimated hit distance: ");
      Serial.println(Distance);
    }
    Serial.print("Lighting Count: ");
    Serial.println(count);
  }

  delay(1000);
}



