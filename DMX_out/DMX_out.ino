#include <DmxInterrupt.h>
#include <DmxSimple.h>

void setup() {
  DmxSimple.maxChannel(14); // Talking to a four channel receiver, so send all four channels.
}

  int brightness=0; 
void loop() { 
  
   // DmxSimple.write(1, 128); // Set DMX channel 1 to new value
   // for(int i = 2 ; i <= 14;i++){
   //      DmxSimple.write(i, random(0,128)); 
   //  }

     DmxSimple.write(1, 0); //Address
     DmxSimple.write(2, 50); //rate
     DmxSimple.write(3, 76); //pattern

     DmxSimple.write(4, brightness++);
     DmxSimple.write(5, 127);
     DmxSimple.write(6, 0);

     DmxSimple.write(7, 127);
     DmxSimple.write(8, 0);
     DmxSimple.write(9, 0);

     DmxSimple.write(10, 0);
     DmxSimple.write(11, 0);
     DmxSimple.write(12, 0);

     DmxSimple.write(13, 127); //brightness
     DmxSimple.write(14, 128);

    delay(10); // Wait 10ms
  brightness%=127;
}
