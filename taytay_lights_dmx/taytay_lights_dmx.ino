#define USE_OCTOWS2811

#include<OctoWS2811.h>
#include<FastLED.h>

#define NUM_LEDS_PER_STRIP 300
#define NUM_STRIPS 6

const int totalLEDs = NUM_LEDS_PER_STRIP * NUM_STRIPS;

CRGB leds[totalLEDs];

boolean useHeartbeat = true;

boolean light = false;

#define DANCERNUMBER 1
#define READBUFFERSIZE 133

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


#define NULL_PATTERN 0
#define OFF_PATTERN 68
#define PAUSE_PATTERN 67
#define ADV_PATTERN 101
#define RESET_FRAME 100

#define TIMING_ADDR 100

#define MAX_FRAME 100000


unsigned int rate = 2;
unsigned int patternByte = NULL_PATTERN;

// unix timestamp that the sketch starts at
unsigned long startedAt = 0;
unsigned long lastTime = -1;

uint8_t r1 = 255, g1 = 0, b1 = 0,
        r2 = 0, g2 = 255, b2 = 0;

uint8_t heartByte1 = 0, heartByte2 = 0, heartByte3 = 0;

float params[20];
uint32_t color1, color2, color3 = 0;

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

boolean stringComplete = false;

IntervalTimer myTimer;

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

  // pattern = &pulseOnce;
  pattern = &gradient;

  rate = 10 ;
  mIndBrightness = 255;

  pattern(-2, 0);

  startedAt = 0;


}
int thiscounter = 0;
char c;


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


  if (Serial1.available() >= READBUFFERSIZE)
  {

    Serial1.readBytes(currentCommandBuf, READBUFFERSIZE);

    if (!fixInputString()) return;



    // Values applied to specific dancer
    // float mIndBrightness = 0;
    // byte mRed1       = 0;
    // byte mGreen1     = 0;
    // byte mBlue1      = 0;
    // byte mPattern    = 0;
    // byte mRate       = 0;
    // byte mMapping    = 0;
    // byte mRed2       = 0;
    // byte mGreen2     = 0;
    // byte mBlue2      = 0;

    mIndBrightness  = currentCommandBuf[ (DANCERNUMBER - 1) * 10 + 1 ] / 255.0;
    mRed1           = currentCommandBuf[ (DANCERNUMBER - 1) * 10 + 2 ];
    mGreen1         = currentCommandBuf[ (DANCERNUMBER - 1) * 10 + 3 ];
    mBlue1          = currentCommandBuf[ (DANCERNUMBER - 1) * 10 + 4 ];
    mPattern        = currentCommandBuf[ (DANCERNUMBER - 1) * 10 + 5 ];
    mRate           = currentCommandBuf[ (DANCERNUMBER - 1) * 10 + 6 ];
    mMapping        = currentCommandBuf[ (DANCERNUMBER - 1) * 10 + 7 ];
    mRed2           = currentCommandBuf[ (DANCERNUMBER - 1) * 10 + 8 ];
    mGreen2         = currentCommandBuf[ (DANCERNUMBER - 1) * 10 + 9 ];
    mBlue2          = currentCommandBuf[ (DANCERNUMBER - 1) * 10 + 10 ];


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

    // Serial.print(     mIndBrightness     );
    // Serial.print(",");
    // Serial.print(     mRed1              );
    // Serial.print(",");
    // Serial.print(     mGreen1            );
    // Serial.print(",");
    // Serial.print(     mBlue1             );
    // Serial.print(",");
    // Serial.print(     patternByte        );
    // Serial.print(",");
    // Serial.print(     mRate              );
    // Serial.print(",");
    // Serial.print(     mMapping           );
    // Serial.print(",");
    // Serial.print(     mRed2              );
    // Serial.print(",");
    // Serial.print(     mGreen2            );
    // Serial.print(",");
    // Serial.println(     mBlue2             );


    // Serial.println(currentCommandBuf[ 14 ]);

    //TODO: find where heartBytes live in buffer
    // heartByte1 = currentCommandBuf[ READBUFFERSIZE - 4 ];
    // heartByte2 = currentCommandBuf[ READBUFFERSIZE - 3 ];
    // heartByte3 = currentCommandBuf[ READBUFFERSIZE - 2 ];

    // currentTime = myColor(heartByte1,heartByte2,heartByte3) * 10;

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


    //   for(int k = 0 ; k < 11; k++){
    //   Serial.print(currentCommandBuf[k]);
    //   Serial.print(",");
    // }
    // Serial.println();

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

}

unsigned long realTime;
unsigned long adjustedTime;

void loop() {

  // unsigned long startMillis = millis();

  read();

  if (isOff) {
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

  // Serial.println(millis()-startMillis);
  // prevMillis = startMillis;

}

/* Helper functions */

//Input a rateue 0 to 384 to get a color rateue.
//The colours are a transition r - g -b - back to r

void hideAll() {
  for (int i = 0; i < totalLEDs; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
}

void showAll() {
  LEDS.show();
}


