/*
    SPI Master Demo Sketch
    Connect the SPI Master device to the following pins on the esp8266:
    GPIO    NodeMCU   Name  |   Uno
   ===================================
     15       D8       SS   |   D10
     13       D7      MOSI  |   D11
     12       D6      MISO  |   D12
     14       D5      SCK   |   D13
*/
#include <SPI.h>

#define MSB        0x80
#define LSB        0x01

class ESPMaster
{
private:
    uint8_t _ss_pin;

public:
    ESPMaster(uint8_t pin):_ss_pin(pin) {}
    void begin()
    {
        pinMode(_ss_pin, OUTPUT);
        digitalWrite(_ss_pin, HIGH);
    }

    uint32_t readStatus()
    {
        digitalWrite(_ss_pin, LOW);
        SPI.transfer(0x04);
        uint32_t status = (SPI.transfer(0) | ((uint32_t)(SPI.transfer(0)) << 8) | ((uint32_t)(SPI.transfer(0)) << 16) | ((uint32_t)(SPI.transfer(0)) << 24));
        digitalWrite(_ss_pin, HIGH);
        return status;
    }

    void writeStatus(uint32_t status)
    {
        digitalWrite(_ss_pin, LOW);
        SPI.transfer(0x01);
        SPI.transfer(status & 0xFF);
        SPI.transfer((status >> 8) & 0xFF);
        SPI.transfer((status >> 16) & 0xFF);
        SPI.transfer((status >> 24) & 0xFF);
        digitalWrite(_ss_pin, HIGH);
    }

    void readData(uint8_t * data)
    {
        digitalWrite(_ss_pin, LOW);
        SPI.transfer(0x03);
        SPI.transfer(0x00);
        for(uint8_t i=0; i<32; i++) {
            data[i] = SPI.transfer(0);
        }
        digitalWrite(_ss_pin, HIGH);
    }

    void writeData(uint8_t * data, size_t len)
    {
        uint8_t i=0;
        digitalWrite(_ss_pin, LOW);
        SPI.transfer(0x02);
        SPI.transfer(0x00);
        while(len-- && i < 32) {
            SPI.transfer(data[i++]);
        }
        while(i++ < 32) {
            SPI.transfer(0);
        }
        digitalWrite(_ss_pin, HIGH);
    }

    String readData()
    {
        char data[33];
        data[32] = 0;
        readData((uint8_t *)data);
        ///DRT-UPDATE
        for(int index = 32; index >= 0; index--){
          data[index] = data[index]>>1;
          if(index > 0 && data[index-1] & LSB){
            data[index] = data[index] | MSB;
          }
        }        
        return String(data);
    }

    void writeData(const char * data)
    {
        writeData((uint8_t *)data, strlen(data));
    }
};

ESPMaster esp(SS);

// 数组数据整体按位左移一位
int left_shift(unsigned char *str, int len)
{
	int i;
	
	for(i = 1; i <= len; i++)
	{
		str[i-1] = str[i-1] << 1;
    
    if(i < len && str[i] & MSB)
    {
    	str[i-1] = str[i-1] | LSB;    
    }    
	}

	return 0;
}

// 数组数据整体按位右移一位
int right_shift(unsigned char *str, int len)
{
  int i;

  for(i = len-1; i >= 0; i--)
  {
    str[i] = str[i] >> 1;
    if(i > 0 && str[i-1] & LSB)
    {
      str[i] = str[i] | MSB;    
    }    
  }
  
  return 0;    
}

void send(const char * message)
{
    Serial.print("Master: ");
    Serial.println(message);
    esp.writeData(message);
    delay(10);
    Serial.print("Slave: ");
    Serial.println(esp.readData());
    Serial.println();
}

void setup()
{
    Serial.begin(115200);
    SPI.begin();
    esp.begin();
    delay(1000);
    send("Hello Slave!");
}

void loop()
{
  while(1){
    delay(1000);
    char answer[33];
    ///send("Are you alive?");
    Serial.print("Master: ");
    Serial.println("Are you alive?\n");
    esp.writeData("Are you alive?\n");

    delay(10);
    Serial.print("Slave: ");
    Serial.println(esp.readData());
  }
}
