#define USE_OCTOWS2811

#include<OctoWS2811.h>
#include<FastLED.h>

#define NUM_LEDS_PER_STRIP 300
#define NUM_STRIPS 6

const int totalLEDs = NUM_LEDS_PER_STRIP * NUM_STRIPS;

CRGB leds[totalLEDs];

boolean useHeartbeat = true;

boolean light = false;


// Access to the pixel strip

#define NUM_POLES 6

#define myADDRESS 11
#define mySETADDRESS 2
#define globalADDR 0


#define NULL_PATTERN 0
#define OFF_PATTERN 68
#define PAUSE_PATTERN 67
#define ADV_PATTERN 101
#define RESET_FRAME 100

#define TIMING_ADDR 100

#define MAX_FRAME 100000

#define PACKET_LENGTH 16 //includes start and end delimeter 

unsigned int incomingBrightness=0;
unsigned int incomingRate=0;
unsigned int rate = 2;
unsigned int patternByte = NULL_PATTERN;
unsigned int mappingByte = 0; // forward

// unix timestamp that the sketch starts at
unsigned long startedAt = 0;
unsigned long lastTime = -1;

float brightness = 1.0;
uint8_t r1 = 255, g1 = 0, b1 = 0, 
r2 = 0, g2 = 255, b2 = 0, 
r3 = 0, g3 = 0, b3 = 0;

float params[20];
uint32_t color1, color2, color3;

boolean isOff = false;
boolean advance = false;

long frame = 0;
long lastFrame = -1;

typedef uint32_t (*Pattern)(long, int);
Pattern patterns[128];
Pattern pattern;

typedef int (*Mapping)(long, int);
Mapping mapping = &forward;

unsigned long currentTime;
unsigned long lastMillis;
unsigned long internalTimeSmoother;

//String inputString = "";
boolean stringComplete = false;

IntervalTimer myTimer;

void setup() {  
  
  pinMode(13, OUTPUT); 

  Serial1.begin(9600); 
  Serial.begin(9600);
  
  LEDS.addLeds<OCTOWS2811>(leds, NUM_LEDS_PER_STRIP).setCorrection( 0x9FFAF0 );;
  LEDS.setBrightness(255);
  
  setColors();

  hideAll();
  showAll();

  patterns[62] = &flickerStrobeTwo;
  patterns[63] = &flickerStrobeFour;
  patterns[64] = &totesRandom;
  patterns[65] = &rainbowCycle;
  patterns[66] = &rainbow;
  // 67 = pause
  // 68 = off
  patterns[69] = &solidColor;
  patterns[70] = &gradient;
  patterns[71] = &pulseSine;
  patterns[72] = &pulseSaw;
  patterns[73] = &bounce;
  patterns[74] = &colorWipe;
  patterns[75] = &colorAlternator;
  patterns[76] = &stripe;
  patterns[77] = &colorChase;
  patterns[78] = &colorWipeMeter;
  patterns[79] = &colorWipeMeterGradient;
  patterns[80] = &pulseOnce;

//  pattern = &pulseOnce;
 pattern = &rainbow;

 rate = 10 ;
 brightness = 127;

  pattern(-2, 0);

  startedAt = 0;


}
int thiscounter = 0;
char c;


byte inputString [PACKET_LENGTH];

boolean fixInputString ()
{
  boolean beginFound  = false;
  boolean endFound    = false;
  int beginIndex = 0;
  int endIndex   = 0;

  for(int i = 0; i<PACKET_LENGTH; i++)
  {
    if(inputString[i] == 130)
    {
      beginFound = true;
      beginIndex = i;
    }
    else if(inputString[i] == 128)
    {
      endFound = true;
      endIndex = i;
    }
  }

  if(endFound && beginFound && (beginIndex == 0) && (endIndex == PACKET_LENGTH))
  {
    return true;
  }

  int difference = PACKET_LENGTH - endIndex;
  byte tempString [PACKET_LENGTH];
  memcpy(tempString, inputString, PACKET_LENGTH);

  for(int i = 0; i<PACKET_LENGTH; i++)
  {
      inputString[i] = tempString[ (beginIndex+i) %PACKET_LENGTH ];
  }

  return beginFound && endFound;

}

