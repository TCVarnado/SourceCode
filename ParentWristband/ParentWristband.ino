/*
 * Taylor Varnado & NAP Monitor Team
 * 
 * Parent Wristband Code 
 */


//Including the Library for Wireless Communication
#include <SoftwareSerial.h>

//Initializing Transceivers with Transmit and Receive Pins
SoftwareSerial HC12(10,11);

//Setting Output Pins
int good = 2;         //Green LED Indicating Idle State
int bad = 3;          //Red LED Indicating Alarm State
int vibrate = 9;      //Vibrating Motor Indicating Alarm State


//Setting up the Code to Run and Intialzing OUTPUT Pins -> Only Runs Once
void setup() {

  //Initialzing the Baud Rate for the Serial Port and HC12 Wireless Transceiver
  Serial.begin(9600);
  HC12.begin(9600);

  //Setting the Pins to OUTPUTS
  pinMode(good,OUTPUT);
  pinMode(bad,OUTPUT);
  pinMode(vibrate,OUTPUT);

  //Initializng the Values for the OUTPUT Pins
  digitalWrite(good,HIGH);                    //Turns On
  digitalWrite(bad,LOW);                      //Turns Off
  digitalWrite(vibrate,LOW);                  //Turns Off
}

//Defining the States for the State Machine
typedef enum
{
  STATE_IDLE,
  STATE_ALARM,
} state_t;

//Update State Function to Navigate Between States
void update_state(void)
{
  //Initializng the Begining State
  static state_t e_state = STATE_IDLE;
  switch (e_state)
  {

    //State Idle -> Stays Here until a Bad Signal from the Home Base is Received
    case STATE_IDLE:
    
        while (HC12.available()) {       // If HC-12 has data
         Serial.write(HC12.read());      // Send the data to Serial monitor 
         e_state = STATE_ALARM;          // Travel to Alarm State
        }
        break;

    //Alarm State -> Stays Here until a Good Signal from the Home Base is Received
    case STATE_ALARM:

      //Sets the Green LED to Low -> Turns Off
      digitalWrite(good,LOW);

      //Traps in a Loop to Vibrate the Motor and Flash the Red LED
      while ( 1 )
      {

        //Sets the LED and Vibrating Motor to High -> Turns On
        digitalWrite(bad, HIGH);
        digitalWrite(vibrate, HIGH);

        //Waits Before Turning Off the Motor and Red LED for .1 Seconds -> 100 ms
        delay(250);

        //Sets the LED and Motor to Low -> Turns Off
        digitalWrite(bad,LOW);
        digitalWrite(vibrate,LOW);

        //If the HC12 Wireless Transceiver Picks up a Good Signal from the Home Base
        while (HC12.available())
        {
          Serial.write(HC12.read());      // Send the data to Serial monitor 

          //Sets the Green LED back to High, Turning it On, when the Good Signal is Received
          digitalWrite(good,HIGH);

          //Traverse back to the Idle State
          e_state = STATE_IDLE; 
        }

        //If the Motor and LED are Still Off before the signal is Received,
        //Turns back HIGH at the begining of the Loop at a Delay of .1 Seconds -> 100ms
        delay(250);
        break;
      }
  }
}

void loop() {
  //Updates the State at the begining of the Code Running
  update_state();
}
