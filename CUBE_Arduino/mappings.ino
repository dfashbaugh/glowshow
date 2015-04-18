int forward(long frame, int i) {
  return i;
}

int backward(long frame, int i) {
  return ledsPerStrip - 1 - i;
}

int peak(long frame, int i) {
  if (i < ledsPerStrip / 2) {
    return i;
  } else { 
    return ledsPerStrip - 1 - i;
  }
}

int valley(long frame, int i) {
  if (i < ledsPerStrip / 2) {
    return ledsPerStrip / 2 - 1 - i;
  } else { 
    return i;
  }
}

int dither(long frame, int i) {
  if (i % 2 == 0) {
    return ledsPerStrip - 1 - i;
  } else { 
    return i;
  }
}
