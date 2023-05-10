#include <Convert.h>                        //Library for response conversion 
#include<SPI.h>                             //Library for SPI protocol

//Chip select pin for KP256
#define CS1 31
#define CS2 33
#define CS3 27
#define CS4 25
#define CS5 11
#define CS6 23
#define CS7 10
#define CS8 3
#define CS9 9
#define CS10 43
#define CS11 41
#define CS12 39
#define CS13 37
#define CS14 35
#define CS15 7
#define CS16 5
#define CS17 8
#define CS18 4
#define CS19 45
#define CS20 47

//Chip select pin for TMP26-Q1
#define CS21 29
#define CS22 49

//Defining KP256 calibration variables as per datasheet
const float KP256_Sensor_Offset_LSB = 204.6;
const float ST = 5.115;  // LSB/deg C as per the datasheet

volatile int Response;
String binaryNumber;
char bin[16] = "";
String temperature ="";
String temp ="";
float final_temperature = 0.00; //in deg C
int delayKP256 = 500;

Convert convert; //Assigning a variable name to call the lib

SPISettings  Setting(5000000, MSBFIRST, SPI_MODE1);

void setup (void)
{
Serial.begin(9600);

//Defining pin mode for KP256
pinMode(CS1, OUTPUT);
pinMode(CS2, OUTPUT);
pinMode(CS3, OUTPUT);
pinMode(CS4, OUTPUT);
pinMode(CS5, OUTPUT);
pinMode(CS6, OUTPUT);
pinMode(CS7, OUTPUT);
pinMode(CS8, OUTPUT);
pinMode(CS9, OUTPUT);
pinMode(CS10, OUTPUT);
pinMode(CS11, OUTPUT);
pinMode(CS12, OUTPUT);
pinMode(CS13, OUTPUT);
pinMode(CS14, OUTPUT);
pinMode(CS15, OUTPUT);
pinMode(CS16, OUTPUT);
pinMode(CS17, OUTPUT);
pinMode(CS18, OUTPUT);
pinMode(CS19, OUTPUT);
pinMode(CS20, OUTPUT);

//Defining pin mode for TMP126-Q1
pinMode(CS21, OUTPUT);
pinMode(CS22, OUTPUT);

//Turning CS pin of KP256 sensors to HIGH because of active LOW
digitalWrite(CS1,HIGH);
digitalWrite(CS2,HIGH);
digitalWrite(CS3,HIGH);
digitalWrite(CS4,HIGH);
digitalWrite(CS5,HIGH);
digitalWrite(CS6,HIGH);
digitalWrite(CS7,HIGH);
digitalWrite(CS8,HIGH);
digitalWrite(CS9,HIGH);
digitalWrite(CS10,HIGH);
digitalWrite(CS11,HIGH);
digitalWrite(CS12,HIGH);
digitalWrite(CS13,HIGH);
digitalWrite(CS14,HIGH);
digitalWrite(CS15,HIGH);
digitalWrite(CS16,HIGH);
digitalWrite(CS17,HIGH);
digitalWrite(CS18,HIGH);
digitalWrite(CS19,HIGH);
digitalWrite(CS20,HIGH);

//Turning CS pin of TMP26-Q1 sensors to HIGH because of active LOW
//  digitalWrite(CS19,HIGH);
//  digitalWrite(CS20,HIGH);

//SPI initialization
SPI.begin();
delay(1000);
}

void loop(void)
{
//Calling the function to read that sensor values
  KP256(CS1, 1);
  delay(delayKP256);
  KP256(CS2, 2);
  delay(delayKP256);
  KP256(CS3, 3);
  delay(delayKP256);
  KP256(CS4, 4);
  delay(delayKP256);
  KP256(CS5, 5);
  delay(delayKP256);
  KP256(CS6, 6);
  delay(delayKP256);
  KP256(CS7, 7);
  delay(delayKP256);
  KP256(CS8, 8);
  delay(delayKP256);
  KP256(CS9, 9);
  delay(delayKP256);
  KP256(CS10, 10);
  delay(delayKP256);
  KP256(CS11, 11);
  delay(delayKP256);
  KP256(CS12, 12);
  delay(delayKP256);
  KP256(CS14, 14);
  delay(delayKP256);
  KP256(CS15, 15);
  delay(delayKP256);
  KP256(CS16, 16);
  delay(delayKP256);
  KP256(CS17, 17);
  delay(delayKP256);
  KP256(CS18, 18);
  delay(delayKP256);
  KP256(CS19, 19);
  delay(delayKP256);
  KP256(CS20, 20);
  delay(delayKP256);

}

void KP256(int CS, int j){
  SPI.beginTransaction(Setting);
  delay(100);
  digitalWrite(CS, LOW); //Activating the sensor
  Response = SPI.transfer16(0b0100000000000000); //Recording the response of the sensor with temperature read command
  digitalWrite(CS, HIGH);
  SPI.endTransaction();
  binaryNumber = convert.decimalToBinary(Response); //Converting the data to binary
  binaryNumber.toCharArray(bin, 16); //Converting the data in a character array
  temp = ""; //Emptying the string temp
  //Going through each bit and taking out the bits [1:10], the bits come MSB first so [4:13] (as per datasheet)
  for(int i=4;i<14;i++){
  temperature = bin[i];
  temp += temperature;
  temperature = ""; }
  temp = convert.binaryToDecimal(temp); //Converting the trimmed data back to decimals
  final_temperature = temp.toInt(); //Converting the data into integer (for calculation purpose)
  final_temperature = (final_temperature - KP256_Sensor_Offset_LSB)/ST; //Calibrating the sensor as given in the datasheet
//  Printing the output of the KP256 sensors
  Serial.print("KP256 Sensor");
  Serial.print(j);
  Serial.print(" temp:  ");
  Serial.println(final_temperature);
  Serial.println(Response);
}
