#include <SPI.h>

/* DIGITS */
const PROGMEM int ZERO[5][8] = {{0,1,1,1,0,0,0,0},{0,1,0,1,0,0,0,0},{0,1,0,1,0,0,0,0},{0,1,0,1,0,0,0,0},{0,1,1,1,0,0,0,0}};
const PROGMEM int ONE[5][8] = {{0,0,0,1,0,0,0,0},{0,0,0,1,0,0,0,0},{0,0,0,1,0,0,0,0},{0,0,0,1,0,0,0,0},{0,0,0,1,0,0,0,0}};
const PROGMEM int TWO[5][8] = {{0,1,1,1,0,0,0,0},{0,0,0,1,0,0,0,0},{0,1,1,1,0,0,0,0},{0,1,0,0,0,0,0,0},{0,1,1,1,0,0,0,0}};
const PROGMEM int THREE[5][8] = {{0,1,1,1,0,0,0,0},{0,0,0,1,0,0,0,0},{0,0,1,1,0,0,0,0},{0,0,0,1,0,0,0,0},{0,1,1,1,0,0,0,0}};
const PROGMEM int FOUR[5][8] = {{0,1,0,1,0,0,0,0},{0,1,0,1,0,0,0,0},{0,1,1,1,0,0,0,0},{0,0,0,1,0,0,0,0},{0,0,0,1,0,0,0,0}};
const PROGMEM int FIVE[5][8] = {{0,1,1,1,0,0,0,0},{0,1,0,0,0,0,0,0},{0,1,1,1,0,0,0,0},{0,0,0,1,0,0,0,0},{0,1,1,1,0,0,0,0}};
const PROGMEM int SIX[5][8] = {{0,1,1,1,0,0,0,0},{0,1,0,0,0,0,0,0},{0,1,1,1,0,0,0,0},{0,1,0,1,0,0,0,0},{0,1,1,1,0,0,0,0}};
const PROGMEM int SEVEN[5][8] = {{0,1,1,1,0,0,0,0},{0,0,0,1,0,0,0,0},{0,0,0,1,0,0,0,0},{0,0,0,1,0,0,0,0},{0,0,0,1,0,0,0,0}};
const PROGMEM int EIGHT[5][8] = {{0,1,1,1,0,0,0,0},{0,1,0,1,0,0,0,0},{0,1,1,1,0,0,0,0},{0,1,0,1,0,0,0,0},{0,1,1,1,0,0,0,0}};
const PROGMEM int NINE[5][8] = {{0,1,1,1,0,0,0,0},{0,1,0,1,0,0,0,0},{0,1,1,1,0,0,0,0},{0,0,0,1,0,0,0,0},{0,0,0,1,0,0,0,0}};

const int bufferRows = 8;
const int bufferCols = 16;
int buffer[bufferRows][bufferCols];

/* MAX7219 */
const long MAX_CLOCK = 10000000;
const byte SCAN_REGISTER_ADDRESS = 0x0B;
const byte DECODE_REGISTER_ADDRESS = 0x09;
const byte SHUTDOWN_REGISTER_ADDRESS = 0x0C;
const byte INTENSITY_REGISTER_ADDRESS = 0x0A;

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
const byte NORMAL_MODE = 0x01;
const byte FIFTEEN = 0x07;

const int startPin = A0;
int startVal = 0;

const int clearPin = 2;
bool clearFirst = false;

/* Time */
unsigned long last = 0;
unsigned long now = 0;
unsigned long period = 60000;

unsigned long days = 25;
unsigned long hours = 2;
unsigned long minutes = 55;
unsigned long seconds = 0;

const int TENS = 0;
const int ONES = 1;

/* MAX7219 */
void setScanLimit(byte val) { 
  SPI.beginTransaction(SPISettings(MAX_CLOCK, MSBFIRST, SPI_MODE0));
  digitalWrite(SS, LOW);
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
  digitalWrite(SS, HIGH);
  SPI.endTransaction();
}

void setShutdownMode(byte val) {
  SPI.beginTransaction(SPISettings(MAX_CLOCK, MSBFIRST, SPI_MODE0));
  digitalWrite(SS, LOW);
  SPI.transfer(SHUTDOWN_REGISTER_ADDRESS);
  SPI.transfer(val);
  digitalWrite(SS, HIGH);
  SPI.endTransaction();
}

void setIntensity(byte val) {
  SPI.beginTransaction(SPISettings(MAX_CLOCK, MSBFIRST, SPI_MODE0));
  digitalWrite(SS, LOW);
  SPI.transfer(INTENSITY_REGISTER_ADDRESS);
  SPI.transfer(val);
  digitalWrite(SS, HIGH);
  SPI.endTransaction();
}

/* Digits */
// Rows and cols not zero-indexed
void copy(int digit[][8], int row, int col) {
  for (int r=0; r<5 & r+row <= bufferRows; r++) {
    for (int c=0; c<4 and c+col <= bufferCols; c++) {
      buffer[row-1+r][col-1+c] = digit[r][c];
    }
  }
}

void setup() {
  pinMode(startPin, INPUT);
  pinMode(clearPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(clearPin), clearInterrupt, LOW);
  pinMode(MOSI, OUTPUT);
  pinMode(SCK, OUTPUT);
  pinMode(SS, OUTPUT);
  
  //Serial.begin(9600);
  
  /* MAX7219 */
  setScanLimit(SCAN_ALL_DIGITS);
  setDecodeMode(NO_DECODE);
  setShutdownMode(NORMAL_MODE);
  setIntensity(FIFTEEN);

  clearBuffer();
  clearFirstMatrix();

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
    SPI.transfer(*COLS[r]);
    SPI.transfer(0);
    digitalWrite(SS, HIGH);
    SPI.endTransaction();
  }
}

void scanFirstMatrix() {
  for (int r=0; r<bufferRows; r++) {
    int val = 0;
    for (int c=7; c>-1; c--) {
      int a = (buffer[r][7-c]) << (7-c);
      val = val + a;
    }
    SPI.beginTransaction(SPISettings(MAX_CLOCK, MSBFIRST, SPI_MODE0));
    digitalWrite(SS, LOW);
    SPI.transfer(*COLS[r]);
    SPI.transfer(val);
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
      clearFirstMatrix();
    }
    
    now = millis();
    if (now-last > period) {
        updateTime(now-last);
        scanFirstMatrix();
        last = now;
    }
  }
}

void updateBuffer(int val, int position) {
  int row = 2; //all digits start at row 2
  int col = 0;
  if (position == TENS) {
    col = 1;
  }
  else {
    col = 5;
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
  
  updateBuffer(days/10,TENS);
  updateBuffer(days%10,ONES);
}