void read() {
  if (Serial1.available()>=PACKET_LENGTH) 
  { 

        Serial1.readBytes(inputString, PACKET_LENGTH);

        if(!fixInputString()) return;

      //   for(int k = 0 ; k < PACKET_LENGTH; k++){
      //   Serial.print(inputString[k]);
      //   Serial.print(",");
      // }
      // Serial.println();



        unsigned char addr = inputString[ 1 ];

//         Serial.println("ADDR");
//         Serial.println(addr);


        // Pattern.
        if (addr == myADDRESS || addr == mySETADDRESS || addr == globalADDR) {
          
          rate = inputString[ 2 ] + 1;

          patternByte = inputString[ 3 ];
          
         // Serial.println("PTTRN");
         // Serial.println(patternByte);
          
          r1 = inputString[ 4 ];
          g1 = inputString[ 5 ];
          b1 = inputString[ 6 ];
          r2 = inputString[ 7 ];
          g2 = inputString[ 8 ];
          b2 = inputString[ 9 ];

          brightness = inputString[ 10 ] / 127.0;
          // Serial.println("BRIGHTNESS");
          // Serial.println(brightness);
          setColors();

          mappingByte = inputString[ 11 ];

          // Serial.println(inputString[ 14 ]);

          //TODO: DELETE COLOR3 --- turn into heartbeat???? use 24bit integer counting tens of milliseconds
          r3 = inputString[ 12 ];
          g3 = inputString[ 13 ];
          b3 = inputString[ 14 ];

          currentTime = myColor(r3,g3,b3) * 10;

          // Serial.println(currentTime);

          //TODO: make mapping it's own byte

          if (mappingByte == 0 ) {
            mapping = &forward;
          } 
          else if (mappingByte == 1 ) {
            mapping = &backward;
          } 
          else if (mappingByte == 2 ) {
            mapping = &peak;
          } 
          else if (mappingByte == 3 ) {
            mapping = &valley;
          } 
          else if (mappingByte == 4 ) {
            mapping = &dither;
          } 

          if (patternByte == OFF_PATTERN) {
            hideAll();
            showAll(); 
            isOff = true;
          } 

          else if (patternByte != NULL_PATTERN && patterns[patternByte] != NULL) {
            isOff = false;
            pattern = patterns[patternByte];
            pattern(-2, 0); // On select initialization
          }


        }

      // }

      //inputString = "";

    // }
    
// }
// if(thiscounter > 0){
// Serial.println(thiscounter);
// }
// thiscounter = 0;
      // inputString = "";
    
}
  Serial1.flush();
}


void setColors() 
{
  
  color1 = myColor(r1, g1, b1);
  color2 = myColor(r2, g2, b2);
  color3 = myColor(r3, g3, b3);
  
}

void setBrightnRate() {

  rate = incomingRate;
  brightness = incomingBrightness;

}

unsigned long realTime;
unsigned long adjustedTime;

void loop() {

  // unsigned long startMillis = millis();

  read();

  if (isOff){
    hideAll();
    showAll();
    return;
  }


  unsigned long currentMillis = millis();// * timeDivider;

  //if heartbeat is updated, just use heartbeat
  if (currentTime != lastTime) {
    internalTimeSmoother = 0;
  }


//rate needs to determine how fast millis() runs and affects heartbeat on transmitter end as well.

  internalTimeSmoother += currentMillis - lastMillis;

  lastMillis = currentMillis;
  lastTime = currentTime;

  // unsigned long smoothedTime = currentTime + internalTimeSmoother;
  // unsigned long smoothedTime = (currentTime * timeDivider ) + internalTimeSmoother; 

  frame = ( currentTime + internalTimeSmoother ) / rate; 

  // Serial.print(internalTimeSmoother);
  // Serial.print(" ");
  // Serial.print(timeDivider);
  // Serial.print(" ");
  // Serial.println(smoothedTime);


  if (frame != lastFrame)
    pattern(-1, 0); // Per frame initialization

  lastFrame = frame;

  for (int i = 0; i < totalLEDs; i++) {

    int j = mapping(frame, i);
    uint32_t color = pattern(frame, j);

  
   uint8_t r = ((color & 0xFF0000) >> 16);
    uint8_t g = ((color & 0x00FF00) >> 8);
    uint8_t b = ((color & 0x0000FF));
    
float whiteDimmer = 0.5;

    if(r == g && g == b){
      r *= whiteDimmer;
      g *= whiteDimmer;
      b *= whiteDimmer;
    }
    
leds[i] = CRGB(r * 2, g * 2, b * 2);


    //      if (i == 0) {
    //        Serial1.println("color ");
    //        Serial1.println(r);
    //        Serial1.println(g);
    //        Serial1.println(b);
    //        Serial1.println("frame ");
    //        Serial1.println(frame);
    //        
    //        Serial1.println("===================== ");
    //      }

  }

  showAll();  

  if (frame >= MAX_FRAME) { 
    frame = 0;
  } 

  // }


  if (light)
    digitalWrite(13, HIGH);
  else
    digitalWrite(13, LOW);
    
  light = !light;

  // Serial.println(millis()-startMillis);
  // prevMillis = startMillis; 

}

/* Helper functions */

//Input a rateue 0 to 384 to get a color rateue.
//The colours are a transition r - g -b - back to r

void hideAll() {
  for (int i = 0; i < totalLEDs; i++) {
    leds[i] = CRGB(0,0,0);
  }
}

void showAll(){
  LEDS.show();     
}


