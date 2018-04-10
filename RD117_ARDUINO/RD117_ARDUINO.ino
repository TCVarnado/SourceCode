/********************************************************
*
* Project: MAXREFDES117#
* Filename: RD117_ARDUINO.ino
* Description: This module contains the Main application for the MAXREFDES117 example program.
*
* Revision History:
*\n 1-18-2016 Rev 01.00 GL Initial release.
*\n 12-22-2017 Rev 02.00 Significantlly modified by Robert Fraczkiewicz
*
* --------------------------------------------------------------------
*
* This code follows the following naming conventions:
*
* char              ch_pmod_value
* char (array)      s_pmod_s_string[16]
* float             f_pmod_value
* int32_t           n_pmod_value
* int32_t (array)   an_pmod_value[16]
* int16_t           w_pmod_value
* int16_t (array)   aw_pmod_value[16]
* uint16_t          uw_pmod_value
* uint16_t (array)  auw_pmod_value[16]
* uint8_t           uch_pmod_value
* uint8_t (array)   auch_pmod_buffer[16]
* uint32_t          un_pmod_value
* int32_t *         pn_pmod_value
*
* ------------------------------------------------------------------------- */

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SoftwareSerial.h>

SoftwareSerial HC12(5,6);

#include "max30102.h"
#include "mlx90615.h"
#include "algorithm.h" 

#define MAX_BRIGHTNESS 255
MLX90615 mlx =MLX90615();
#if defined(ARDUINO_AVR_UNO)
//Arduino Uno doesn't have enough SRAM to store 100 samples of IR led data and red led data in 32-bit format
//To solve this problem, 16-bit MSB of the sampled data will be truncated.  Samples become 16-bit data.
uint16_t aun_ir_buffer[100]; //infrared LED sensor data
uint16_t aun_red_buffer[100];  //red LED sensor data
#else
uint32_t aun_ir_buffer[100]; //infrared LED sensor data
uint32_t aun_red_buffer[100];  //red LED sensor data
#endif
int32_t n_ir_buffer_length; //data length
int32_t n_spo2;  //SPO2 value
int8_t ch_spo2_valid;  //indicator to show if the SPO2 calculation is valid
int32_t n_heart_rate; //heart rate value
int8_t  ch_hr_valid;  //indicator to show if the heart rate calculation is valid
uint8_t uch_dummy;

//const byte batteryPin = 9;
const byte oxiInt = 10; // lilypad USB pin connected to MAX30102 INT

// the setup routine runs once when you press reset:
void setup() {

  #if defined(ARDUINO_AVR_LILYPAD_USB)    
    pinMode(13, OUTPUT);  //LED output pin on Lilypad
    maxim_max30102_reset(); //resets the MAX30102
  #endif

//  pinMode(batteryPin,INPUT);
  pinMode(oxiInt, INPUT);  //pin D10 connects to the interrupt output pin of the MAX30102
  
  Wire.begin();

  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);
  
  maxim_max30102_reset(); //resets the MAX30102
  delay(1000);
  
  HC12.begin(9600);

  maxim_max30102_read_reg(REG_INTR_STATUS_1,&uch_dummy);  //Reads/clears the interrupt status register

    // Measure battery voltage
//  float measuredvbat = analogRead(batteryPin);
//  measuredvbat *= 2;    // we divided by 2, so multiply back
//  measuredvbat *= 3.7;  // Multiply by 3.3V, our reference voltage
//  measuredvbat /= 1024; // convert to voltage
  

  while(Serial.available()==0)  //wait until user presses a key
  {
//      Serial.print(F("Vbatt=\t"));
//      Serial.println(measuredvbat);

      Serial.write(27);       // ESC command
      Serial.print(F("[2J"));    // clear screen command
    #if defined(ARDUINO_AVR_LILYPAD_USB)    
        Serial.println(F("Lilypad"));
    #endif
    
        Serial.println(F("Press any key to start conversion"));
        delay(1000);
  }
  uch_dummy=Serial.read();

  maxim_max30102_init();  //initialize the MAX30102
  delay(100);
}

