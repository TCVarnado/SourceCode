/*
  Monitor Display Code
  
  Code written by Taylor Varnado and NAP Monitor team
  
  Mostly Functional 2.0
*/


#include <LiquidCrystal_I2C.h>

#include <SoftwareSerial.h>

SoftwareSerial HC12(10,11);

int totalReads = 0;
int badReads = 0;

//Defining inputs/ouputs
int inc = 25;
int dec = 24;
int nxt = 26;
int reset = 27;
int AT = 9;


//Initializing incoming data from vitals monitor
int heartRate = 100;
int spO2 = 90;
float temp = 70;


//Initializing Min and Max Values
int heartRateMax = 100;
int heartRateMin = 50;
int spO2Min = 90;
int tempMax = 100;
int tempMin = 95;


//Defining the State Machine
typedef enum
{
  STATE_START,
  STATE_HR_MAX,
  STATE_HR_HALF,
  STATE_HR_MIN,
  STATE_HR2SPO2,
  STATE_SPO2_MIN,
  STATE_SPO22TEMP,
  STATE_TEMP_MAX,
  STATE_TEMP_HALF,
  STATE_TEMP_MIN,
  STATE_READ,
  STATE_ALARM,
} state_t;


//Initializing the LCD Screen
LiquidCrystal_I2C lcd(0x3F,20,4);  // set the LCD address to 0x3F for a 20 chars and 4 line display

