#include "OneWire.h"

uint8_t rom[8];
uint8_t resp[9];
char result[16] = "NONE";
char device[16] = "NONE";

int temp_sensor = D3;
OneWire d3 = OneWire(temp_sensor);  // DS18B20 on pin D3

#define publish_delay 60000
unsigned int lastPublish = 0;

void setup() {
  Spark.variable("result", &result, STRING);
  Spark.variable("device", &device, STRING);
  pinMode(temp_sensor, INPUT);
}

void loop() {

  unsigned long now = millis();

  if ((now - lastPublish) < publish_delay) {
    return;
  }

  d3.reset_search(); // Start back at the beginning
  if ( !d3.reset() ) {
    sprintf(device, "NO DEVICE");
    delay(5000);
    return;
   }

  d3.skip(); // no need to search, only 1 device on the bus
  d3.write(0x44,1);  // start conversation, with parasite power on at the end
  delay(1000);  // maybe 750ms is enough, maybe not

  // Get the ROM address
  d3.reset();
  d3.write(0x33);
  d3.read_bytes(rom, 8);
  // Get the temp
  d3.reset();
  d3.write(0x55);
  d3.write_bytes(rom,8);
  d3.write(0x44);
  delay(10);
  d3.reset();
  d3.write(0x55);
  d3.write_bytes(rom, 8);
  d3.write(0xBE);
  d3.read_bytes(resp, 9);

  byte MSB = resp[1];
  byte LSB = resp[0];

  int16_t intTemp = ((MSB << 8) | LSB); //using two's compliment 16-bit
  float celsius =   ((double)intTemp)/16.0;
  float fahrenheit = (( celsius*9.0)/5.0+32.0);

  sprintf(result, "%2.2f", fahrenheit);
  sprintf(device, "%x%x%x%x%x%x%x%x", rom[0], rom[1],
    rom[2], rom[3], rom[4], rom[5], rom[6], rom[7]);

  Spark.publish(device, String(result), 60, PRIVATE);
  lastPublish = now;
}
