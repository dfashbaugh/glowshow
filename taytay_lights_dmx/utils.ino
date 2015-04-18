uint8_t red(uint32_t c) {
  return ((c & 0xFF0000) >> 16);
}
uint8_t green(uint32_t c) {
  return ((c & 0x00FF00) >> 8);
}
uint8_t blue(uint32_t c) {
  return (c & 0x0000FF);
}

byte lerp(byte a, byte b, float t) {
  return a + (b - a)*t; 
}

float triangleWave(int frame, float rate) {
  if ((int)(frame / rate) % 2 == 0) {
    return (frame % (int)rate / rate);
  } 
  else { 
    return 1 - (frame % (int)rate / rate);
  }
}


uint32_t lerpColor(uint32_t a, uint32_t b, float t) {
  return myColor(lerp(red(a), red(b), t), 
  lerp(green(a), green(b), t), 
  lerp(blue(a), blue(b), t));
}

uint32_t Wheel(uint16_t WheelPos) {

  WheelPos %= 384;
  byte r, g, b;
  switch(WheelPos / 128) {
  case 0:
    r = 127 - WheelPos % 128;		//Red down
    g = WheelPos % 128;			 // Green up
    b = 0;									//blue off
    break; 
  case 1:
    g = 127 - WheelPos % 128;	 //green down
    b = WheelPos % 128;			 //blue up
    r = 0;									//red off
    break; 
  case 2:
    b = 127 - WheelPos % 128;	 //blue down 
    r = WheelPos % 128;			 //red up
    g = 0;									//green off
    break; 
  }
  return myColor(r,g,b);
}
uint32_t myColor(uint8_t r , uint8_t g , uint8_t b){
  return ((uint32_t)(r) << 16) | ((uint32_t)(g ) <<  8) | (b );
}

