int forward(long frame, int i) {
  return i;
}

int backward(long frame, int i) {
  return totalLEDs - 1 - i;
}

int peak(long frame, int i) {
  if (i < totalLEDs / 2) {
    return i;
  } else { 
    return totalLEDs - 1 - i;
  }
}

int valley(long frame, int i) {
  if (i < totalLEDs / 2) {
    return totalLEDs / 2 - 1 - i;
  } else { 
    return i;
  }
}

int dither(long frame, int i) {
  if (i % 2 == 0) {
    return totalLEDs - 1 - i;
  } else { 
    return i;
  }
}
