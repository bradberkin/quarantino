#include <SPI.h>

/* 
 * DIGITS
 */
const int ZERO[5][8] = {{0,1,1,1,0,0,0,0},{0,1,0,1,0,0,0,0},{0,1,0,1,0,0,0,0},{0,1,0,1,0,0,0,0},{0,1,1,1,0,0,0,0}};
const int ONE[5][8] = {{0,0,0,1,0,0,0,0},{0,0,0,1,0,0,0,0},{0,0,0,1,0,0,0,0},{0,0,0,1,0,0,0,0},{0,0,0,1,0,0,0,0}};
const int TWO[5][8] = {{0,1,1,1,0,0,0,0},{0,0,0,1,0,0,0,0},{0,1,1,1,0,0,0,0},{0,1,0,0,0,0,0,0},{0,1,1,1,0,0,0,0}};
const int THREE[5][8] = {{0,1,1,1,0,0,0,0},{0,0,0,1,0,0,0,0},{0,0,1,1,0,0,0,0},{0,0,0,1,0,0,0,0},{0,1,1,1,0,0,0,0}};
const int FOUR[5][8] = {{0,1,0,1,0,0,0,0},{0,1,0,1,0,0,0,0},{0,1,1,1,0,0,0,0},{0,0,0,1,0,0,0,0},{0,0,0,1,0,0,0,0}};
const int FIVE[5][8] = {{0,1,1,1,0,0,0,0},{0,1,0,0,0,0,0,0},{0,1,1,1,0,0,0,0},{0,0,0,1,0,0,0,0},{0,1,1,1,0,0,0,0}};
const int SIX[5][8] = {{0,1,1,1,0,0,0,0},{0,1,0,0,0,0,0,0},{0,1,1,1,0,0,0,0},{0,1,0,1,0,0,0,0},{0,1,1,1,0,0,0,0}};
const int SEVEN[5][8] = {{0,1,1,1,0,0,0,0},{0,0,0,1,0,0,0,0},{0,0,0,1,0,0,0,0},{0,0,0,1,0,0,0,0},{0,0,0,1,0,0,0,0}};
const int EIGHT[5][8] = {{0,1,1,1,0,0,0,0},{0,1,0,1,0,0,0,0},{0,1,1,1,0,0,0,0},{0,1,0,1,0,0,0,0},{0,1,1,1,0,0,0,0}};
const int NINE[5][8] = {{0,1,1,1,0,0,0,0},{0,1,0,1,0,0,0,0},{0,1,1,1,0,0,0,0},{0,0,0,1,0,0,0,0},{0,0,0,1,0,0,0,0}};

const int bufferRows = 8;
const int bufferCols = 16; // second matrix spans columns 1-8; first spans 9-16
const int firstMatrixFirstCol = 9;
const int secondMatrixFirstCol = 1;
int buffer[bufferRows][bufferCols];

/* 
 * MAX7219
 * Two cascaded MAX7219's for driving two LED matrices
 * First matrix displays ones and tens; second displays hundreds (and thousands eventually)
 * Write to second matrix first, then to first matrix
 */
const long MAX_CLOCK = 10000000;

// Register addresses
const byte SCAN_REGISTER_ADDRESS = 0x0B;
const byte DECODE_REGISTER_ADDRESS = 0x09;
const byte SHUTDOWN_REGISTER_ADDRESS = 0x0C;
const byte INTENSITY_REGISTER_ADDRESS = 0x0A;
const byte DISPLAY_TEST_REGISTER_ADDRESS = 0x0F;
const byte NOOP_ADDRESS = 0x00;
const byte COL1_ADDRESS = 0x01;
const byte COL2_ADDRESS = 0x02;
const byte COL3_ADDRESS = 0x03;
const byte COL4_ADDRESS = 0x04;
const byte COL5_ADDRESS = 0x05;
const byte COL6_ADDRESS = 0x06;
const byte COL7_ADDRESS = 0x07;
const byte COL8_ADDRESS = 0x08;
const byte* COLS[] = {&COL1_ADDRESS, &COL2_ADDRESS, &COL3_ADDRESS, &COL4_ADDRESS, &COL5_ADDRESS, &COL6_ADDRESS, &COL7_ADDRESS, &COL8_ADDRESS};

