#define USE_OCTOWS2811

#include<OctoWS2811.h>
#include<FastLED.h>

#define NUM_STRIPS 6

boolean useHeartbeat = true;

boolean light = false;


int taylorTopLength = 150;

int taylorBotLength = 150;



//IF YOU ARE TAYLOR SWIFT, UNCOMMENT THIS LINE:
#define IS_TAYLOR 1

#ifdef IS_TAYLOR

#define DANCERNUMBER 13
//set this to the longest strip output length
#define NUM_LEDS_PER_STRIP 300

#else

#define DANCERNUMBER 10
//set this to the longest strip output length
#define NUM_LEDS_PER_STRIP 300

#endif

const int totalLEDs = NUM_LEDS_PER_STRIP * NUM_STRIPS;

CRGB leds[totalLEDs];

#define READBUFFERSIZE 133

#define NUM_ROWS 36

// Values applied to specific dancer
float mIndBrightness = 1.0;
byte mRed1       = 0;
byte mGreen1     = 0;
byte mBlue1      = 0;
byte mPattern    = 0;
byte mRate       = 0;
byte mMapping    = 0;
byte mRed2       = 0;
byte mGreen2     = 0;
byte mBlue2      = 0;

// Only applies to Taylor
float mIndBrightness_Bottom = 1.0;
byte mRed1_Bottom           = 0;
byte mGreen1_Bottom         = 0;
byte mBlue1_Bottom          = 0;
byte mPattern_Bottom        = 0;
byte mRate_Bottom           = 0;
byte mMapping_Bottom        = 0;
byte mRed2_Bottom           = 0;
byte mGreen2_Bottom         = 0;
byte mBlue2_Bottom          = 0;

int  mCurrentFrameCount = 0;
int  mCurrentFrameCount_Bottom = 0;

byte lastPattern = 0;
byte lastPattern_Bottom = 0;


byte mTaylorMappingTop   = 0;
byte mTaylorMappingBot   = 0;

#define NUM_STEPS_PER_FRAME 20
#define NULL_PATTERN 0
#define OFF_PATTERN 68
#define PAUSE_PATTERN 67
#define ADV_PATTERN 101
#define RESET_FRAME 100

#define TIMING_ADDR 100

#define MAX_FRAME 2000000


unsigned int rate;
unsigned int patternByte = NULL_PATTERN;

unsigned int rate_Bottom;
unsigned int patternByte_Bottom = NULL_PATTERN;



uint8_t r1 = 255, g1 = 0, b1 = 0,
        r2 = 0, g2 = 255, b2 = 0;

uint8_t r3 = 255, g3 = 0, b3 = 0,
        r4 = 0, g4 = 255, b4 = 0;


float params[20];
uint32_t color1, color2, color3 = 0, color4, color5;

boolean isOff = false;

long frame = 1000000;
long lastFrame = -1;

boolean isOff_Bottom = false;

long frame_Bottom = 1000000;
long lastFrame_Bottom = -1;

typedef uint32_t (*Pattern)(long, int);
Pattern patterns[128];
Pattern pattern;

Pattern pattern_Bottom;

typedef int (*Mapping)(long, int);
Mapping mapping = &forward;

Mapping mapping_Bottom = &forward;

Mapping taylorMapTop = &snake;
Mapping taylorMapBottom = &snake;


void setup() {

  pinMode(13, OUTPUT);

  Serial1.begin(115200);
  Serial.begin(115200);

  LEDS.addLeds<OCTOWS2811>(leds, NUM_LEDS_PER_STRIP).setCorrection( 0x9FFAF0 );;
  LEDS.setBrightness(32);

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

  rate = 126;
  // pattern = &pulseOnce;
  pattern = &stripe;

  mIndBrightness = 255;

  rate_Bottom = 126;
  // pattern = &pulseOnce;
  pattern_Bottom = &stripe;

  mIndBrightness_Bottom = 255;

  pattern(-2, 0);
  pattern_Bottom(-2, 0);



}

byte currentCommandBuf [READBUFFERSIZE];

