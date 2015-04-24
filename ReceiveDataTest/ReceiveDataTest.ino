/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */
  #define PACKETSIZE 144
 
// Pin 13 has an LED connected on most Arduino boards.
// Pin 11 has the LED on Teensy 2.0
// Pin 6  has the LED on Teensy++ 2.0
// Pin 13 has the LED on Teensy 3.0
// give it a name:
int led = 13;
char listing [112];


char valuesLastOne [PACKETSIZE];
char valuesLastTwo [PACKETSIZE];

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
	Serial1.begin(115200);
	Serial.begin(115200);
}

// the loop routine runs over and over again forever:
void loop() {
	
  

  if(Serial1.available() >= PACKETSIZE)
  {
	Serial1.readBytes(listing, PACKETSIZE);
	
	// Check if string has valid delimeters
	if(fixInputString())
	{
	
	// Check string order
	//if(checkStringOrder())
	//{
		for(int i = 0; i<PACKETSIZE; i++)
		{
			Serial.print((int)listing[i]);
			Serial.print(',');
		}
	Serial.println();
	//}
	
	memcpy(valuesLastTwo, valuesLastOne, PACKETSIZE);
	memcpy(valuesLastOne, listing, PACKETSIZE);
	}
  }
}

boolean checkStringOrder()
{
	for(int i = 0; i<PACKETSIZE; i++)
	{
		if( !(valuesLastOne[i] == valuesLastTwo[i] == listing[i]) )
		{
			return false;
		}
	}
	
	return true;
}


boolean fixInputString ()
{
  boolean beginFound  = false;
  boolean endFound    = false;
  int beginIndex = 0;
  int endIndex   = 0;

  for(int i = 0; i<PACKETSIZE; i++)
  {
    if(listing[i] == 130)
    {
      beginFound = true;
      beginIndex = i;
    }
    else if(listing[i] == 128)
    {
      endFound = true;
      endIndex = i;
    }
  }

  if(endFound && beginFound && (beginIndex == 0) && (endIndex == PACKETSIZE))
  {
    return true;
  }

  int difference = PACKETSIZE - endIndex;
  byte tempString [PACKETSIZE];
  memcpy(tempString, listing, PACKETSIZE);

  for(int i = 0; i<PACKETSIZE; i++)
  {
      listing[i] = tempString[ (beginIndex+i) %PACKETSIZE ];
  }

  return beginFound && endFound;

}