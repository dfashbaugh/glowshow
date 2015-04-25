/* DmxReceiver DmxTest.ino - DMX Test
   Copyright (c) 2014 Jim Paris

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.

   Required Connections
   --------------------
   pin 0: DMX in (from RS485 transceiver)

   This test prints out DMX data to USB serial every 1 second.
   The on-board LED is toggled every time a DMX frame is received.
*/



#include <DmxReceiver.h>

// #define MAX_DMX_CHANNEL 131

// #define PACKETSIZE   131 // Not counting start/end delimters and packet ID

#define MAX_DMX_CHANNEL 142

#define PACKETSIZE   142 // WITH TAYLORS two x 11 channels

DmxReceiver dmx;
IntervalTimer dmxTimer;

char message[MAX_DMX_CHANNEL] = "";

char lastMessage[MAX_DMX_CHANNEL] = "";



void dmxTimerISR(void)
{
        dmx.bufferService();
}

void setup() {
        /* USB serial */
        // Serial.begin(115200);
        Serial2.begin(115200);
		// delay(1000);
		// Serial.println("STARTUP");
         
        /* DMX */
        dmx.begin();

        // /* Use a timer to service DMX buffers every 1ms */
        dmxTimer.begin(dmxTimerISR, 1000);

        pinMode(LED_BUILTIN, OUTPUT);

}

uint8_t v; 
int i;
int led = 0;
elapsedMillis elapsed;

void loop()
{

        /* Toggle LED on every new frame */
        if (dmx.newFrame())
        {
                led = !led;
                digitalWrite(LED_BUILTIN, led);
        }

        /* Dump DMX data every second */
        if (elapsed > 50) {
                elapsed -= 50;
        /* Dump DMX data 30 times a second */

                /* Display all nonzero DMX values */
                for ( i = 1; i <= MAX_DMX_CHANNEL; i++) {
                        
                        v = dmx.getDimmer(i);

                        if(v > 127 && v < 131)
                        {
                          v = 129;
                        }

                        message[i-1] = v;
                        
                }

				
				// delay(10);
				
                Serial2.write(130); // Start Delimeter
				//Serial2.write(1); // Write packet number
				
                for ( i = 0; i < PACKETSIZE; i++) {
                   Serial2.write(message[i]);
                }
                // delay((10);

                Serial2.write(128); //end delim
				
				/*
				// Send Packet 2
				delay(10);
				Serial2.write(130);
				Serial2.write(2);
				
				for (i = 0; i < PACKETSIZE; i++)
				{
					if( (i + 53) > MAX_DMX_CHANNEL )
					{
						Serial2.write(0);
					}
					else
					{
						Serial2.write(message[i+53]);
					}
				}
				
				Serial2.write(130);
				*/
                 // Serial.println();
                   // memcpy(lastMessage, message, MAX_DMX_CHANNEL);
                   // printDebug();
             // }

             

                 // printDebug();
                

        }


}



int k;
int allZero = 0;

boolean compareMessage(){
    
    boolean isDiff = false;
    
    for(k=0; k < MAX_DMX_CHANNEL; k++){

        if(message[k] != lastMessage[k]){
            // Serial.println("true");
            isDiff = true;
        } 

        if(message[k] == 0){

            allZero++;
        
        }

    }

//they are the same or all glitchy zero. Ignore packet
        
        // if(allZero >= MAX_DMX_CHANNEL){
        //     return false;
        // }

    return isDiff;

}

void printDebug(){

     // Serial.print("orig: ");
    for ( i = 0; i < MAX_DMX_CHANNEL; i++) {
        Serial.print(message[i],DEC);
        Serial.print(",");
     }
     Serial.println();
     // Serial.print("last: ");
     // for ( i = 0; i < MAX_DMX_CHANNEL; i++) {
     //     Serial.print(lastMessage[i],DEC);
     //     Serial.print(",");
     //  }
     //  Serial.println();

}

uint8_t red(uint32_t c) {
  return ((c & 0xFF0000) >> 16);
}
uint8_t green(uint32_t c) {
  return ((c & 0x00FF00) >> 8);
}
uint8_t blue(uint32_t c) {
  return (c & 0x0000FF);
}