boolean fixInputString ()
{
  boolean beginFound  = false;
  boolean endFound    = false;
  int beginIndex = 0;
  int endIndex   = 0;

  for (int i = 0; i < READBUFFERSIZE; i++)
  {
    if (currentCommandBuf[i] == 130)
    {
      beginFound = true;
      beginIndex = i;
    }
    else if (currentCommandBuf[i] == 128)
    {
      endFound = true;
      endIndex = i;
    }
  }

  if (endFound && beginFound && (beginIndex == 0) && (endIndex == READBUFFERSIZE))
  {
    return true;
  }

  int difference = READBUFFERSIZE - endIndex;
  byte tempString [READBUFFERSIZE];
  memcpy(tempString, currentCommandBuf, READBUFFERSIZE);

  for (int i = 0; i < READBUFFERSIZE; i++)
  {
    currentCommandBuf[i] = tempString[ (beginIndex + i) % READBUFFERSIZE ];
  }

  return beginFound && endFound;

}

void read() {

  if (Serial1.available()>=READBUFFERSIZE) 
  { 

        Serial1.readBytes(currentCommandBuf, READBUFFERSIZE);

        if(!fixInputString()) return;

        if(DANCERNUMBER < 13){
            mIndBrightness  = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 1 ] / 255.0;
            mRed1           = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 2 ];
            mGreen1         = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 3 ];
            mBlue1          = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 4 ];
            mPattern        = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 5 ];
            mRate           = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 6 ];
            mMapping        = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 7 ];
            mRed2           = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 8 ]; 
            mGreen2         = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 9 ];
            mBlue2          = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 10 ]; 
        }

        if(DANCERNUMBER == 13){
                      mIndBrightness    = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 1 ] / 255.0;
                      mRed1             = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 2 ];
                      mGreen1           = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 3 ];
                      mBlue1            = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 4 ];
                      mPattern          = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 5 ];
                      mRate             = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 6 ];
                      mMapping          = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 7 ];
                      mRed2             = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 8 ]; 
                      mGreen2           = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 9 ];
                      mBlue2            = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 10 ]; 
                      mTaylorMappingTop = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 11 ];

                      mIndBrightness_Bottom    = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 12 ] / 255.0;
                      mRed1_Bottom             = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 13 ];
                      mGreen1_Bottom           = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 14 ];
                      mBlue1_Bottom            = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 15 ];
                      mPattern_Bottom          = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 16 ];
                      mRate_Bottom             = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 17 ];
                      mMapping_Bottom          = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 18 ];
                      mRed2_Bottom             = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 19 ]; 
                      mGreen2_Bottom           = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 20 ];
                      mBlue2_Bottom            = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 21 ]; 
                      mTaylorMappingBot        = currentCommandBuf[ (DANCERNUMBER-1) * 10 + 22 ];
                      
                      r3 = mRed1_Bottom;
                      g3 = mGreen1_Bottom;
                      b3 = mBlue1_Bottom;

                      r4 = mRed2_Bottom;
                      g4 = mGreen2_Bottom;
                      b4 = mBlue2_Bottom;


                      rate_Bottom = mRate_Bottom + 1;

                      patternByte_Bottom = mPattern_to_patternByte(mPattern_Bottom);


                      if (mMapping_Bottom == 0 ) {
                        mapping_Bottom = &forward;
                      } 
                      else if (mMapping_Bottom == 1 ) {
                        mapping_Bottom = &backward;
                      } 
                      else if (mMapping_Bottom == 2 ) {
                        mapping_Bottom = &peak;
                      } 
                      else if (mMapping_Bottom == 3 ) {
                        mapping_Bottom = &valley;
                      } 
                      else if (mMapping_Bottom == 4 ) {
                        mapping_Bottom = &dither;
                      } 

                      if (patternByte_Bottom == OFF_PATTERN) {
                        hideAll();
                        showAll(); 
                        isOff_Bottom = true;
                      } 

                      else if (patternByte_Bottom != NULL_PATTERN && patterns[patternByte_Bottom] != NULL) {
                        isOff_Bottom = false;
                        pattern_Bottom = patterns[patternByte_Bottom];
                        pattern_Bottom(-2, 0); // On select initialization
                      }


                      // Reset frame if pattern changes
                      if(patternByte_Bottom != lastPattern_Bottom)
                      {
                        lastPattern_Bottom = patternByte_Bottom;
                         frame_Bottom = 1000000;
                      }

        }

          r1 = mRed1;
          g1 = mGreen1;
          b1 = mBlue1;

          r2 = mRed2;
          g2 = mGreen2;
          b2 = mBlue2;

          //singleColor has black second color
          setColors();

          rate = mRate + 1;

          patternByte = mPattern_to_patternByte(mPattern);


          if (mMapping == 0 ) {
            mapping = &forward;
          } 
          else if (mMapping == 1 ) {
            mapping = &backward;
          } 
          else if (mMapping == 2 ) {
            mapping = &peak;
          } 
          else if (mMapping == 3 ) {
            mapping = &valley;
          } 
          else if (mMapping == 4 ) {
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


          // Reset frame if pattern changes
          if(patternByte != lastPattern)
          {
            lastPattern = patternByte;
             frame = 1000000;
          }

        }

  Serial1.flush();

}



