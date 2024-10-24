/*
* Name: clock, temp, piezo and IR project
* Author: Hugo Uddmar
* Date: 2024-10-24
* Description: This project uses a ds3231 to measure time and displays the time to an 1306 oled display,
* Further, it measures temprature with a analog temprature module and displays a mapped value to a 9g-servo-motor. 
* The piezo also plays sound when the temperature changes. Higher temperatures mean higher frequencies and the tone only plays once when the temperature changes.
* Finally, I tried to use the ir light transmitter to send the temperature to the receiver and then write the temperature on the screen but it didn't work.
* So I just wrote the temperature on the screen the normal way.
*/

// Include Libraries

#include <Arduino.h>
#include <IRremote.hpp>
#include <RTClib.h> 
#include <Wire.h>
#include "U8glib.h"
#include <Servo.h>

// Init constants

int piezoPin = 10; 
int irreceiverPin = 5;
int irtransmitterPin = 4;

// Init global variables

uint8_t hello = 0x34; //meddelandet som man ska skicka med ir ljus

float oldTone; //initiera oldtone, den tonen som piezon spelade senast.

// construct objects

Servo myservo; //kontruerar servo objektet
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK); //konstruerar skärmen och i2c
RTC_DS3231 rtc; //konstruerar klockan

void setup() {
  // init communication
  Serial.begin(9600);
  // Init Hardware
  Wire.begin();
  u8g.setFont(u8g_font_unifont);
  rtc.begin(); //sätter igång klockan
  rtc.adjust(DateTime(F(__DATE__),F(__TIME__))); //justerar tiden till nuvarande
  IrReceiver.begin(irreceiverPin, ENABLE_LED_FEEDBACK);//sätter igång irreceiver och sen irsender
  IrSender.begin(irtransmitterPin);

  //sätter igång alla pins för alla kompononenter
  pinMode(A1,INPUT); //tempsensor
  pinMode(piezoPin,OUTPUT); //piezo
  myservo.attach(8); //sätter igång servon
  pinMode(irreceiverPin,INPUT); //irreceiver pin
  pinMode(irtransmitterPin,OUTPUT); //irsender/transmitter pin//
}

void loop() {
  
  oledWrite(getTime(),30,30, String(getTemp()),45,55); //skriver ut tiden på skärmen vid en viss koordinat och temperaturen
  servoWrite(getTemp()); //roterar servon med tempartur som input
  piezoplay(getTemp()); //spelar piezo med temperatur som input

  irsendandreceive(getTemp()); //skickar och tar emot ir ljus
  
  delay(1000); 
}


/*
*This function reads time from an ds3231 module and package the time as a String
*Parameters: Void
*Returns: time in hh:mm:ss as String
*/
String getTime(){
  DateTime now = rtc.now(); //skaffar tiden men klockan
  return String(now.hour()) + ":" + String(now.minute()) +":"+ String(now.second());
  delay(1000);
}

/*
* This function reads an analog pin connected to an analog temprature sensor and calculates the corresponding temp
*Parameters: Void
*Returns: temprature as float
*/
float getTemp() {
  float temp = 0;
  float R1 = 10000;  // value of R1 on board
  float logR2, R2, T;
  float c1 = 0.001129148, c2 = 0.000234125, c3 = 0.0000000876741;  //steinhart-hart coeficients for thermistor

  int Vo = analogRead(A1);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);  //calculate resistance on thermistor
  logR2 = log(R2);
  temp = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));  // temperature in Kelvin
  temp = temp - 273.15;                                           //convert Kelvin to Celcius

  return temp;
}

/*
* This function takes a string and draws it to an oled display
*Parameters: - text: String to write to display
*Returns: void
*/
void oledWrite(String text, int x, int y, String text1, int x1, int y1) {
    u8g.firstPage();

    do{

      u8g.drawStr(x,y,text.c_str()); //skriver två rader på skärmen
      u8g.drawStr(x1,y1,text1.c_str());

    } while (u8g.nextPage());
  }

/*
* takes a temprature value and maps it to corresppnding degree on a servo
*Parameters: - value: temprature
*Returns: void
*/
void servoWrite(float value){
  float ang = map(value,22,30,0,180); //mappar värdet och sen roterar servot
  myservo.write(ang);
  
}

/*
* this function takes the temperature and maps it to frequency. Then it plays the piezo on that frequency but a tone cannot be repeated.
*Parameters: - value: temperature
*Return: void
*/

void piezoplay(float value) {

  float tone1 = map(value,22,30,20,1000); //mappa värdet

  if(oldTone == tone1)  //kollar så det inte är samma ton
  {

  }
  else
  {
    tone(piezoPin,tone1); //spelar ton
    delay(400);
    noTone(piezoPin);
  }

  oldTone = tone1;

}

/*
* this function takes the temperature and sends it to the ir receiver via the ir sender and then prints out the temperature. 
* Unfortunately I could not get it to work. I would also want to print the value of the received message (the temperature) 
* on the screen with the function by returning the value that the ir receiver received.
*Parameters: - value: temperature
*Return: void
*/
void irsendandreceive(float value){
  IrSender.sendNEC(0x00, hello /*value*/); //skickar ett meddelande i hex men kom inte så långt
  if (IrReceiver.decode()) {
    IrReceiver.printIRResultShort(&Serial); // printa kompletta meddelandet i en rad
    IrReceiver.resume(); // gör så att den kan ta emot nästa signal
  }
}