void setup()
{
  //Push Buttons are INPUTs
  pinMode(inc,INPUT);
  pinMode(dec,INPUT);
  pinMode(nxt,INPUT);
  pinMode(reset,INPUT);
  pinMode(AT,OUTPUT);
  Serial.begin(9600);
  HC12.begin(9600);

  
  digitalWrite(AT,LOW);
  delay(40);
  HC12.write("AT+C001");
  delay(80);
  digitalWrite(AT,HIGH);


  //Printing the initial Message
  lcd.begin();                      // initialize the lcd   
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Heart Rate = ");
  lcd.setCursor(13,0);
  lcd.print(heartRate);  
  lcd.setCursor(0,1);
  lcd.print("SpO2 = ");
  lcd.setCursor(8,1);
  lcd.print(spO2);
  lcd.setCursor(0,2);
  lcd.print("Temp = ");
  lcd.setCursor(7,2);
  lcd.print(temp);
  lcd.setCursor(0,3);
  lcd.print("Press nxt 2 Start");
}


  //Super Duper Fun State Machine
  void update_state(void)
  {
  static state_t e_state = STATE_START;
  switch (e_state)
  {

    //Start Case

    
    case STATE_START:
    if ( digitalRead(nxt) ==  LOW )
    {                                               //Start Case
      lcd.clear();
      e_state = STATE_HR_MAX;
    }
    break;


    
    //Setting the Maximum Heart Rate


    
    case STATE_HR_MAX:                              //HR MAX Case
    
    lcd.setCursor(0,0);
    if ( heartRateMax >= 100 )
    {
      lcd.print("Heart Rate Max = ");
      lcd.setCursor(17,0);
    }
    else
    {
      lcd.print("Heart Rate Max =  ");
      lcd.setCursor(18,0);
    }
    lcd.print(heartRateMax); 
    
    if ( digitalRead(inc) == LOW )
    {
      heartRateMax++;
    }
    if ( digitalRead(dec) == LOW )
    {
      heartRateMax--;
      if ( heartRateMax <= 0)
      {
        heartRateMax = 0;
      }
    }
    if ( digitalRead(nxt) == LOW )
    {
      e_state = STATE_HR_HALF;
    }
    break;
  

    case STATE_HR_HALF:                             //In Between Case
    if ( digitalRead(nxt) == HIGH )
    {
      lcd.clear();
      e_state = STATE_HR_MIN;
    }
    break;



    
    //Setting the Minimum Heart Rate


    
    case STATE_HR_MIN:                             //HR MIN Case
    
    lcd.setCursor(0,0);
    if ( heartRateMin >= 100 )
    {
      lcd.print("Heart Rate Min = ");
      lcd.setCursor(17,0);
    }
    else
    {
      lcd.print("Heart Rate Min =  ");
      lcd.setCursor(18,0);
    } 
    lcd.print(heartRateMin); 
    
    if ( digitalRead(inc) == LOW )
    {
      heartRateMin++;
    }
    if ( digitalRead(dec) == LOW )
    {
      heartRateMin--;
      if ( heartRateMin <= 0 )
      {
        heartRateMin = 0;
      }
    }
    if ( digitalRead(nxt) == LOW )
    {
      e_state = STATE_HR2SPO2;
    }
    break;

    

    case STATE_HR2SPO2:                          //In Between State

    if ( digitalRead(nxt) == HIGH )
    {
      lcd.clear();
      e_state = STATE_SPO2_MIN;
    }
    break;



    
    //Setting the Minimum SpO2 Level


    
    case STATE_SPO2_MIN:                        //SPO2 MIN Case
    
    lcd.setCursor(0,1);

    if ( spO2Min >= 100 )
    {
      lcd.print("SpO2 Min = ");
      lcd.setCursor(11,1);
    }

    else
    {
      lcd.print("SpO2 Min =  ");
      lcd.setCursor(12,1);
    }
    lcd.print(spO2Min); 
    if ( digitalRead(inc) == LOW )
    {
      spO2Min++;
      if ( spO2Min >= 100 )
      {
        spO2Min = 100;
      }
    }
    if ( digitalRead(dec) == LOW )
    {
      spO2Min--;
      if ( spO2Min <= 0 )
      {
        spO2Min = 0;
      }
    }
    if ( digitalRead(nxt) == LOW )
    {
      e_state = STATE_SPO22TEMP;
    }
    break;


    

    case STATE_SPO22TEMP:                       //In Between State
    if ( digitalRead(nxt) == HIGH )
    {
      lcd.clear();
      e_state = STATE_TEMP_MAX;
    }
    break;



    
    //Setting the Maximum Temperature


      
    case STATE_TEMP_MAX:                        //MAX TEMP STATE
    
    lcd.setCursor(0,2);
    if ( tempMax >= 100 )
    {
      lcd.print("Temp. Max = ");
      lcd.setCursor(12,2);
    }
    else 
    {
      lcd.print("Temp. Max =  ");
      lcd.setCursor(13,2);
    }
    lcd.print(tempMax); 

    
    if ( digitalRead(inc) == LOW )
    {
      tempMax++;
    }
    
    if ( digitalRead(dec) == LOW )
    {
      tempMax--;
      if ( tempMax <= 0 )
      {
        tempMax = 0;
      }
    }
    if ( digitalRead(nxt) == LOW )
    {
      e_state = STATE_TEMP_HALF;
    }
    break;


    

    case STATE_TEMP_HALF:                     //In Between State
    if ( digitalRead(nxt) ==  HIGH )
    {
      lcd.clear();
      e_state = STATE_TEMP_MIN;
    }
    break;



    
    
    //Setting the Minimum Temperature


    
    case STATE_TEMP_MIN:                       //MIN TEMP State
    
    lcd.setCursor(0,2);
    if ( tempMin >= 100 )
    {
      lcd.print("Temp. Min = ");
      lcd.setCursor(12,2);
    }
    else 
    {
      lcd.print("Temp. Min =  ");
      lcd.setCursor(13,2);
    }
    lcd.print(tempMin); 

    
    if ( digitalRead(inc) == LOW )
    {
      tempMin++;
    }
    if ( digitalRead(dec) == LOW )
    {
      tempMin--;
      if ( tempMin <= 0 )
      {
        tempMin = 0;
      }
    }
    if ( digitalRead(nxt) == LOW )
    {
      lcd.clear();
      e_state = STATE_READ;
    }
    break;

    case STATE_READ:                                      //read state
    while (HC12.available())   // If HC-12 has data
    {        
      heartRate = HC12.read();
      Serial.print("\nHR = ");
      Serial.print(heartRate);
      
      spO2 = HC12.read();      // Send the data to Serial monitor
      Serial.print("\nSpO2 = ");
      Serial.print(spO2);

      temp = HC12.read();
      Serial.print("\nTemp = ");
      Serial.print(temp);

      totalReads++;
      if(totalReads > 125)
      {
        totalReads = 0;
        badReads = 0;
      }
   
    }
    
    lcd.setCursor(0,0);
    
    if ( heartRate >= 100 )
    {
      lcd.print("Heart Rate = ");
      lcd.setCursor(13,0);
      lcd.print(heartRate);
    }
    else 
    {
      lcd.print("Heart Rate =  ");
      lcd.setCursor(14,0);
      lcd.print(heartRate);
    }

    lcd.setCursor(0,1);
    if ( spO2 >= 100 )
    {
      lcd.print("SpO2 = ");
      lcd.setCursor(7,1);
      lcd.print(spO2);
    }
    else 
    {
      lcd.print("SpO2 =  ");
      lcd.setCursor(8,1);
      lcd.print(spO2);
    }

    lcd.setCursor(0,2);
    if ( temp >= 100 )
    {
      lcd.print("Temperature = ");
      lcd.setCursor(14,2);
      lcd.print(temp);
    }
    else 
    {
      lcd.print("Temperature =  ");
      lcd.setCursor(15,2);
      lcd.print(temp);
    }

    //Logic

    if(heartRate > heartRateMax and spO2 < spO2Min)
    {
      badReads++;
    }
    if(heartRate < heartRateMin and spO2 < spO2Min)
    {
      badReads++;
    }

    if(badReads > 20)
    {
      digitalWrite(AT,LOW);
      delay(40);
      HC12.write("AT+C002");
      delay(80);
      digitalWrite(AT,HIGH);
      HC12.write("Ruh Roh");
      e_state = STATE_ALARM;
    }
    break;

    case STATE_ALARM:
      if (digitalRead(reset) == LOW)
      {
        HC12.write("All Clear");
        delay(100);
        digitalWrite(AT,LOW);
        delay(50);
        HC12.write("AT+C001");
        delay(90);
        digitalWrite(AT,HIGH);
        badReads = 0;
        totalReads = 0;
        heartRate = 85;
        spO2 = 98;
        temp = 93;
        e_state = STATE_READ;
      }
      break;
  }
}





void loop()
{  
  if (digitalRead(nxt)== HIGH)
    update_state();

}

