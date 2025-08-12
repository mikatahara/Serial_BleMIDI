#include <Arduino.h>
#include "RBL_nRF8001.h"

extern bool mBledo;
static uint8_t status = 0x00;
static uint8_t channel = 0x00;
static uint8_t mcnt = 0x00;
static uint8_t data1, data2;
static uint8_t msysex = 0;

void sendBleMessage3(uint8_t status, uint8_t data2, uint8_t data1){
  if(!mBledo){
    ble_write(0x80);
    ble_write(0x80);
    mBledo=true;
  } else {
    ble_write(0x80);
  }
  ble_write(status);
  ble_write(data2);
  ble_write(data1);
}

void sendBleMessage2(uint8_t status, uint8_t data1){
  if(!mBledo){
    ble_write(0x80);
    ble_write(0x80);
    mBledo=true;
  } else {
    ble_write(0x80);
  }
  ble_write(status);
  ble_write(data1);
}

void sendBleMessage1(uint8_t status){
  if(!mBledo){
    ble_write(0x80);
    ble_write(0x80);
    mBledo=true;
  } else {
    ble_write(0x80);
  }
  ble_write(status);
}

void sendBleSysEx(uint8_t val){
  if(!mBledo){
    Serial.print("+++");
    ble_write(0x80);
    mBledo=true;
    msysex=1;
  } 
  ble_write(val);
  msysex++;
  if(msysex>18){
    ble_do_events();
    msysex=0;
    mBledo=false;
  }
}

void parseSerialMIDI(uint8_t *c, uint8_t cnt)
{
  for(uint8_t i=0; i<cnt; i++){
    if(c[i]==0xFE || c[i]==0xF8){
      sendBleMessage1(c[i]);
      continue;
    }

    else if(c[i]==0xF0){
      status = 0xF0;
      msysex = 3;
      sendBleMessage1(status);
      continue;
    }

    else if(c[i]==0xF7){
      status = 0x00;
      sendBleMessage1(c[i]);
      continue;
    }

    else if(status==0xF0){
      sendBleSysEx(c[i]);
      continue;
    }

    if(c[i]&0x80){
      if(c[i]==0xFE) continue;
      status=c[i]&0xF0;
      channel=c[i]&0x0F;
      if(status==0x80 || status==0x90
        || status==0xA0
        || status==0xB0
        || status==0xE0){
          mcnt=2;
        } else if(status==0xC0 
          || status==0xD0){
          mcnt=1;
        }
    } else {
      if(mcnt==1){
        data1 = c[i];
        mcnt--;
        if(status==0x80 || status==0x90
          || status==0xA0
          || status==0xB0
          || status==0xE0){
            sendBleMessage3(status|channel, data2, data1);
            mcnt=2;
        } else if(status==0xC0 
          || status==0xD0){
            sendBleMessage2(status|channel, data1);
            mcnt=1;
        }
      } else if(mcnt==2){
        data2 = c[i];
        mcnt--;
      }
    }
  }//
////  Serial.println(" ");
}