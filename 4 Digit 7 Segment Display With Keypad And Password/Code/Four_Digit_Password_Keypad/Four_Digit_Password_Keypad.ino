//Four Digit Password Keypad

/*
Needed libraries
*/

#include <ArxContainer.h>
#include <Keypad.h>

/* SET UP FOR THE DISPLAY */

/*
Needed pins
*/

const int dataPin  = 13;  // 74HC595 pin 8 DS
const int latchPin = 12;  // 74HC595 pin 9 STCP
const int clockPin = 11;   // 74HC595 pin 10 SHCP
const int digit0   = 10;   // 7-Segment pin D4
const int digit1   = 9;   // 7-Segment pin D3
const int digit2   = 8;   // 7-Segment pin D2
const int digit3   = 7;   // 7-Segment pin D1 

/*
Needed variables and lists
*/

//creates the c++ equivalent of a dictionary of needed values
//each string correlates to a number which is an index in the list of binary values
//no zero, code later defaults to zero due to limitations and me wanting to have the Y be unique
//the length of the map is limited, so S is just 5
std::map<String, int> digitOutputs {
  {"1", 1},
  {"2", 2},
  {"3", 3},
  {"4", 4},
  {"5", 5},
  {"6", 6},
  {"7", 7},
  {"8", 8},
  {"9", 9},
  {"A", 10},
  {"B", 11},
  {"C", 12},
  {"D", 13},
  {"E", 14},
  {"N", 15},
  {"Y", 16}
};

//holds all the bytes that create the digits on the display
//may have to change if wired differently
const byte byteArray[] = {
  0b11010111, //0
  0b00010100, //1
  0b11001101, //2
  0b01011101, //3
  0b00011110, //4
  0b01011011, //5
  0b11011011, //6
  0b00010101, //7
  0b11011111, //8
  0b00011111, //9
  0b10011111, //A
  0b11011010, //B
  0b11000011, //C
  0b11011100, //D
  0b11001011, //E
  0b10010111, //N
  0b01011110  //Y
};

byte controlDigits[] = {digit0, digit1, digit2, digit3};
byte displayDigits[] = {0,0,0,0,0};
String userString = "";

//type the code you want here
String correctCode = "4321";

unsigned long onTime = 0;
bool switchView = true;
int correctionCount = 0;
int digitDelay = 50;
int brightness = 90;

/* SET UP FOR KEYPAD */

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {22, 23, 24, 25}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {26, 27, 28, 29}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

void setup() {
  // put your setup code here, to run once:
  pinMode(latchPin,OUTPUT);
  pinMode(clockPin,OUTPUT);
  pinMode(dataPin,OUTPUT);
  for (int x=0; x<4; x++){
      pinMode(controlDigits[x],OUTPUT);
      digitalWrite(controlDigits[x],LOW);  // Turns off the digit  
  }
}

void types(String a) { Serial.println("it's a String"); }
void types(int a) { Serial.println("it's an int"); }
void types(char *a) { Serial.println("it's a char*"); }
void types(float a) { Serial.println("it's a float"); }
void types(bool a) { Serial.println("it's a bool"); }

void DisplaySegments(){
    /* Display will send out all four digits
     * one at a time.  Elegoo kit only has 1 74HC595, so
     * the Arduino will control the digits
     *   displayDigits[4] = the right nibble controls output type
     *                      1 = raw, 0 = table array
     *                  upper (left) nibble ignored
     *                  starting with 0, the least-significant (rightmost) bit
     */
    
    for (int x=0; x<4; x++){
        for (int j=0; j<4; j++){
            digitalWrite(controlDigits[j],LOW);    // turn off digits
        }
        digitalWrite(latchPin,LOW);
        if (bitRead(displayDigits[4], x)==1){
            // raw byte value is sent to shift register
            shiftOut(dataPin,clockPin,MSBFIRST,displayDigits[x]);
        } else {
            // table array value is sent to the shift register
            shiftOut(dataPin,clockPin,MSBFIRST,byteArray[displayDigits[x]]);
        }
        
        digitalWrite(latchPin,HIGH);
        digitalWrite(controlDigits[x],HIGH);   // turn on one digit
        delay(1);                              // 1 or 2 is ok
    }
    for (int j=0; j<4; j++){
        digitalWrite(controlDigits[j],LOW);    // turn off digits
    }
}

void RawDisplay(String typedString) {
    for(int x =0; x<5; x++){ displayDigits[x]=0; } //reset before outputing
    for (int c = 0; c < typedString.length(); c++) {
      char keyChar = typedString[c];
      String keyString = "";
      keyString += keyChar;
      if (digitOutputs.find(keyString) != digitOutputs.end()) { //check for if the keys exists to prevent overwriting values
        int index = digitOutputs[keyString];
        displayDigits[c] = byteArray[index];
      } else {
        displayDigits[c] = byteArray[0];
      }
    }
    // Set digitSwitch option
    displayDigits[4] = 0b1111;
    if (switchView == true) {
      switchView = !switchView;
      for(int x =0; x<5; x++){ displayDigits[x]=0; }        // Reset array
      displayDigits[4] = 0b0000;
    }
}

void loop() {
  DisplaySegments();
  if (correctionCount == 1) {
    userString = "";
    correctionCount -= 1;
  } else if (correctionCount == 0) {
    char userInput = customKeypad.getKey();
    if (userInput) {
      if (userInput == '*') {
        userString.remove(0, 1);
      } else if (userInput == '#') {
        if (userString == correctCode) {
          userString = "5EY";
          correctionCount = 50;
        } else {
          userString = "ON";
          correctionCount = 50;
        }
      } else {
        if (userString.length() < 4) {
          userString = userInput + userString;
        }
      }
    }
  }
  
  delayMicroseconds(1638*((100-brightness)/10));         // largest value 16383
  
  unsigned long nowValue = millis() - onTime;
  if (nowValue >= long(digitDelay)){
      if (correctionCount > 1) {
        correctionCount--;
      }
      onTime = millis();
      RawDisplay(userString);
  }
}