const byte NO_DECODE = 0x00;
const byte SCAN_ALL_DIGITS = 0x07;
const byte SCAN_FIVE_DIGITS = 0x04;
const byte NORMAL_MODE = 0x01; // Used for shutdown mode
const byte TEST_DISPLAY_MODE = 0x01; // Used for display test mode
const byte FIFTEEN = 0x07;

// Counting starts when startPin (switch) goes high (is switched on)
const int startPin = A0;
int startVal = 0;

const int clearPin = 2;
bool clearFirst = false;

/* Time */
unsigned long last = 0;
unsigned long now = 0;
unsigned long elapsed = 0;
unsigned long period = 60000;

// Initial count. Set to future date and throw start switch when ready to begin counting.
unsigned long days = 89;
unsigned long hours = 8;
unsigned long minutes = 5;
unsigned long seconds = 0;

const int HUNDREDS = 2;
const int TENS = 1;
const int ONES = 0;

/* MAX7219 */
void setScanLimit(byte val) { 
  SPI.beginTransaction(SPISettings(MAX_CLOCK, MSBFIRST, SPI_MODE0));
  digitalWrite(SS, LOW);
  SPI.transfer(SCAN_REGISTER_ADDRESS);
  SPI.transfer(val);
  SPI.transfer(SCAN_REGISTER_ADDRESS);
  SPI.transfer(val);
  digitalWrite(SS, HIGH);
  SPI.endTransaction();
}

void setDecodeMode(byte val) {
  SPI.beginTransaction(SPISettings(MAX_CLOCK, MSBFIRST, SPI_MODE0));
  digitalWrite(SS, LOW);
  SPI.transfer(DECODE_REGISTER_ADDRESS);
  SPI.transfer(val);
  SPI.transfer(DECODE_REGISTER_ADDRESS);
  SPI.transfer(val);
  digitalWrite(SS, HIGH);
  SPI.endTransaction();
}

void setShutdownMode(byte val) {
  SPI.beginTransaction(SPISettings(MAX_CLOCK, MSBFIRST, SPI_MODE0));
  digitalWrite(SS, LOW);
  SPI.transfer(SHUTDOWN_REGISTER_ADDRESS);
  SPI.transfer(val);
  SPI.transfer(SHUTDOWN_REGISTER_ADDRESS);
  SPI.transfer(val);
  digitalWrite(SS, HIGH);
  SPI.endTransaction();
}

void setDisplayTestMode(byte val) {
  SPI.beginTransaction(SPISettings(MAX_CLOCK, MSBFIRST, SPI_MODE0));
  digitalWrite(SS, LOW);
  SPI.transfer(DISPLAY_TEST_REGISTER_ADDRESS);
  SPI.transfer(val);
  SPI.transfer(DISPLAY_TEST_REGISTER_ADDRESS);
  SPI.transfer(val);
  digitalWrite(SS, HIGH);
  SPI.endTransaction();
}

void setIntensity(byte val) {
  SPI.beginTransaction(SPISettings(MAX_CLOCK, MSBFIRST, SPI_MODE0));
  digitalWrite(SS, LOW);
  SPI.transfer(INTENSITY_REGISTER_ADDRESS);
  SPI.transfer(val);
  SPI.transfer(INTENSITY_REGISTER_ADDRESS);
  SPI.transfer(val);
  digitalWrite(SS, HIGH);
  SPI.endTransaction();
}

/*
 * Digits
 * Copies a digit to buffer at specified row and col
 * Rows and cols not zero-indexed
 */
void copy(const int digit[][8], int row, int col) {
  for (int r=0; r<5 & r+row <= bufferRows; r++) {
    for (int c=0; c<4 and c+col <= bufferCols; c++) { // Only copy first 4 values of digit (second 4 are zero)
      buffer[row-1+r][col-1+c] = digit[r][c];
    }
  }
}

