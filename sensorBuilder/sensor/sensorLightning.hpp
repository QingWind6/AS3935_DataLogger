#ifndef _SENSOR_LIGHTNING_H
#define _SENSOR_LIGHTNING_H

#include "../sensorClass.hpp"

#define LIGHT_ADC_PIN A3

#define INDOOR            0b00100100 //power ON bit0= 0
#define OUTDOOR           0b00011100 //power ON bit0= 0
#define DIRECT_COMMAND    0x96 //write this value in the following registers to operate a command

int indevice = 3;
int count = 0;
int L_struck = 0;
int Distance = 0;
int L_s = 0, D_s = 0;

char LITERAL_mode = 1;  // 0 = no print data in SERIAL MONITOR, 1 = yes print data on SERIAL MONITOR
char GRAPH_mode = 0;

class sensorLightning : public sensorClass
{
public:
    sensorLightning() : sensorClass("Lightning"){};
    ~sensorLightning(){};

    uint16_t init(uint16_t reg, bool i2c_available);
    bool connected();
    bool sample();

    enum
    {
        LIGHTNING,
        DISTANCE,
        MAX
    };
};

uint16_t sensorLightning::init(uint16_t reg, bool i2c_available)
{
    uint16_t t_reg = reg;

    for (uint16_t i = 0; i < sensorLightning::MAX; i++)
    {
        sensorClass::reg_t value;
        value.addr = t_reg;
        value.type = sensorClass::regType_t::REG_TYPE_S32_ABCD;
        t_reg += sensorClass::valueLength(value.type);
        value.value.s32 = 0;
        m_valueVector.emplace_back(value);
    }

    if (!i2c_available)
    {
        _connected = false;
        return t_reg - reg;
    }

    GROVE_SWITCH_IIC;
    Wire.begin();
    Wire.setClock(100000);  // set I2C speed

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

    _connected = true; // Communication failed
    return t_reg - reg;
}

void get_data(int& count, int& Distance)
{
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
  }
  delay(50);
}

bool sensorLightning::sample()
{
    GROVE_SWITCH_IIC;
    get_data(L_s, D_s);

    m_valueVector[sensorLightning::LIGHTNING].value.s32 = L_s;
    m_valueVector[sensorLightning::DISTANCE].value.s32 = D_s;

    return true;
}

bool sensorLightning::connected()
{
    return _connected;
}

#endif

