#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID   "TMPL5utonFri"
char auth[] = "knN1BtKKxfuhQjz0XM-NGexujum0khtT";

#define TINY_GSM_MODEM_SIM800

#define BLYNK_HEARTBEAT 120

#include <TinyGsmClient.h>
#include <BlynkSimpleTinyGSM.h>
#include <Wire.h>

// Your GPRS credentials
char apn[]  = "www";
char user[] = "";
char pass[] = "";

#define SerialAT Serial1
#include <SoftwareSerial.h>
SoftwareSerial SerialAT(2, 3); // RX, TX
TinyGsm modem(SerialAT);

#include "EmonLib.h"
#include <Filters.h>
EnergyMonitor emon1, emon2, emon3;                // Creating an instance for every phase
float Irms_R=0;
float Irms_Y=0;
float Irms_B=0;

String string;
String V_R, V_Y, V_B;

float R_Volts; //R Phase Voltage
float Y_Volts; //Y Phase Voltage
float B_Volts; //B Phase Voltage

unsigned long Current_time =0;
unsigned long Current_time1 =0;
unsigned long Last_time =0;
unsigned long Last_time1 =0;

unsigned long last_time =0;
unsigned long current_time =0;
float Kwh=0;
float Wh=0;
float Power=0;
float Power_R=0;
float Power_Y=0;
float Power_B=0;
bool Error=0;
bool Status=0;
bool L_State=0;
bool LO_State=1;
bool E_State=0;
bool EO_State=1;

WidgetLED Led_Status(V6);
WidgetLED Led_Fault(V7);

BLYNK_WRITE(V8) {
  
if(param.asInt() == 1)
{
  digitalWrite(4,HIGH);
}
else
{
  digitalWrite(4,LOW);
 }
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V8);
}


void setup()
{
Wire.begin(); 
Serial.begin(9600);
pinMode(4,OUTPUT);
delay(10);
SerialAT.begin(9600);
delay(3000);

emon1.current(A3,18);
emon2.current(A7,18);
emon3.current(A6,18);

Serial.println("Initializing modem...");
modem.restart();

Blynk.begin(auth, modem, apn, user, pass);
}

void loop()
{

Blynk.run();

Current_time = millis();
if(Current_time - Last_time >= 2000){
Last_time = Current_time;
Calculation();
}

Current_time1 = millis();
if(Current_time1 - Last_time1 >= 30000){
Last_time1 = Current_time1;
Send_Data();
//Serial.println("Sending Data...");
 }
}

void Calculation(){
  
  Irms_R = emon1.calcIrms(1480);
  Irms_Y = emon2.calcIrms(1480);
  Irms_B = emon3.calcIrms(1480);
  
  if(Irms_R < 0.3){
    Irms_R=0;
    Status=0;
    L_State=1;
    if(LO_State==1){
      Led_Status.off();
      LO_State=0;
      }
  }
    if(Irms_Y < 0.3){
    Irms_Y=0;
    Status=0;
    L_State=1;
  }
    if(Irms_B < 0.3){
    Irms_B=0;
    Status=0;
    L_State=1;
   }

  if(Irms_R >0||Irms_B >0||Irms_Y >0){
    Status=1;
    LO_State=1;
  }
//  Serial.print("R_Current:");
//  Serial.println(Irms_R);
//  Serial.print("Y_Current:");
//  Serial.println(Irms_Y);
//  Serial.print("B_Current:");
//  Serial.println(Irms_B);

  Wire.requestFrom(2,30);
  do
  {
  char c = Wire.read();                             // receive a byte as character
  string = string + c;                              //Keep saving whatever is comming
  int comma1 = string.indexOf(',');                     //Sorting the values and assigning to the variables
  V_R = string.substring(0,comma1);
  int comma2 = string.indexOf(',' , comma1+1);
  V_Y = string.substring(comma1+1,comma2);
  int comma3 = string.indexOf('!' , comma2+1);
  V_B = string.substring(comma2+1,comma3);
  } 
while (Wire.available());

  R_Volts = V_R.toFloat();
  Y_Volts = V_Y.toFloat();
  B_Volts = V_B.toFloat();
//  Serial.print("V_R = ");
//  Serial.println(R_Volts);
//  Serial.print("V_Y = ");
//  Serial.println(Y_Volts);
//  Serial.print("V_B = ");
//  Serial.println(B_Volts);

if(R_Volts<15){
  R_Volts=0;
  Error=1;
  EO_State=1;
}
if(Y_Volts<15){
  Y_Volts=0;
  Error=1;
  EO_State=1;
}
if(B_Volts<15){
  B_Volts=0;
  Error=1;
  EO_State=1;
}
if(R_Volts >170 && Y_Volts >170 && B_Volts >170){
  Error=0;
  E_State=1;
  if(EO_State==1){
    Led_Fault.off();
    EO_State=0;
  }
}
  string="";
  delay(50);

  Power_R = (Irms_R*R_Volts)*0.85;
  Power_Y = (Irms_Y*Y_Volts)*0.85;
  Power_B = (Irms_B*B_Volts)*0.85;
  Power = Power_R + Power_Y + Power_B;
  last_time = current_time;
  current_time = millis();  
  Wh = Wh+Power*((current_time -last_time)/3600000.0) ;          // calculating energy in Watt-Hour
  Kwh = Wh/1000;
}

void Send_Data(){
  Blynk.virtualWrite(V3,Irms_R);
  Blynk.virtualWrite(V4,Irms_Y);
  Blynk.virtualWrite(V5,Irms_B);
  Blynk.virtualWrite(V0,R_Volts);
  Blynk.virtualWrite(V1,Y_Volts);
  Blynk.virtualWrite(V2,B_Volts);
  Blynk.virtualWrite(V9,Kwh);
  Blynk.virtualWrite(V10,Power);
  delay(100);
  if(L_State==1 && Status==1){
    Led_Status.on();
    L_State==0;
  }
  if(E_State==1 && Error==1){
    Led_Fault.on();
    E_State==0;
  }
}
