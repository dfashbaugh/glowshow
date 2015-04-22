int stripLengthFront[24] = {3, 3, 7, 4, 8, 5, 10, 10, 10, 10, 8, 9, 8, 8, 9, 9, 10, 10, 7, 9, 6, 8, 3, 6};
int stripLengthBack1[10] = {4,7,10,10,11,10,10,10,10,10};       //92 pixels
int stripLengthBack2[10] = {5,7,10,10,11,10,10,10,10,10};       //93 pixels


int taylorTopLongY[] = { } ;
int taylorTopLongX[] = { } ;


int taylorTopCropY[] = {

    3
    ,5
    ,7

    ,6
    ,4
    ,2

    ,3
    ,5
    ,7

    ,6
    ,4
    ,2

    ,3
    ,5
    ,7

    ,6
    ,4
    ,2

    ,3
    ,5
    ,7

    ,6
    ,4
    ,2

    ,3
    ,5
    ,7

    ,6
    ,4
    ,2

    ,3
    ,5
    ,7
    ,7
    ,5
    ,3
    ,3
    ,5
    ,7

    ,6
    ,4
    ,2

    ,2
    ,4
    ,6
    ,8

    ,7
    ,5
    ,3
    ,1

    ,1
    ,3
    ,5
    ,7

    ,7
    ,5
    ,3
    ,1

    ,1
    ,3
    ,5

    ,9
    ,7
    ,5
    ,3
    ,1

    ,2
    ,4
    ,6
    ,8

    ,9
    ,7
    ,5
    ,3
    ,1

    ,2
    ,4
    ,6
    ,8

    ,9
    ,7
    ,5
    ,3
    ,1

    ,2
    ,4
    ,6
    ,8

    ,9
    ,7
    ,5
    ,3
    ,1

    ,2
    ,4
    ,6
    ,8

    ,9
    ,7
    ,5
    ,3
    ,1

    ,2
    ,4
    ,6

    ,9
    ,7
    ,5
    ,3
    ,1

    ,2
    ,4
    ,6
    ,8

    ,7
    ,5
    ,3
    ,1

    ,2
    ,4
    ,6
    ,8

    ,7
    ,5
    ,3

    ,2
    ,4
    ,6
    ,8

    ,6       
    ,4       
    ,2       
          
    ,2       
    ,4       
    ,6       
           
    ,7     
    ,5       
    ,3       
           
    ,2       
    ,4       
    ,6       
          
    ,7       
    ,5       
    ,3       
           
    ,2       
    ,4       
    ,6       
          
    ,7     
    ,5       
    ,3       
        
    ,2       
    ,4       
    ,6       
           
    ,7       
    ,5       
    ,3       
           
    ,2       
    ,4       
    ,6   

    ,7       
    ,5       
    ,3
};

int taylorTopCropX [] = {
      3
      ,3
      ,3
      ,3
      ,3
      ,3
      ,3
      ,3
      ,3
      ,3
      ,3
      ,3
      ,3
      ,3
      ,4
      ,4
      ,4
      ,4
      ,3
      ,5
      ,4
      ,5
      ,4
      ,5
      ,4
      ,5
      ,4
      ,5
      ,3
      ,5
      ,4
      ,4
      ,4
      ,3
      ,4
      ,3
      ,3
      ,3
      ,3
      ,3
      ,3
      ,3
      ,3
      ,3
      ,3
      ,3
};


int taylorXsize = sizeof(taylorTopCropX)/sizeof(int);

int taylorYsize = 9;

int forward(long frame, int i) {
  return i;
}

int backward(long frame, int i) {
  return totalLEDs - 1 - i;
}

int peak(long frame, int i) {

if(taylorMapTop == &pixToTaylorTopX){

  if (i < taylorXsize / 2) {
    return i;
  } else { 
    return taylorXsize - 1 - i;
  }
}
else if(taylorMapTop == &pixToTaylorTopY){
  if (i < taylorYsize / 2) {
    return i;
  } else { 
    return taylorYsize - 1 - i;
  }
}
else{

  if (i < totalLEDs / 2) {
    return i;
  } else { 
    return totalLEDs - 1 - i;
  }
}

}

int valley(long frame, int i) {
 
if(taylorMapTop == &pixToTaylorTopX){
  if (i < taylorXsize / 2) {
    return taylorXsize / 2 - i;
  } else { 
    return i + taylorXsize/2 + 4;
  }
}


else if(taylorMapTop == &pixToTaylorTopY){
  if (i <= taylorYsize / 2 + 1 ) {
    return taylorYsize / 2 - i ;
  } else { 
    return i-taylorYsize;
  }
}

 else 
{
  if (i < totalLEDs / 2) {
    return totalLEDs / 2 - 1 - i;
  } else { 
    return i;
  }
}

}

int dither(long frame, int i) {
  if(taylorMapTop == &pixToTaylorTopX){
      if (i % 2 == 0) {
        return taylorXsize - 1 - i;
      } else { 
        return i;
      }
}

else if(taylorMapTop == &pixToTaylorTopY){
 if (i % 2 == 0) {
        return taylorYsize - 1 - i;
      } else { 
        return i;
      }
}

 else 
{
  if (i % 2 == 0) {
    return totalLEDs - 1 - i;
  } else { 
    return i;
  }
}
}



int grid(long frame, int i) {

  // Is this an odd column?
  if (i % (NUM_ROWS*2) >= NUM_ROWS) {

    // // How many odd colums so far?
    // int o = i / (NUM_ROWS*2);

    // // Fake index and real index add up to this number
    // int s = (NUM_ROWS*3-1) + o*(NUM_ROWS*4);

    // return s - i;

    return (NUM_ROWS*3-1) + (i / (NUM_ROWS*2))*(NUM_ROWS*4) - i;

  }

  return i;

}

int snake(long frame, int i) {
  return i; 
}


int pixToTaylorTopY(long frame, int p){
  return taylorTopCropY[p];
}

int pixToTaylorTopX(long frame, int p){
  int index = 0;
  for(int i = 0; i < sizeof(taylorTopCropX); i++){

    if(p < index + taylorTopCropX[i]  && p > index - 1){
      return i;
    }
    index+=taylorTopCropX[i];
  }

}















