#include <Wire.h>  //FOR I2C MODE COMUNICATION BUS


#define INDOOR 0b00100100        //power ON bit0= 0
#define OUTDOOR 0b00011100       //power ON bit0= 0
#define DIRECT_COMMAND 0x96      //write this value in the following registers to operate a command
char REG0X02reset_startup;       //this is used during resetting STATISTICAL in working operation
char REG0X02reset_startup_up;    //this is used to start up the sensor
char REG0X02reset_startup_down;  //this is used to start down the sensor

char LITERAL_mode = 1;  //0= no print data in SERIAL MONITOR,    1= yes print data on SERIAL MONITOR
char GRAPH_mode = 0;    //0= no graphical data in SERIAL PLOTTER,  1= yes graphical data   SERIAL PLOTTER

void setup() {
  Serial.begin(115200);

  Wire.begin();
  Wire.setClock(100000);  // set I2C  speed
}

void loop() {
  int c;
  int indevice = 3;  //device number I2C address, in final application you can assign
  int done = 1;      //flag to start self test on I2C sensor address

  Wire.beginTransmission(indevice);
  Wire.write(0x00);
  Wire.write(OUTDOOR);
  Wire.endTransmission(true);
  delay(200);

  Wire.beginTransmission(indevice);
  Wire.write(0x01);
  Wire.write(0b00100010);
  Wire.endTransmission(true);
  delay(200);

  Wire.beginTransmission(indevice);
  Wire.write(0x02);
  REG0X02reset_startup_up = 0b01000010;    //this is used during resetting STATISTICAL in working operation
  REG0X02reset_startup_down = 0b00000010;  //this is used during resetting STATISTICAL in working operation
  REG0X02reset_startup = 0b01000010;       //this is used at start  the sensor
  Wire.write(REG0X02reset_startup);
  Wire.endTransmission(true);
  delay(200);

  Wire.beginTransmission(indevice);
  Wire.write(0x03);
  Wire.write(0b00000000);
  Wire.endTransmission(true);
  delay(200);

  Wire.beginTransmission(indevice);
  Wire.write(0x3A);             //address to be read
  Wire.endTransmission(false);  //do not release the line!
  delay(200);

  Wire.beginTransmission(indevice);
  Wire.write(0x3B);             //address to be read
  Wire.endTransmission(false);  //do not release the line!
  delay(200);

  Wire.beginTransmission(indevice);
  Wire.write(0x08);
  Wire.write(0b00000000);  //to achieve LCO on IRQ pin program (0b10000000)
  Wire.endTransmission(true);
  delay(200);


  int INT_struck;   //event interrupt register
  int N_lightning;  //number of lightnings detected
  int Av_distance;  //average distance extimation
  int NOISE;        //interrupt for high noise level
  int Disturber;    //interrupt for disturber detected
  int L_struck;     //interrupt for lightning detected
  int Distance;     //estimated hit distance
  int Energy_LSB;   //energy single event LOW BYTE
  int Energy_MSB;   //energy single event HIGH BYTE
  int Energy_MMSB;  //energy single event AV.Energy content
  //GRAPH SCALING MIN-MAX: MODIFY AS YOU LIKE!!!
  int MAX_scale = 30;  //setting max Y of plotting
  int min_scale = 0;   //setting min Y of plotting


  while (1) {
    if (LITERAL_mode ==1)
    { 
      Serial.println();
      Serial.print(" *** WAITING FOR LIGHTNING - WATCHING ");         // Print the character
      Serial.println();
    }
    Wire.beginTransmission(indevice);
    Wire.write(0x03);               //address to be read
    Wire.endTransmission(false);    //do not release the line!
    Wire.requestFrom(indevice, 1);  // request bytes from register XY
    c = 0;
    while (Wire.available()) {
      NOISE = 0;
      Disturber = 0;
      L_struck = 0;
      Distance = 0;
      Energy_LSB = 0;
      Energy_MSB = 0;
      Energy_MMSB = 0;
      //------------------------
      c = Wire.read();  // Receive a byte as character
      INT_struck = c;
      NOISE = INT_struck & (0b00000001);
      Disturber = INT_struck & (0b00000100);
      L_struck = (INT_struck & (0b00001000)) / 8;  //this is a flag bit so can be 0 or 1.
    }
    if (L_struck > 0) {
      Wire.beginTransmission(indevice);
      Wire.write(0x07);               //address to be read
      Wire.endTransmission(false);    //do not release the line!
      Wire.requestFrom(indevice, 1);  // request bytes from register XY
      c = 0;
      while (Wire.available()) {
        c = Wire.read();  // Receive a byte as character
        Distance = c;
        Distance = Distance & (0b00111111);
      }
      //-------------------------
      //ENERGY LSB - MSB - MAIN
      //-------------------------
      Wire.beginTransmission(indevice);
      Wire.write(0x04);               //address to be read
      Wire.endTransmission(false);    //do not release the line!
      Wire.requestFrom(indevice, 1);  // request bytes from register XY
      c = 0;
      while (Wire.available()) {
        c = Wire.read();  // Receive a byte as character
        Energy_LSB = c;
      }
      //-------------------------
      //ENERGY MSB
      //-------------------------
      Wire.beginTransmission(indevice);
      // Wire.requestFrom(indevice, 1);    // Request 1 bytes from slave device
      Wire.write(0x05);               //address to be read
      Wire.endTransmission(false);    //do not release the line!
      Wire.requestFrom(indevice, 1);  // request bytes from register XY
      c = 0;
      while (Wire.available()) {
        c = Wire.read();  // Receive a byte as character
        Energy_MSB = c;
      }
      //-------------------------
      //ENERGY MMSB
      //-------------------------
      Wire.beginTransmission(indevice);
      Wire.write(0x06);               //address to be read
      Wire.endTransmission(false);    //do not release the line!
      Wire.requestFrom(indevice, 1);  // request bytes from register XY
      c = 0;
      while (Wire.available()) {
        c = Wire.read();  // Receive a byte as character
        Energy_MMSB = c;
        Energy_MMSB = Energy_MMSB & (0b00011111);
      }
      if (LITERAL_mode == 1) {
        Serial.println();
        Serial.println("*****************************************************");
        Serial.println();
        Serial.print("Noise LEVEL detected: ");
        Serial.println(NOISE, DEC);
        Serial.print("Disturber detected: ");
        Serial.println(Disturber, DEC);
        Serial.print("Lightining detected: ");
        Serial.println(L_struck, DEC);
        Serial.println();
        Serial.print("Estimated hit distance: ");
        Serial.println(Distance, DEC);
        Serial.print("LSB single event energy: ");
        Serial.println(Energy_LSB, DEC);
        Serial.print("MSB single event energy: ");
        Serial.println(Energy_MSB, DEC);
        Serial.print("MMSB single event energy: ");
        Serial.println(Energy_MMSB, DEC);
      }
      delay(200);
    }
    delay(1000);  // <------------- SCANNING INTERVAL .... MODIFY AS NEEDED !!!
  }
}