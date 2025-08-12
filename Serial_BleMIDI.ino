// Serial MIDI <-> BLE MIDI Converter
#include "RBL_nRF8001.h"

#define SYSBUFSIZE  128

extern void parseBleMIDI(uint8_t *c, uint8_t cnt);
extern void parseSerialMIDI(uint8_t *c, uint8_t cnt);

bool mBledo=false;
uint8_t *sysbuf;

void setup() {
  Serial.begin(115200);  //Serial MIDI 31.25kbps
  //BLE initialize
  ble_set_pins(9, 8);
  ble_set_name("BlueMIDI");
  ble_begin();
  Serial1.begin(31250);   //For Serial MIDI
  sysbuf = (uint8_t*)malloc(SYSBUFSIZE);
}

void loop() {
  uint8_t c[20];
  uint8_t cnt=0;
  uint8_t scnt=0;

  // BleMIDI to Serial
  while ( ble_available() )
  {
    c[cnt]=ble_read(); // Receive MIDI Packet
    cnt++;
  }
  if(cnt){
    parseBleMIDI(c,cnt);
  }

  // Serial to BleMIDI
  while (Serial1.available()) {
    sysbuf[scnt] = Serial1.read();
    scnt++;
    if(scnt>SYSBUFSIZE){
      scnt=0;
      break;
     }
  }

  if(scnt){
    parseSerialMIDI(sysbuf,scnt);
  }

  mBledo =false;
  ble_do_events();
 }