//Continuously taking samples from MAX30102.  Heart rate and SpO2 are calculated every 4 seconds
void loop() {
  
  uint32_t un_min, un_max, un_prev_data, un_brightness;  //variables to calculate the on-board LED brightness that reflects the heartbeats
  int32_t i;
  float f_temp;
  
  un_brightness=0;
  un_min=0x3FFFF;
  un_max=0;

  n_ir_buffer_length=100;
  
  //buffer length of 100 stores 4 seconds of samples running at 25sps
  //read 100 samples, and determine the signal range
  for(i=0;i<n_ir_buffer_length;i++)
  {
    while(digitalRead(10)==1);  //wait until the interrupt pin asserts
    maxim_max30102_read_fifo((aun_red_buffer+i), (aun_ir_buffer+i));  //read from MAX30102 FIFO

    if(un_min>aun_red_buffer[i])
      un_min=aun_red_buffer[i];  //update signal min
    if(un_max<aun_red_buffer[i])
      un_max=aun_red_buffer[i];  //update signal max
      
    Serial.print(F("red="));
    Serial.print(aun_red_buffer[i], DEC);

    Serial.print(F(", ir="));
    Serial.println(aun_ir_buffer[i], DEC);    

     
  }
  
   un_prev_data=aun_red_buffer[i];
  
//calculate heart rate and SpO2 after 100 samples (4 seconds of samples) using MAXIM's method  
  maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_spo2, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);      
    
  while(1)
  {
    i=0;
    un_min=0x3FFFF;
    un_max=0;

    //dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
    for(i=25;i<100;i++)
    {
      aun_red_buffer[i-25]=aun_red_buffer[i];
      aun_ir_buffer[i-25]=aun_ir_buffer[i];

      //update the signal min and max
      if(un_min>aun_red_buffer[i])
        un_min=aun_red_buffer[i];
      if(un_max<aun_red_buffer[i])
        un_max=aun_red_buffer[i];
    }

    //take 25 sets of samples before calculating the heart rate.
    for(i=75;i<100;i++)
    {
      un_prev_data=aun_red_buffer[i-1];
      while(digitalRead(10)==1);
      digitalWrite(9, !digitalRead(9));
      maxim_max30102_read_fifo((aun_red_buffer+i), (aun_ir_buffer+i));

      //calculate the brightness of the LED
      if(aun_red_buffer[i]>un_prev_data)
      {
        f_temp=aun_red_buffer[i]-un_prev_data;
        f_temp/=(un_max-un_min);
        f_temp*=MAX_BRIGHTNESS;
        f_temp=un_brightness-f_temp;
        if(f_temp<0)
          un_brightness=0;
        else
          un_brightness=(int)f_temp;
      }
      else
      {
        f_temp=un_prev_data-aun_red_buffer[i];
        f_temp/=(un_max-un_min);
        f_temp*=MAX_BRIGHTNESS;
        un_brightness+=(int)f_temp;
        if(un_brightness>MAX_BRIGHTNESS)
          un_brightness=MAX_BRIGHTNESS;
      }

  
//  float tempAmbient = mlx90615.getTemperature(MLX90615_AMBIENT_TEMPERATURE);
    float tempObject = mlx.get_object_temp();

    tempObject = tempObject * (9.0 / 5.0) + 32.0;
   
    #if defined(ARDUINO_AVR_LILYPAD_USB)  
      analogWrite(13, un_brightness);
    #endif

//      delay(100);   
//      Serial.print(F("red = "));
//      Serial.print(aun_red_buffer[i], DEC);
//      Serial.print(F(", ir = "));
//      Serial.print(aun_ir_buffer[i], DEC);
         
      Serial.print(F("HR = "));
      Serial.print(n_heart_rate, DEC);
      
      Serial.print(F(", HRvalid = "));
      Serial.print(ch_hr_valid, DEC);
      
      Serial.print(F(", SPO2 = "));
      Serial.print(n_spo2, DEC);

      Serial.print(F(", SPO2Valid = "));
      Serial.print(ch_spo2_valid, DEC);

      Serial.print(" Temperature = ");
      Serial.println(tempObject);

      //Sending data to other devices 
      
      if (ch_hr_valid == 1 && ch_spo2_valid == 1)
      {
        HC12.write(n_heart_rate);
        HC12.write(n_spo2);
        HC12.write(tempObject);
      }

      
    }
    maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_spo2, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid); 
  }
}
 