void setup() {
  pinMode(startPin, INPUT);
  pinMode(clearPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(clearPin), clearInterrupt, LOW);
  pinMode(MOSI, OUTPUT); // D11 on Arduino Uno
  pinMode(SCK, OUTPUT); // D13 on Arduino Uno
  pinMode(SS, OUTPUT); // D10 on Arduino Uno
  
  //Serial.begin(9600);
  
  /* MAX7219 */
  setScanLimit(SCAN_ALL_DIGITS);
  setDecodeMode(NO_DECODE);
  setShutdownMode(NORMAL_MODE);
  setIntensity(FIFTEEN);

  clearBuffer();
  clearMatrices();

  last = millis();
}

void clearInterrupt() {
  clearFirst = true;
}

void clearBuffer() {
  for (int r=0; r<bufferRows; r++) {
    for (int c=0; c<bufferCols; c++) {
      buffer[r][c] = 0;
    }
  }
}

void printBuffer() {
  for (int r=0; r<bufferRows; r++) {
    for (int c=0; c<bufferCols; c++) {
      Serial.print(buffer[r][c]);
    }
    Serial.println();
  }
}

void clearFirstMatrix() {
  for (int r=0; r<bufferRows; r++) {
    SPI.beginTransaction(SPISettings(MAX_CLOCK, MSBFIRST, SPI_MODE0));
    digitalWrite(SS, LOW);
    
    // No op to second matrix
    SPI.transfer(NOOP_ADDRESS);
    SPI.transfer(0);

    // Zeroes to first matrix
    SPI.transfer(*COLS[r]);
    SPI.transfer(0);
 
    digitalWrite(SS, HIGH);
    SPI.endTransaction();
  }
}

void clearSecondMatrix() {
  for (int r=0; r<bufferRows; r++) {
    SPI.beginTransaction(SPISettings(MAX_CLOCK, MSBFIRST, SPI_MODE0));
    digitalWrite(SS, LOW);
    
    // Zeroes to second matrix
    SPI.transfer(*COLS[r]);
    SPI.transfer(0);
    
    // No op to first matrix
    SPI.transfer(NOOP_ADDRESS);
    SPI.transfer(0);

    digitalWrite(SS, HIGH);
    SPI.endTransaction();
  }
}

void clearMatrices() {
  for (int r=0; r<bufferRows; r++) {
    SPI.beginTransaction(SPISettings(MAX_CLOCK, MSBFIRST, SPI_MODE0));
    digitalWrite(SS, LOW);
    
    // Zeroes to second matrix
    SPI.transfer(*COLS[r]);
    SPI.transfer(0);
    
    // Zeroes to first matrix
    SPI.transfer(*COLS[r]);
    SPI.transfer(0);

    digitalWrite(SS, HIGH);
    SPI.endTransaction();
  }
}

void scanFirstMatrix() {
  int val, a;
  
  for (int r=0; r<bufferRows; r++) {
    val = 0;
    // Convert int array of 1's and 0's into decimal value
    for (int c=7; c>-1; c--) {
      a = (buffer[r][firstMatrixFirstCol+7-c-1]) << (7-c);
      val = val + a;
    }
    SPI.beginTransaction(SPISettings(MAX_CLOCK, MSBFIRST, SPI_MODE0));
    digitalWrite(SS, LOW);
    
    // No op to second matrix
    SPI.transfer(NOOP_ADDRESS);
    SPI.transfer(0x00);
    
    // Copy buffer value to first matrix
    SPI.transfer(*COLS[r]);
    SPI.transfer(val);
    
    digitalWrite(SS, HIGH);
    SPI.endTransaction();
  }
}