unsigned int mPattern_to_patternByte(byte incomingByte)
{
  switch (incomingByte) {
    case 0:
      return 68;
      break;
    case 1:
      return 69;
      break;
    case 2:
      return 70;
      break;
    case 3:
      return 62;
      break;
    case 4:
      return 63;
      break;
    case 5:
      return 64;
      break;
    case 6:
      return 65;
      break;
    case 7:
      return 66;
      break;
    case 8:
      return 71;
      break;
    case 9:
      return 72;
      break;
    case 10:
      return 73;
      break;
    case 11:
      return 74;
      break;
    case 12:
      return 75;
      break;
    case 13:
      return 76;
      break;
    case 14:
      return 77;
      break;
    case 15:
      return 78;
      break;
    case 16:
      return 79;
      break;
    case 17:
      return 80;
      break;
    default:
      return 0;
      break;
  }


}

void setColors()
{
  color1 = myColor(r1, g1, b1);
  color2 = myColor(r2, g2, b2);

  color4 = myColor(r3, g3, b3);
  color5 = myColor(r4, g4, b4);
}


void loop() {

  read();

#ifndef IS_TAYLOR
  if (isOff) {
    hideAll();
    showAll();
    return;
  }
#endif

#ifdef IS_TAYLOR
  if (isOff) {
    hideAll_top();
    showAll();
    return;
  }
    if (isOff_Bottom) {
    hideAll_bot();
    showAll();
    return;
  }
#endif


  int usedRate = 128-rate;
  mCurrentFrameCount += abs(usedRate);
  
  if(mCurrentFrameCount >= NUM_STEPS_PER_FRAME)
  {
    int framesToMove = mCurrentFrameCount/NUM_STEPS_PER_FRAME;
    mCurrentFrameCount = mCurrentFrameCount - framesToMove*NUM_STEPS_PER_FRAME;
    
    frame += usedRate < 0 ? -1*framesToMove : framesToMove;  
  }

  if (frame != lastFrame)
    pattern(-1, 0); // Per frame initialization

  lastFrame = frame;

  for (int i = 0; i < totalLEDs; i++) {

    int j = mapping(frame, taylorMapTop(frame,i));
    uint32_t color = pattern(frame, j);


    uint8_t r = ((color & 0xFF0000) >> 16);
    uint8_t g = ((color & 0x00FF00) >> 8);
    uint8_t b = ((color & 0x0000FF));


    if (mIndBrightness < 1.0) {
      r = lerp(0, r, mIndBrightness);
      g = lerp(0, g, mIndBrightness);
      b = lerp(0, b, mIndBrightness);
    }


    float whiteDimmer = 0.5;

    if (r == g && g == b) {
      r *= whiteDimmer;
      g *= whiteDimmer;
      b *= whiteDimmer;
    }

    leds[i] = CRGB(r, g, b);

  }


  showAll();

  if (frame >= MAX_FRAME) {
    frame = 0;
  }



  if (light)
    digitalWrite(13, HIGH);
  else
    digitalWrite(13, LOW);

  light = !light;

}

/* Helper functions */

//Input a rateue 0 to 384 to get a color rateue.
//The colours are a transition r - g -b - back to r

void hideAll() {
  for (int i = 0; i < totalLEDs; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
}

void hideAll_top() {
  for (int i = 0; i < taylorTopLength; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
}

void hideAll_bot() {
  for (int i = taylorTopLength; i < taylorTopLength + taylorBotLength; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
}

void showAll() {
  LEDS.show();
}


