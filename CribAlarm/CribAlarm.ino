//***************************************************************//
//  Name    : shiftOutCode, Dual One By One                      //
//  Author  : Carlyn Maw, Tom Igoe                               //
//  Date    : 25 Oct, 2006                                       //
//  Version : 1.0                                                //
//  Notes   : Code for using a 74HC595 Shift Register            //
//          : to count from 0 to 255                             //
//***************************************************************//

//Taylor Craig Varnado & NAP Monitor Team

//Adding the Library for Wireless Transceivers
#include <SoftwareSerial.h>

//Initializing the HC12 Transceivers 
SoftwareSerial HC12(10, 11); // HC-12 TX Pin -> 10, HC-12 RX Pin -> 11

//Pin connected to ST_CP of 74HC595
int latchPin = 8;
//Pin connected to SH_CP of 74HC595
int clockPin = 12;
////Pin connected to DS of 74HC595
int dataPin = 9;

//Sets the Speaker to Digital I/O Pin 7
int speaker = 7;

//holder for infromation you're going to pass to shifting function
byte data = 0; 


//Initialzing the States for the State Machine
typedef enum
{
  STATE_IDLE,
  STATE_ALARM,
} state_t;


//Code that Only Runs Once when the Code Starts
void setup() {
  //set pins to output because they are addressed in the main loop
  pinMode(latchPin, OUTPUT);

  //function that blinks all the LEDs
  //gets passed the number of blinks and the pause time
  blinkAll_2Bytes(3,100); 

  //Setting the Baud Rate for the Serial Monitor and the HC12 Wireless Transceiver
  Serial.begin(9600);             // Serial port to computer
  HC12.begin(9600);               // Serial port to HC12
}


//Update State Function that Navigates the State Machine
void update_state(void)
{
  //Initializes Idle State as the Start State
  static state_t e_state = STATE_IDLE;
  switch ( e_state)
  {

    //Idle State -> Sleeps Until a Bad Signal is Received from the Home Base
    case STATE_IDLE:
    
        while (HC12.available()) {       // If HC-12 has data
         Serial.write(HC12.read());      // Send the data to Serial monitor 
         e_state = STATE_ALARM;          // Go to Alarm State
        }
        break;

     //Alarm State -> Alarms Until a Good Signal is Received from the Home Base
     case STATE_ALARM:

        //Traps the Code in a Loop that will Alarm 
        while (1)
        {
            //Sets Speaker Tone Frequency
            tone(speaker, 800);
            
            //Light Function of the Alarm
            // light each pin one by one using a function A
            for (int j = 0; j < 8; j++) {
              //ground latchPin and hold low for as long as you are transmitting
              digitalWrite(latchPin, 0);
              //red LEDs
              lightShiftPinA(7-j);
              //green LEDs
              lightShiftPinA(j);
              //return the latch pin high to signal chip that it 
              //no longer needs to listen for information
              digitalWrite(latchPin, 1);
              delay(50);
            }
              
            //Sets Speaker Tone Frequency
            tone(speaker, 700);

            //Light Function of the Alarm
            // light each pin one by one using a function A
            for (int j = 0; j < 8; j++) {
              //ground latchPin and hold low for as long as you are transmitting
              digitalWrite(latchPin, 0);
              //red LEDs
              lightShiftPinB(j);
              //green LEDs
              lightShiftPinB(7-j);
              //return the latch pin high to signal chip that it 
              //no longer needs to listen for information
              digitalWrite(latchPin, 1);
              delay(50);
            } 

            //Checking for Good Signal from Home Base
            while (HC12.available()) {        // If HC-12 has data
              Serial.write(HC12.read());      // Send the data to Serial monitor

              //Turns Speaker and LEDs Off
              noTone(speaker);
              digitalWrite(latchPin, 0);
              shiftOut(dataPin, clockPin, 0);
              shiftOut(dataPin, clockPin, 0);
              digitalWrite(latchPin, 1);

              //Goes to Idle State
              e_state = STATE_IDLE;
            }
        
        break;
      }
        
      
          
  }
}


void loop() {
  update_state();
}

//This function uses bitwise math to move the pins up
void lightShiftPinA(int p) {
  //defines a local variable
  int pin;

  //this is line uses a bitwise operator
  //shifting a bit left using << is the same
  //as multiplying the decimal number by two. 
  pin = 1<< p;

  //move 'em out
  shiftOut(dataPin, clockPin, pin);   

}

//This function uses that fact that each bit in a byte
//is 2 times greater than the one before it to
//shift the bits higher
void lightShiftPinB(int p) {
  //defines a local variable
  int pin;

  //start with the pin = 1 so that if 0 is passed to this
  //function pin 0 will light. 
  pin = 1;

  for (int x = 0; x < p; x++) {
    pin = pin * 2; 
  }
  //move 'em out
  shiftOut(dataPin, clockPin, pin);   
}


// the heart of the program
void shiftOut(int myDataPin, int myClockPin, byte myDataOut) {
  // This shifts 8 bits out MSB first, 
  //on the rising edge of the clock,
  //clock idles low

  //internal function setup
  int i=0;
  int pinState;
  pinMode(myClockPin, OUTPUT);
  pinMode(myDataPin, OUTPUT);

  //clear everything out just in case to
  //prepare shift register for bit shifting
  digitalWrite(myDataPin, 0);
  digitalWrite(myClockPin, 0);

  //for each bit in the byte myDataOut�
  //NOTICE THAT WE ARE COUNTING DOWN in our for loop
  //This means that %00000001 or "1" will go through such
  //that it will be pin Q0 that lights. 
  for (i=7; i>=0; i--)  {
    digitalWrite(myClockPin, 0);

    //if the value passed to myDataOut and a bitmask result 
    // true then... so if we are at i=6 and our value is
    // %11010100 it would the code compares it to %01000000 
    // and proceeds to set pinState to 1.
    if ( myDataOut & (1<<i) ) {
      pinState= 1;
    }
    else {  
      pinState= 0;
    }

    //Sets the pin to HIGH or LOW depending on pinState
    digitalWrite(myDataPin, pinState);
    //register shifts bits on upstroke of clock pin  
    digitalWrite(myClockPin, 1);
    //zero the data pin after shift to prevent bleed through
    digitalWrite(myDataPin, 0);
  }

  //stop shifting
  digitalWrite(myClockPin, 0);
}


//blinks both registers based on the number of times you want to 
//blink "n" and the pause between them "d"
//starts with a moment of darkness to make sure the first blink
//has its full visual effect.
void blinkAll_2Bytes(int n, int d) {
  digitalWrite(latchPin, 0);
  shiftOut(dataPin, clockPin, 0);
  shiftOut(dataPin, clockPin, 0);
  digitalWrite(latchPin, 1);
  delay(200);
  for (int x = 0; x < n; x++) {
    digitalWrite(latchPin, 0);
    shiftOut(dataPin, clockPin, 255);
    shiftOut(dataPin, clockPin, 255);
    digitalWrite(latchPin, 1);
    delay(d);
    digitalWrite(latchPin, 0);
    shiftOut(dataPin, clockPin, 0);
    shiftOut(dataPin, clockPin, 0);
    digitalWrite(latchPin, 1);
    delay(d);
  }
}
