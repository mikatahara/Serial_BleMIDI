#include <Arduino.h>

#define _DEBUG_ 0

static uint8_t status = 0x00;

void sendSysEx(uint8_t x){
#if _DEBUG_
  Serial.print(x,HEX);
  Serial.print(" ");
#endif
  Serial1.write(x);
}

uint8_t parseSysEx(uint8_t *c, uint8_t cnt){
  uint8_t i=1;

  if(!(c[1]&0x80)){
    sendSysEx(c[1]);
  }
  for(i=2; i<cnt; i++){
    if(!(c[i]&0x80)){
      sendSysEx(c[i]);
    } else {
      i++;
      if(c[i]==0xF7){
        sendSysEx(0xF7);
        status=0x00;
        i++;
        break;
      } else if(c[i]==0xF8 || c[i]==0xFE){
        Serial1.write(c[i]);
      }
    }
  }
  return i;
}

void parseBleMIDI(uint8_t *c, uint8_t cnt) {
  uint8_t data1;
  uint8_t data2;
  uint8_t i;
  uint8_t st = 2;

#if _DEBUG_
Serial.println("R:");
for(i=0;i<cnt;i++){
  Serial.print(c[i],HEX);
  Serial.print(" ");
}
Serial.println(" ");
#endif

  if (status == 0xF0) { //System Exclusive continue
    st=parseSysEx(c, cnt);
    st++;
#if 0
    Serial.print("RT=");
    Serial.println(c[st],HEX);
#endif
  }

  i=st;
  while (i < cnt) {
    if (c[i] & 0x80) {
      status = c[i++];
    }

    if(status==0xF8 || status==0xFE){
      Serial1.write(status);
    }

    else {
      switch (status & 0xF0) {
        case 0x80:
        case 0x90:
        case 0xA0:
        case 0xB0:
        case 0xE0:
          data1 = c[i++];
          data2 = c[i++];
          Serial1.write(status);
          Serial1.write(data1);
          Serial1.write(data2);
#if _DEBUG_
          Serial.print(status, HEX);
          Serial.print(" ");
          Serial.print(data1, HEX);
          Serial.print(" ");
          Serial.println(data2, HEX);
#endif
         break;
        case 0xC0:
        case 0xD0:
          data1 = c[i++];
          Serial1.write(status);
          Serial1.write(data1);
#if _DEBUG_
          Serial.print(status, HEX);
          Serial.print(" ");
          Serial.println(data1, HEX);
#endif
          break;
      
        case 0xF0:
          if (status == 0xF0) {
            sendSysEx(status);
            while (i < cnt) {
              uint8_t v = c[i++];
              if (v & 0x80) { //v=time stamp
                v = c[i++];   //0xF7
                if(v==0xF7){
                  sendSysEx(v);
                  status = 0x00;
                  break;
                } else {
                  sendSysEx(0xF7);
                  status = 0x00;
                  break;
                }
              } else {
                sendSysEx(v);
              }
            }
          } 
        }
    }
    if(i<cnt){
      uint8_t tm=c[i++];
#if _DEBUG_
      Serial.print("tm=");
      Serial.println(tm,HEX);
#endif      
    }
  }
}