void scanSecondMatrix() {
  int val, a;
  
  for (int r=0; r<bufferRows; r++) {
    val = 0;
    // Convert int array of 1's and 0's into decimal value
    for (int c=7; c>-1; c--) {
      a = (buffer[r][secondMatrixFirstCol+7-c-1]) << (7-c);
      val = val + a;
    }
    SPI.beginTransaction(SPISettings(MAX_CLOCK, MSBFIRST, SPI_MODE0));
    digitalWrite(SS, LOW);
    
    // Copy buffer value to second matrix
    SPI.transfer(*COLS[r]);
    SPI.transfer(val);
    
    // No op to first matrix
    SPI.transfer(NOOP_ADDRESS);
    SPI.transfer(0x00);
    
    digitalWrite(SS, HIGH);
    SPI.endTransaction();
  }
}

void scanMatrices() {
  int secondVal, firstVal, a, b;
  
  for (int r=0; r<bufferRows; r++) {
    secondVal = 0;
    firstVal = 0;
    a = 0;
    b = 0;
    // Convert int array of 1's and 0's into decimal value
    for (int c=7; c>-1; c--) {
      a = (buffer[r][secondMatrixFirstCol+7-c-1]) << (7-c);
      b = (buffer[r][firstMatrixFirstCol+7-c-1]) << (7-c);
      secondVal = secondVal + a;
      firstVal = firstVal + b;
    }
    
    SPI.beginTransaction(SPISettings(MAX_CLOCK, MSBFIRST, SPI_MODE0));
    digitalWrite(SS, LOW);
    
    // Copy buffer value to second matrix
    SPI.transfer(*COLS[r]);
    SPI.transfer(secondVal);
    // Copy buffer value to first matrix
    SPI.transfer(*COLS[r]);
    SPI.transfer(firstVal);
    
    digitalWrite(SS, HIGH);
    SPI.endTransaction();
  }
}

void loop() {
  if (analogRead(startPin) < 300) {
    ;
  }
  else {
    if (clearFirst) {
      clearFirst = false;
      clearMatrices();
    }
    
    now = millis();
    elapsed = now-last;
    if (elapsed > period) {
        last = now;
        updateTime(elapsed);
        scanMatrices();
    }
  }
}

void updateBuffer(int val, int position) {
  int row = 2; //all digits start at row 2
  int col = 0;
  if (position == HUNDREDS) {
    col = 5;
  } else if (position == TENS) {
    col = 9;
  } else if (position == ONES) {
    col = 13;
  }
  
  if (val == 1) {
    copy(ONE, row, col);
  } else if (val == 2) {
    copy(TWO, row, col);
  } else if (val == 3) {
    copy(THREE, row, col);
  } else if (val == 4) {
    copy(FOUR, row, col);
  } else if (val == 5) {
    copy(FIVE, row, col);
  } else if (val == 6) {
    copy(SIX, row, col);
  } else if (val == 7) {
    copy(SEVEN, row, col);
  } else if (val == 8) {
    copy(EIGHT, row, col);
  } else if (val == 9) {
    copy(NINE, row, col);
  } else if (val == 0) {
    copy(ZERO, row, col);
  }
}

void updateTime(unsigned long elapsed) {
  long elapsedSeconds = elapsed/1000;
  long elapsedMinutes = elapsedSeconds/60;
  elapsedSeconds = elapsedSeconds%60;
  long elapsedHours = elapsedMinutes/60;
  elapsedMinutes = elapsedMinutes%60;
  long elapsedDays = elapsedHours/24;
  elapsedHours = elapsedHours%24;

  minutes += (elapsedSeconds+seconds)/60;
  seconds = (elapsedSeconds+seconds)%60;
  
  hours += (elapsedMinutes+minutes)/60;
  minutes = (elapsedMinutes+minutes)%60;

  days += (elapsedHours+hours)/24;
  hours = (elapsedHours+hours)%24;

  /*Serial.print(days);
  Serial.print(".");
  Serial.print(hours);
  Serial.print(".");
  Serial.print(minutes);
  Serial.print(".");
  Serial.println(seconds);*/

  updateBuffer(days/100,HUNDREDS);
  int hModulo = days%100;
  updateBuffer(hModulo/10,TENS);
  updateBuffer(hModulo%10,ONES);
}
