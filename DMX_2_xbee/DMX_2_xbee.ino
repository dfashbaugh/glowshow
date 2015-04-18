#include "DmxReceiver.h"
DmxReceiver DMX = DmxReceiver();

void setup() {
        DMX.begin();
        Serial.begin(115200);
        Serial2.begin(9600);
        delay(500);
        Serial.println("hi there");
      
    }

unsigned int packet [] = {128,0,0,0,0,0,0,0,0,0,0};
void loop() {
  DMX.bufferService();
  delay(30);
  if (DMX.newFrame()) //get packet
        {
          for(int i  = 1; i <= 14; i++){
            Serial.print(DMX.getDimmer(i));
            // Serial2.write(DMX.getDimmer(i));
            // if(i != 14){
              // Serial2.print(",");
              Serial.print(",");
              // }
              
          }
          Serial.println();
           // Serial.println();
        }
        
}


