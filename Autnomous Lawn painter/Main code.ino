#include <digitalWriteFast.h>
#include <Servo.h>
#include <Wire.h>
#include <MechaQMC5883.h>

#define s_pwm 7
Servo servo1;

#define cm_pwm 11
#define cm_brk 12

#define pwm1 8
#define brk1 48
#define dir1 49

#define pwm2 9
#define brk2 31
#define dir2 30
 
#define c_LeftEncoderPinA 2
#define c_LeftEncoderPinB 3
#define LeftEncoderIsReversed

MechaQMC5883 qmc;
int azimuth;

volatile long tickgoal1 = 0;
volatile long slowd = 0;
volatile bool _LeftEncoderBSet;
volatile long _LeftEncoderTicks = 0;
volatile long degreeTicks=0;
int pulseschangedleft;

String readString;
String dx;
String dy;
String dl;
String Type;
volatile long dataX;
volatile long dataY;
int comma1;
int comma2;
int comma3;
int comma4;
char letter;
int firstletter=0;
int AZI_PWM = 50;
int state = 0;
const int NV= 97;
const int SV= 249;
const int EV= 162;
const int WV= 28;
const int NE= 0;
const int NW= 0;
const int SW= 0;
const int SEAST= 72;
const int LNE= 0;
const int LNW= 0;
const int LSE= 72;
const int LSW= 0;
const int VD= 0;
const int VU= 0;


void setup() {
  Serial.begin (9600);
  Wire.begin();
  qmc.init();
  delay(1000);
  servo1.attach(s_pwm);
  servo1.write(180);
  pinMode(pwm1, OUTPUT);
  pinMode(brk1, OUTPUT);
  pinMode(dir1, OUTPUT);
  pinMode(pwm2, OUTPUT);
  pinMode(brk2, OUTPUT);
  pinMode(dir2, OUTPUT); 
  pinMode(cm_pwm, OUTPUT);
  pinMode(cm_brk, OUTPUT);
  pinMode(c_LeftEncoderPinA, INPUT);      // sets pin A as input
  digitalWrite(c_LeftEncoderPinA, LOW);  // turn on pullup resistors
  pinMode(c_LeftEncoderPinB, INPUT);      // sets pin B as input
  digitalWrite(c_LeftEncoderPinB, LOW);  // turn on pullup resistors
  attachInterrupt(0, HandleLeftMotorInterruptA, RISING);
  east();
}
 
void loop()
{ 

while(Serial.available())
  {
    delay(3);
    char c= Serial.read();
    readString += c;
  }

 if ( readString.length() >0)
 {
  comma1 = readString.indexOf(',');
  dx = readString.substring(0,comma1);

  comma2 = readString.indexOf(',' , comma1+1);
  dy = readString.substring(comma1+1,comma2+1);

  comma3 = readString.indexOf(',' , comma2+1);
  dl = readString.substring(comma2+1,comma3);

  comma4 = readString.indexOf('*' , comma3+1);
  Type = readString.substring(comma3+1,comma4);
  
  dataX = dx.toInt()-100;
  dataY = dy.toInt();
  
  Serial.println(dataX);
  Serial.println(dataY);
  Serial.println(dl);
  Serial.println(Type);
  delay(1000);
  
  print_encleft();
  moveF(dataX);
  delay(3000);
  north();
  delay(3000);
  moveF(dataY);
  delay(3000);
  printletter();
  delay(3000);
 }
}
 
void HandleLeftMotorInterruptA()
{
  _LeftEncoderBSet = digitalReadFast(c_LeftEncoderPinB); // read the input pin
  // and adjust counter + if A leads B
  #ifdef LeftEncoderIsReversed
    _LeftEncoderTicks -= _LeftEncoderBSet ? -1 : +1;
  #else
    _LeftEncoderTicks += _LeftEncoderBSet ? -1 : +1;
  #endif
  pulseschangedleft = 1;
}

void print_encleft()
{
   if (pulseschangedleft != 0) {
    pulseschangedleft = 0;
    Serial.println(_LeftEncoderTicks);
  }
  return;
 }

void moveF(long GO)
  {
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  _LeftEncoderTicks=0;
  tickgoal1= GO*15.8;
  slowd= tickgoal1 - ((tickgoal1*15)/100);
  while(_LeftEncoderTicks < tickgoal1)
  {
    if(_LeftEncoderTicks < slowd){
    for(int i=30,j=30;i<=80,j<=80;i+=10,j+=10){
    analogWrite(pwm1,i);
    analogWrite(pwm2,j);
    azimuth_print();
    while(azimuth < state-1){
    analogWrite(pwm1,100);
    analogWrite(pwm2,50);
    azimuth_print();
      if(_LeftEncoderTicks > slowd){
        cmoff();
        goto slow;
      }
    }
    while(azimuth > state+1){
     analogWrite(pwm1,50);
     analogWrite(pwm2,100);
     azimuth_print();
      if(_LeftEncoderTicks > slowd){
        goto slow;
     }
    }
   }
  }
   slow:
    if(_LeftEncoderTicks > slowd){
    for(int i=30,j=30;i<=70,j<=70;i+=10,j+=10){
    analogWrite(pwm1,i);
    analogWrite(pwm2,j);
    azimuth_print();
    while(azimuth < state){
     analogWrite(pwm1,70);
     analogWrite(pwm2,30);
     azimuth_print();
    }
    while(azimuth > state){
     analogWrite(pwm1,30);
     analogWrite(pwm2,70);
     azimuth_print();
    }
    if(_LeftEncoderTicks > tickgoal1){
      break;
     }
    }
   }
  }
  digitalWrite(brk1,HIGH);
  digitalWrite(brk2,HIGH);
  delay(500);
  if(_LeftEncoderTicks != tickgoal1) {
  if(_LeftEncoderTicks > tickgoal1)  {
        moveRF();
    if(_LeftEncoderTicks < tickgoal1){
        moveFF();
   }
  }
 }
else{
  digitalWrite(brk1,HIGH);
  digitalWrite(brk2,HIGH);
  }
  tickgoal1=0;
  slowd=0;
  dataX=0;
  return;
}
void moveFF()
{
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  while(_LeftEncoderTicks < tickgoal1)
  {
    for(int k=15,l=30;k<=70,l<=60;k+=10,l+=10){
    analogWrite(pwm1,k);
    analogWrite(pwm2,l);
    if(_LeftEncoderTicks > tickgoal1){
      break;
    }
   }
  }
  digitalWrite(brk1,HIGH);
  digitalWrite(dir1,LOW);
  digitalWrite(brk2,HIGH);
  digitalWrite(dir2,LOW);
  return;
}
void moveRF()
{
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  while(tickgoal1 < _LeftEncoderTicks)
  {
    digitalWrite(dir1,HIGH);
    digitalWrite(dir2,HIGH);
    for(int m=15,n=15;m<=70,n<=60;m+=10,n+=10){
    analogWrite(pwm1,m);
    analogWrite(pwm2,n);
      if(_LeftEncoderTicks < tickgoal1){
      break;
    }
   }  
  }
  digitalWrite(brk1,HIGH);
  digitalWrite(dir1,LOW);
  digitalWrite(brk2,HIGH);
  digitalWrite(dir2,LOW);
  delay(500);
  return;
}

void moveR(long LO)
{
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  _LeftEncoderTicks=0;
  tickgoal1=0;
  tickgoal1= -LO*15.87 ;
  while(_LeftEncoderTicks > tickgoal1)
  {
    digitalWrite(dir1,HIGH);
    digitalWrite(dir2,HIGH);
    print_encleft();
    for(int i=20,j=40;i<=80,j<=160;i+=25,j+=40){
    analogWrite(pwm1,j);
    analogWrite(pwm2,i);
    print_encleft();
   }
  }
  digitalWrite(brk1,HIGH);
  digitalWrite(brk2,HIGH);
  tickgoal1=0;
  dataX=0;
  return;
}

void azimuth_print()
{ 
  int x, y, z;
  qmc.read(&x, &y, &z,&azimuth);
  Serial.print("                          a: ");
  Serial.print(                              azimuth);
  Serial.println();
}


void north()
{ state = 0;
  delay(400);
  azimuth_print();
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  while( azimuth > NV+1 || azimuth < NV-1){
  azimuth_print();
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  if(azimuth > NV && azimuth <= SV){
  digitalWrite(dir1,HIGH);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,LOW);
  analogWrite(pwm2, AZI_PWM);
  }
  else{
  digitalWrite(dir1,LOW);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,HIGH);
  analogWrite(pwm2, AZI_PWM);
  }
}
   digitalWrite(brk1,HIGH);
   digitalWrite(brk2,HIGH);
   state = NV;
   delay(1000);
   return;
}
void west(){
  state = 0;
  delay(400);
  azimuth_print();
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  while( azimuth > WV+1 || azimuth < WV-1){
  azimuth_print();
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  if(azimuth > WV && azimuth <= EV){
  digitalWrite(dir1,HIGH);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,LOW);
  analogWrite(pwm2, AZI_PWM);
  }
  else{
  digitalWrite(dir1,LOW);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,HIGH);
  analogWrite(pwm2, AZI_PWM);
  }
}
   digitalWrite(brk1,HIGH);
   digitalWrite(brk2,HIGH);
   state = WV;
   delay(1000);
   return;
}
void south(){
  state = 0;
  delay(400);
  azimuth_print();
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  while( azimuth > SV+1 || azimuth < SV-1){
  azimuth_print();
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  if(azimuth < SV && azimuth >= NV){
  digitalWrite(dir1,LOW);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,HIGH);
  analogWrite(pwm2, AZI_PWM);
  }
  else{
  digitalWrite(dir1,HIGH);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,LOW);
  analogWrite(pwm2, AZI_PWM);
  }
}
   digitalWrite(brk1,HIGH);
   digitalWrite(brk2,HIGH);
   state = SV;
   delay(1000);
   return;
}
void east(){
  state = 0;
  delay(400);
  azimuth_print();
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  while( azimuth > EV+1 || azimuth < EV-1){
  azimuth_print();
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  if(azimuth < EV && azimuth >= WV){
  digitalWrite(dir1,LOW);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,HIGH);
  analogWrite(pwm2, AZI_PWM);
  }
  else{
  digitalWrite(dir1,HIGH);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,LOW);
  analogWrite(pwm2, AZI_PWM);
  }
}
   digitalWrite(brk1,HIGH);
   digitalWrite(brk2,HIGH);
   state = EV;
   delay(1000);
   return;
}
void nw(){

  delay(1000);
  azimuth_print();
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  while( azimuth > NW+1 || azimuth < NW-1){
  azimuth_print();
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  digitalWrite(dir1,LOW);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,HIGH);
  analogWrite(pwm2, AZI_PWM);
}
   digitalWrite(brk1,HIGH);
   digitalWrite(brk2,HIGH);
   delay(1000);
   return;
}
void ne(){

  delay(1000);
  azimuth_print();
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  while( azimuth > NE+1 || azimuth < NE-1){
  azimuth_print();
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  digitalWrite(dir1,LOW);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,HIGH);
  analogWrite(pwm2, AZI_PWM);

}
   digitalWrite(brk1,HIGH);
   digitalWrite(brk2,HIGH);
   delay(1000);
   return;
}
void se(){

  delay(1000);
  azimuth_print();
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  while( azimuth > SEAST+1 || azimuth < SEAST-1){
  azimuth_print();
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  digitalWrite(dir1,LOW);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,HIGH);
  analogWrite(pwm2, AZI_PWM);
  }
   digitalWrite(brk1,HIGH);
   digitalWrite(brk2,HIGH);
   delay(1000);
   return;
}
void sw(){

  delay(1000);
  azimuth_print();
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  while( azimuth > SW+1 || azimuth < SW-1){
  azimuth_print();
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  digitalWrite(dir1,LOW);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,HIGH);
  analogWrite(pwm2, AZI_PWM);
  }
   digitalWrite(brk1,HIGH);
   digitalWrite(brk2,HIGH);
   delay(1000);
   return;
}
void lne(){

  delay(1000);
  azimuth_print();
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  while( azimuth > LNE+1 || azimuth < LNE-1){
  azimuth_print();
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  digitalWrite(dir1,LOW);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,HIGH);
  analogWrite(pwm2, AZI_PWM);
  }
   digitalWrite(brk1,HIGH);
   digitalWrite(brk2,HIGH);
   delay(1000);
   return;
}
void lnw(){

  delay(1000);
  azimuth_print();
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  while( azimuth > LNW+1 || azimuth < LNW-1){
  azimuth_print();
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  digitalWrite(dir1,LOW);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,HIGH);
  analogWrite(pwm2, AZI_PWM);
}
   digitalWrite(brk1,HIGH);
   digitalWrite(brk2,HIGH);
   delay(1000);
   return;
}
void lse(){

  delay(1000);
  azimuth_print();
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  while( azimuth > LSE+1 || azimuth < LSE-1){
  azimuth_print();
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  digitalWrite(dir1,LOW);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,HIGH);
  analogWrite(pwm2, AZI_PWM);
  }
   digitalWrite(brk1,HIGH);
   digitalWrite(brk2,HIGH);
   delay(1000);
   return;
}
void lsw(){

  delay(1000);
  azimuth_print();
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  while( azimuth > LSW+1 || azimuth < LSW-1){
  azimuth_print();
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  digitalWrite(dir1,LOW);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,HIGH);
  analogWrite(pwm2, AZI_PWM);
  }
   digitalWrite(brk1,HIGH);
   digitalWrite(brk2,HIGH);
   delay(1000);
   return;
}
void vu(){

  delay(1000);
  azimuth_print();
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  while( azimuth > VU+1 || azimuth < VU-1){
  azimuth_print();
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  digitalWrite(dir1,HIGH);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,LOW);
  analogWrite(pwm2, AZI_PWM);

}
   digitalWrite(brk1,HIGH);
   digitalWrite(brk2,HIGH);
   delay(1000);
   return;
}
void vd(){

  delay(1000);
  azimuth_print();
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  while( azimuth > VD+1 || azimuth < VD-1){
  azimuth_print();
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  digitalWrite(dir1,LOW);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,HIGH);
  analogWrite(pwm2, AZI_PWM);
}
   digitalWrite(brk1,HIGH);
   digitalWrite(brk2,HIGH);
   delay(1000);
   return;
}
void norths()
{
   delay(1000);
   moveR(210);
   north();
   moveF(210);
   delay(2000);
   return;
}
void wests()
{
   delay(1000);
   moveR(210);
   west();
   moveF(210);
   delay(2000);
   return;
}
void souths()
{ 
   delay(1000);
   moveR(210);
   south();
   moveF(210);
   delay(2000);
   return;
}
void easts()
{  
   delay(1000);
   moveR(210);
   east();
   moveF(210);
   delay(2000);
   return;
}
void vus()
{
   delay(1000);
   moveR(210);
   vu();
   moveF(210);
   delay(2000);
   return;
}
void vds()
{
   delay(1000);
   moveR(210);
   vd();
   moveF(210);
   delay(2000);
   return;
}
void lsws()
{
   delay(1000);
   moveR(210);
   lsw();
   moveF(210);
   delay(2000);
   return;
}
void lses()
{
   delay(1000);
   moveR(210);
   lse();
   moveF(210);
   delay(2000);
   return;
}
void lnws()
{
   delay(1000);
   moveR(210);
   lnw();
   moveF(210);
   delay(2000);
   return;
}
void lnes()
{
   delay(1000);
   moveR(210);
   lne();
   moveF(210);
   delay(2000);
   return;
}
void nes()
{
   delay(1000);
   moveR(210);
   ne();
   moveF(210);
   delay(2000);
   return;
}
void nws()
{
   delay(1000);
   moveR(210);
   nw();
   moveF(210);
   delay(2000);
   return;
}
void sws()
{
   delay(1000);
   moveR(210);
   sw();
   moveF(210);
   delay(2000);
   return;
}
void ses()
{
   delay(1000);
   moveR(210);
   se();
   moveF(210);
   delay(2000);
   return;
}
void nwr()
{
  delay(1000);
  azimuth_print();
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  while( azimuth > NW+1 || azimuth < NW-1){
  azimuth_print();
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  digitalWrite(dir1,HIGH);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,LOW);
  analogWrite(pwm2, AZI_PWM);
}
    digitalWrite(brk1,HIGH);
    digitalWrite(brk2,HIGH);
    delay(1000);
    return;
}
void ner(){

  delay(1000);
  azimuth_print();
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  while( azimuth > NE+1 || azimuth < NE-1){
  azimuth_print();
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  digitalWrite(dir1,HIGH);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,LOW);
  analogWrite(pwm2, AZI_PWM);

}
    digitalWrite(brk1,HIGH);
    digitalWrite(brk2,HIGH);
    delay(1000);
    return;
}
void ser(){

  delay(1000);
  azimuth_print();
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  while( azimuth > SEAST+1 || azimuth < SEAST-1){
  azimuth_print();
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  digitalWrite(dir1,HIGH);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,LOW);
  analogWrite(pwm2, AZI_PWM);
  }
    digitalWrite(brk1,HIGH);
    digitalWrite(brk2,HIGH);
    delay(1000);
    return;
}
void swr(){

  delay(1000);
  azimuth_print();
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  while( azimuth > SW+1 || azimuth < SW-1){
  azimuth_print();
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  digitalWrite(dir1,HIGH);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,LOW);
  analogWrite(pwm2, AZI_PWM);
  }
    digitalWrite(brk1,HIGH);
    digitalWrite(brk2,HIGH);
    delay(1000);
    return;
}
void lner(){

  delay(1000);
  azimuth_print();
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  while( azimuth > LNE+1 || azimuth < LNE-1){
  azimuth_print();
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  digitalWrite(dir1,HIGH);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,LOW);
  analogWrite(pwm2, AZI_PWM);
  }
    digitalWrite(brk1,HIGH);
    digitalWrite(brk2,HIGH);
    delay(1000);
    return;
}
void lnwr(){

  delay(1000);
  azimuth_print();
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  while( azimuth > LNW+1 || azimuth < LNW-1){
  azimuth_print();
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  digitalWrite(dir1,HIGH);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,LOW);
  analogWrite(pwm2, AZI_PWM);

}
    digitalWrite(brk1,HIGH);
    digitalWrite(brk2,HIGH);
    delay(1000);
    return;
}

void lser(){

  delay(1000);
  azimuth_print();
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  while( azimuth > LSE+1 || azimuth < LSE-1){
  azimuth_print();
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  digitalWrite(dir1,HIGH);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,LOW);
  analogWrite(pwm2, AZI_PWM);
  }
    digitalWrite(brk1,HIGH);
    digitalWrite(brk2,HIGH);
    delay(1000);
    return;
}
void lswr(){

  delay(1000);
  azimuth_print();
  digitalWrite(dir1,LOW);
  digitalWrite(dir2,LOW);
  while( azimuth > LSW+1 || azimuth < LSW-1){
  azimuth_print();
  digitalWrite(brk1,LOW);
  digitalWrite(brk2,LOW);
  digitalWrite(dir1,HIGH);
  analogWrite(pwm1, AZI_PWM);
  digitalWrite(dir2,LOW);
  analogWrite(pwm2, AZI_PWM);
  }
    digitalWrite(brk1,HIGH);
    digitalWrite(brk2,HIGH);
    delay(1000);
    return;
}
void northsr(){

   delay(1000);
   moveR(285);
   north();
   moveF(285);
   delay(2000);
   return;
}
void westsr(){

   delay(1000);
   moveR(285);
   west();
   moveF(285);
   delay(2000);
   return;
}
void southsr(){
  
   delay(1000);
   moveR(285);
   south();
   moveF(285);
   delay(2000);
   return;
}
void eastsr(){
  
   delay(1000);
   moveR(285);
   east();
   moveF(285);
   delay(2000);
   return;
}
void lswsr(){

   delay(1000);
   moveR(285);
   lswr();
   moveF(285);
   delay(2000);
   return;
}
void lsesr(){

   delay(1000);
   moveR(285);
   lser();
   moveF(285);
   delay(2000);
   return;
}
void lnwsr(){

   delay(1000);
   moveR(285);
   lnwr();
   moveF(285);
   delay(2000);
   return;
}
void lnesr(){

   delay(1000);
   moveR(285);
   lner();
   moveF(285);
   delay(2000);
   return;
}
void nesr(){

   delay(1000);
   moveR(285);
   ner();
   moveF(285);
   delay(2000);
   return;
}
void nwsr(){

   delay(1000);
   moveR(285);
   nwr();
   moveF(285);
   delay(2000);
   return;
}
void swsr(){

   delay(1000);
   moveR(285);
   swr();
   moveF(285);
   delay(2000);
   return;
}
void sesr(){

   delay(1000);
   moveR(285);
   ser();
   moveF(285);
   delay(2000);
   return;
}
void cmon()
{
  digitalWrite(cm_brk,LOW);
  analogWrite(cm_pwm,255);
  delay(1000);
}
void cmoff()
{
 digitalWrite(cm_brk,HIGH);
 delay(1000);
}
void spray_on()
{
  servo1.write(110);
}
void spray_off()
{
  servo1.write(180);
}
void sprayl(int dist)
{
  delay(1000);
  spray_on();
  delay(500);
  moveF(dist);
  spray_off();
  dist=0;
  delay(1000);
}
void cutl(int cdist)
{
  cmon();
  moveF(cdist);
  cmoff();
}
void printletter()
{
  int letterlength = dl.length();
  int i;
  for (i=0 ; i < letterlength ; i++)
  {
    firstletter=0;
       if(i==0){
          firstletter=1;
        }
  Serial.println(dl.charAt(i));
  letter = dl.charAt(i);
    
  if(letter== 'A' || letter== 'a' )
  {
    printA();
  }
    else if(letter== 'B' || letter=='b')
  {
    printB();
  }
    else if(letter== 'C' || letter=='c')
  {
    printC();
  }
    else if(letter== 'D' || letter=='d')
  {
    printD();
  }
    else if(letter== 'E' || letter=='e')
  {
    printE();
  }
    else if(letter== 'F' || letter=='f')
  {
    printF();
  } 
    else if(letter== 'G' || letter=='g')
  {
    printG();
  }
    else if(letter== 'H' || letter=='h')
  {
    printH();
  }
    else if(letter== 'I' || letter=='i')
  {
    printI();
  }
    else if(letter== 'J' || letter=='j')
  {
    printJ();
  }
    else if(letter== 'K' || letter=='k')
  {
    printK();
  }
    else if(letter== 'L' || letter=='l')
  {
    printL();
  }
    else if(letter== 'M' || letter=='m')
  {
    printM();
  }
    else if(letter== 'N' || letter=='n')
  {
    printN();
  }
    else if(letter== 'O' || letter=='o')
  {
    printO();
  }
    else if(letter== 'P' || letter=='p')
  {
    printP();
  }
    else if(letter== 'Q' || letter=='q')
  {
    printQ();
  }
    else if(letter== 'R' || letter=='r')
  {
    printR();
  }
    else if(letter== 'S' || letter=='s')
  {
    printS();
  }
    else if(letter== 'T' || letter=='t')
  {
    printT();
  }
    else if(letter== 'U' || letter=='u')
  {
    printU();
  }
    else if(letter== 'V' || letter=='v')
  {
    printV();
  }
    else if(letter== 'W' || letter=='w')
  {
    printW();
  }
    else if(letter== 'X' || letter=='x')
  {
    printX();
  }   
    else if(letter== 'Y' || letter=='y')
  {
    printY();
  }
    else if(letter== 'Z' || letter=='z')
  {
    printZ();
  }
  delay(3000);
  }
 }
 
 void printA()
 {
  if(Type=="S"){
    printAs();
    return;
  }
  Serial.println("A ok");
   if(firstletter==1)
  {
    goto no_left;
  }
  north();
  no_left:
  cmon();
  moveF(1200);
  east();
  moveF(600);
  south();
  moveF(1200);
  cmoff();
  west();
  moveF(600);
  north();
  cmon();
  moveF(600);
  east();
  moveF(600);
  south();
  moveF(600);
  cmoff();
  east();
  moveF(600);
  delay(1000);
  Serial.println("A DONE");
  return;
  }

void printB()
 {
    if(Type=="S"){
    printBs();
    return;
  }
  Serial.println("B ok");
  if(firstletter==1)
  {
    goto no_left1;
  }
  north();
  no_left1:
  cmon();
  moveF(1200);
  east();
  moveF(600);
  south();
  moveF(1200);
  west();
  moveF(600);
  north();
  moveF(600);
  east();
  moveF(600);
  south();
  moveF(600);
  cmoff();
  east();
  moveF(600);
  delay(1000);
  Serial.println("B DONE");
  return;
 }

void printC()
 {
    if(Type=="S"){
    printCs();
    return;
  }
  Serial.println("C ok");
    if(firstletter==1)
  {
    goto no_left2;
  }
  north();
  no_left2:
  cmon();
  moveF(1200);
  east();
  moveF(600);
  cmoff();
  south();
  moveF(1200);
  west();
  cmon();
  moveF(600);
  cmoff();
  east();
  moveF(1200);
  delay(1000);
  Serial.println("C DONE");
  return;
 }

void printD()
 {
    if(Type=="S"){
    printDs();
    return;
  }
  Serial.println("D ok");
    if(firstletter==1)
  {
    goto no_left3;
  }
  north();
  no_left3:
  cutl(1200);
  west();
  cutl(200);
  east();
  cutl(800);
  south();
  cutl(1200);
  west();
  cutl(800);
  east();
  moveF(1400);
  delay(1000);
  Serial.print("D DONE");
  return;
 }
 
void printE()
 {
    if(Type=="S"){
    printEs();
    return;
  }
  Serial.println("E ok");
    if(firstletter==1)
  {
    goto no_left4;
  }
  north();
  no_left4:
  cmon();
  moveF(1200);
  east();
  moveF(600);
  south();
  cmoff();
  moveF(600);
  west();
  cmon();
  moveF(600);
  cmoff();
  south();
  moveF(600);
  east();
  cmon();
  moveF(600);
  cmoff();
  moveF(600);
  delay(1000);
  Serial.print("E DONE");
  return;
 }
 
void printF()
 {
    if(Type=="S"){
    printFs();
    return;
  }
  Serial.println("F ok");
    if(firstletter==1)
  {
    goto no_left5;
  }
  north();
  no_left5:
  cmon();
  moveF(1200);
  east();
  moveF(600);
  cmoff();
  south();
  moveF(600);
  west();
  cmon();
  moveF(600);
  south();
  moveF(600);
  cmoff();
  east();
  moveF(1200);
  delay(1000);
  Serial.print("F DONE");
  return;
 }

void printG()
 {
    if(Type=="S"){
    printGs();
    return;
  }
  Serial.println("G ok");
    if(firstletter==1)
  {
    goto no_left6;
  }
  north();
  no_left6:
  cmon();
  moveF(1200);
  east();
  moveF(600);
  cmoff();
  south();
  moveF(1200);
  west();
  cmon();
  moveF(600);
  cmoff();
  north();
  moveF(600);
  east();
  moveF(300);
  cmon();
  moveF(300);
  south();
  moveF(600);
  cmoff();
  east();
  moveF(600);
  delay(1000);
  Serial.print("G DONE");
  return;
 }

void printH()
 {
    if(Type=="S"){
    printHs();
    return;
  }
  Serial.println("H ok");
   if(firstletter==1)
  {
    goto no_left7;
  }
  north();
  no_left7:
  cmon();
  moveF(1200);
  cmoff();
  east();
  moveF(600);
  south();
  cmon();
  moveF(1200);
  cmoff();
  west();
  moveF(600);
  north();
  cmon();
  moveF(600);
  east();
  moveF(600);
  cmoff();
  south();
  moveF(600);
  east();
  moveF(600);
  delay(1000);
  Serial.println("H DONE");
  return;
 }

void printI()
  {
    if(Type=="S"){
    printIs();
    return;
  }
  Serial.println("I ok");
  if(firstletter==1)
  {
    goto no_left8;
  }
  north();
  no_left8:
  moveF(1200);
  east();
  cmon();
  moveF(600);
  cmoff();
  south();
  moveF(1200);
  west();
  cmon();
  moveF(600);
  cmoff();
  east();
  moveF(300);
  north();
  cmon();
  moveF(1200);
  cmoff();
  east();
  moveF(300);
  south();
  moveF(1200);
  east();
  moveF(600);
  delay(1000);
  Serial.println("I DONE");
  return;
 }

void printJ()
 {
    if(Type=="S"){
    printJs();
    return;
  }
  Serial.println("J ok");
   if(firstletter==1)
  {
       goto no_left9;
  }
  north();
  no_left9:
  moveF(1200);
  east();
  cmon();
  moveF(600);
  cmoff();
  west();
  moveF(300);
  south();
  cmon();
  moveF(1200);
  cmoff();
  west();
  cmon();
  moveF(300);
  cmoff();
  east();
  moveF(1200);
  delay(1000);
  Serial.println("J DONE");
  return;

 }

void printK()
 {
    if(Type=="S"){
    printKs();
    return;
  }
  Serial.println("K ok");
   if(firstletter==1)
  {
    goto no_left8;
  }
  north();
  no_left8:
  cutl(1200);
  south();
  moveF(600);
  ner();
  cutl(848);
  sw();
  moveF(848);
  se();
  cutl(848);
  east();
  moveF(600);
  delay(1000);
  Serial.println("J DONE");
  return;
 }

void printL()
 {
    if(Type=="S"){
    printLs();
    return;
  }
  
  Serial.println("L ok");
   if(firstletter==1)
  {
    goto no_left8;
  }
  north();
  no_left8:
  cutl(1200);
  south();
  moveF(1200);
  east();
  cutl(600);
  moveF(600);
  delay(1000);
  Serial.println("LDONE");
  return;
 }

void printM()
 {
    if(Type=="S"){
    printMs();
    return;
  }
  Serial.println("M ok");
  if(firstletter==1)
  {
    goto no_left8;
  }
  north();
  no_left8:
  cutl(1200);
  lse();
  cutl(670);
  lner();
  cutl(670);
  south();
  cutl(1200);
  east();
  moveF(600);
  delay(1000);
  Serial.println("LDONE");
  return;
 }

void printN()
 {
    if(Type=="S"){
    printNs();
    return;
  }
  Serial.println("N ok");
    if(firstletter==1)
  {
    goto no_left8;
  }
  north();
  no_left8:
  cutl(1200);
  lse();
  cutl(1341);
  north();
  cutl(1200);
  south();
  moveF(1200);
  east();
  moveF(600);
  delay(1000);
  Serial.println("LDONE");
  return;
 }

void printO()
 {
    if(Type=="S"){
    printOs();
    return;
  }
  Serial.println("O ok");
  if(firstletter==1)
  {
    goto no_left8;
  }
  north();
  no_left8:
  cutl(1200);
  east();
  cutl(600);
  south();
  cutl(1200);
  west();
  cutl(600);
  east();
  moveF(1200);
  delay(1000);
  Serial.println("O DONE");
  return;
 }

void printP()
 {
    if(Type=="S"){
    printPs();
    return;
  }
  Serial.println("P ok");
  if(firstletter==1)
  {
    goto no_left8;
  }
  north();
  no_left8:
  cutl(1200);
  east();
  cutl(600);
  south();
  cutl(600);
  west();
  cutl(600);
  south();
  moveF(600);
  east();
  moveF(1200);
  delay(1000);
  Serial.println("P DONE");
  return;
 }

void printQ()
 {
    if(Type=="S"){
    printQs();
    return;
  }
  Serial.println("Q ok");
   if(firstletter==1)
  {
    goto no_left8;
  }
  north();
  no_left8:
  cutl(1200);
  east();
  cutl(600);
  south();
  cutl(1200);
  west();
  cutl(600);
  east();
  moveF(600);
  nwr();
  cutl(435);
  lse();
  moveF(435);
  east();
  moveF(600);
  delay(1000);
  Serial.println("LDONE");
  return;
 }

 void printR()
 {
    if(Type=="S"){
    printRs();
    return;
  }
  Serial.println("R ok");
  if(firstletter==1)
  {
    goto no_left8;
  }
  north();
  no_left8:
  cutl(1200);
  east();
  cutl(600);
  south();
  cutl(600);
  west();
  cutl(600);
  ser();
  cutl(848);
  east();
  moveF(600);
  delay(1000);
  Serial.println("S DONE");
  return; 
 }

void printS()
 {
    if(Type=="S"){
    printSs();
    return;
  }
  Serial.println("S ok");
  if(firstletter==1)
  {
    goto no_left8;
  }
  north();
  no_left8:
  moveF(600);
  cutl(600);
  east();
  cutl(600);
  south();
  moveF(600);
  cutl(600);
  west();
  cutl(600);
  north();
  moveF(600);
  east();
  cutl(600);
  south();
  moveF(600);
  east();
  moveF(600);
  delay(1000);
  Serial.println("S DONE");
  return; 
 }

void printT()
 {
    if(Type=="S"){
    printTs();
    return;
  }
  Serial.println("T ok");
   if(firstletter==1)
  {
    goto no_left8;
  }
  north();
  no_left8:
  moveF(1200);
  east();
  cutl(600);
  west();
  moveF(300);
  south();
  cutl(1200);
  east();
  moveF(900);
  delay(1000);
  Serial.println("S DONE");
  return; 
 }

void printU()
 {
    if(Type=="S"){
    printUs();
    return;
  }
  Serial.println("U ok");
  if(firstletter==1)
  {
    goto no_left8;
  }
  north();
  no_left8:
  cutl(1200);
  east();
  moveF(600);
  south();
  cutl(600);
  west();
  cutl(600);
  east();
  moveF(1200);
  delay(1000);
  Serial.println("U DONE");
  return; 
 }

void printV()
 {
    if(Type=="S"){
    printVs();
    return;
  }
  Serial.println("V ok");
   if(firstletter==1)
  {
    goto no_left8;
  }
  north();
  no_left8:
  moveF(1200);
  vd();
  cutl(1236);
  vu();
  cutl(1236);
  south();
  moveF(1200);
  east();
  moveF(600);
  delay(1000);
  Serial.println("U DONE");
  return; 
 }

void printW()
 {
    if(Type=="S"){
    printWs();
    return;
  }
  Serial.println("W ok");
   if(firstletter==1)
  {
    goto no_left8;
  }
  north();
  no_left8:
  cutl(1200);
  east();
  moveF(600);
  south();
  cutl(1200);
  lnw();
  cutl(670);
  lswr();
  cutl(670);
  east();
  moveF(1200);
  delay(1000);
  Serial.println("U DONE");
  return; 
 }

 void printX()
 {
    if(Type=="S"){
    printXs();
    return;
  }
  Serial.println("X ok");
   if(firstletter==1)
  {
    goto no_left8;
  }
  north();
  no_left8:
  moveF(1200);
  lse();
  cutl(1341);
  north();
  moveF(1200);
  lswr();
  cutl(1341);
  east();
  moveF(1200);
  delay(1000);
  Serial.println("U DONE");
  return; 
 }

void printY()
 {
    if(Type=="S"){
    printYs();
    return;
  }
  Serial.println("Y ok");
   if(firstletter==1)
  {
    goto no_left8;
  }
  north();
  no_left8:
  moveF(1200);
  east();
  moveF(600);
  lsw();
  cutl(1341);
  north();
  moveF(1200);
  lse();
  cutl(670);
  moveF(671);
  east();
  moveF(600);
  Serial.println("U DONE");
  return; 
 }

void printZ()
 {
    if(Type=="S"){
    printZs();
    return;
  }
  Serial.println("Z ok");
   if(firstletter==1)
  {
    goto no_left8;
  }
  north();
  no_left8:
  moveF(1200);
  east();
  cutl(600);
  lsw();
  cutl(1341);
  east();
  cutl(600);
  moveF(600);
  delay(1000);
  return;
 }



  void printAs()
 {
  
  Serial.println("A ok");
   if(firstletter==1)
  {
    goto no_left0;
  }
  norths();
  no_left0:
  sprayl(1200);
  easts();
  sprayl(600);
  souths();
  sprayl(1200);
  wests();
  moveF(600);
  norths();
  moveF(600);
  easts();
  sprayl(600);
  souths();
  moveF(600);
  eastsr();
  moveF(600);
  delay(1000);
  Serial.println("A DONE");
  return;
  }

void printBs()
 {
  Serial.println("B ok");
  if(firstletter==1)
  {
    goto no_left1;
  }
  norths();
  no_left1:
  sprayl(1200);
  easts();
  sprayl(600);
  souths();
  sprayl(1200);
  wests();
  sprayl(600);
  norths();
  moveF(600);
  easts();
  sprayl(600);
  souths();
  moveF(600);
  eastsr();
  moveF(600);
  delay(1000);
  Serial.println("B DONE");
  return;
 }

void printCs()
 {
  Serial.println("C ok");
    if(firstletter==1)
  {
    goto no_left2;
  }
  norths();
  no_left2:
  sprayl(1200);
  easts();
  sprayl(600);
  souths();
  moveF(1200);
  wests();
  sprayl(600);
  easts();
  moveF(1200);
  delay(1000);
  Serial.println("C DONE");
  return;
 }

void printDs()
 {
  Serial.println("D ok");
    if(firstletter==1)
  {
    goto no_left3;
  }
  norths();
  no_left3:
 sprayl(1200);
  westsr();
  sprayl(200);
  easts();
  sprayl(800);
  souths();
  sprayl(1200);
  wests();
  sprayl(800);
  easts();
  moveF(1400);
  delay(1000);
  Serial.print("D DONE");
  return;
 }

void printEs()
 {
  Serial.println("E ok");
    if(firstletter==1)
  {
    goto no_left4;
  }
  norths();
  no_left4:
sprayl(1200);
  easts();
sprayl(600);
  souths();
  moveF(600);
  wests();
sprayl(600);
  souths();
  moveF(600);
  eastsr();
sprayl(600);
  moveF(600);
  delay(1000);
  Serial.print("E DONE");
  return;
  
 }

void printFs()
 {
  Serial.println("F ok");
    if(firstletter==1)
  {
    goto no_left5;
  }
  norths();
  no_left5:
sprayl(1200);
  easts();
sprayl(600);
  souths();
  moveF(600);
  wests();
sprayl(600);
  souths();
  moveF(600);
  eastsr();
  moveF(1200);
  delay(1000);
  Serial.print("F DONE");
  return;
 }

void printGs()
 {
  Serial.println("G ok");
    if(firstletter==1)
  {
    goto no_left6;
  }
  norths();
  no_left6:
sprayl(1200);
  easts();
sprayl(600);
  souths();
  moveF(1200);
  wests();
sprayl(600);
  norths();
  moveF(600);
  easts();
  moveF(300);
sprayl(300);
  souths();
sprayl(600);
  eastsr();
  moveF(600);
  delay(1000);
  Serial.print("G DONE");
  return;
 }

void printHs()
 {
  Serial.println("H ok");
   if(firstletter==1)
  {
    goto no_left7;
  }
  norths();
  no_left7:
sprayl(1200);
  easts();
  moveF(600);
  souths();
sprayl(1200);
  wests();
  moveF(600);
  norths();
  moveF(600);
  easts();
sprayl(600);
  souths();
  moveF(600);
  eastsr();
  moveF(600);
  delay(1000);
  Serial.println("H DONE");
  return;
 }

void printIs()
 {
  Serial.println("I ok");
  if(firstletter==1)
  {
    goto no_left8;
  }
  north();
  delay(3000);
  no_left8:
 moveF(1200);
  easts();
sprayl(600);
  souths();
  moveF(1200);
  wests();
sprayl(600);
  easts();
  moveF(300);
  northsr();
sprayl(1200);
  souths();
  moveF(1200);
  eastsr();
  moveF(1200);
  delay(1000);
  Serial.println("I DONE");
  return;
 }

void printJs()
 {
  Serial.println("J ok");
   if(firstletter==1)
  {
       goto no_left9;
  }
  norths();
  no_left9:
  moveF(1200);
  easts();
sprayl(600);
  wests();
  moveF(300);
  souths();
sprayl(1200);
  wests();
sprayl(300);
  easts();
  moveF(1200);
  delay(1000);
  Serial.println("J DONE");
  return;
  
 }

void printKs()
 {
  Serial.println("K ok");
  if(firstletter==1)
  {
    goto no_left8;
  }
  norths();
  no_left8:
  sprayl(1200);
  souths();
  moveF(600);
  nesr();
  sprayl(848);
  sws();
  moveF(848);
  ses();
  sprayl(848);
  eastsr();
  moveF(600);
  delay(1000);
  Serial.println("J DONE");
  return;
 }

void printLs()
 {
  Serial.println("L ok");
  if(firstletter==1)
  {
    goto no_left8;
  }
  norths();
  no_left8:
  sprayl(1200);
  souths();
  moveF(1200);
  eastsr();
  sprayl(600);
  moveF(600);
  delay(1000);
  Serial.println("LDONE");
  return;
 }

void printMs()
 {
  Serial.println("M ok");
  if(firstletter==1)
  {
    goto no_left8;
  }
  norths();
  no_left8:
  sprayl(1200);
  lses();
  sprayl(670);
  lnesr();
  sprayl(670);
  souths();
  sprayl(1200);
  eastsr();
  moveF(600);
  delay(1000);
  Serial.println("LDONE");
  return;
 }

void printNs()
 {
  Serial.println("N ok");
    if(firstletter==1)
  {
    goto no_left8;
  }
  norths();
  no_left8:
  sprayl(1200);
  lses();
  sprayl(1341);
  northsr();
  sprayl(1200);
  souths();
  moveF(1200);
  eastsr();
  moveF(600);
  delay(1000);
  Serial.println("LDONE");
  return;
 }

void printOs()
 {
  Serial.println("O ok");
  if(firstletter==1)
  {
    goto no_left8;
  }
  norths();
  no_left8:
  sprayl(1200);
  easts();
  sprayl(600);
  souths();
  sprayl(1200);
  wests();
  sprayl(600);
  easts();
  moveF(1200);
  delay(1000);
  Serial.println("O DONE");
  return;
 }

void printPs()
 {
  Serial.println("P ok");
  if(firstletter==1)
  {
    goto no_left8;
  }
  norths();
  no_left8:
  sprayl(1200);
  easts();
  sprayl(600);
  souths();
  sprayl(600);
  wests();
  sprayl(600);
  southsr();
  moveF(600);
  eastsr();
  moveF(1200);
  delay(1000);
  Serial.println("P DONE");
  return;
 }

void printQs()
 {
  Serial.println("Q ok");
   if(firstletter==1)
  {
    goto no_left8;
  }
  norths();
  no_left8:
  sprayl(1200);
  easts();
  sprayl(600);
  souths();
  sprayl(1200);
  wests();
  sprayl(600);
  easts();
  moveF(600);
  nwsr();
  sprayl(435);
  lses();
  moveF(435);
  eastsr();
  moveF(600);
  delay(1000);
  Serial.println("LDONE");
  return;
 }

 void printRs()
 {
  Serial.println("R ok");
   if(firstletter==1)
  {
    goto no_left8;
  }
  norths();
  no_left8:
  sprayl(1200);
  easts();
  sprayl(600);
  souths();
  sprayl(600);
  wests();
  sprayl(600);
  sesr();
  sprayl(848);
  eastsr();
  moveF(600);
  delay(1000);
  Serial.println("S DONE");
  return; 
 }

void printSs()
 {
  Serial.println("S ok");
  if(firstletter==1)
  {
    goto no_left8;
  }
  norths();
  no_left8:
  moveF(600);
  sprayl(600);
  easts();
  sprayl(600);
  souths();
  moveF(600);
  sprayl(600);
  wests();
  sprayl(600);
  norths();
  moveF(600);
  easts();
  sprayl(600);
  souths();
  moveF(600);
  eastsr();
  moveF(600);
  delay(1000);
  Serial.println("S DONE");
  return; 
 }

void printTs()
 {
  Serial.println("T ok");
   if(firstletter==1)
  {
    goto no_left8;
  }
  norths();
  no_left8:
  moveF(1200);
  easts();
  sprayl(600);
  wests();
  moveF(300);
  southsr();
  sprayl(1200);
  eastsr();
  moveF(900);
  delay(1000);
  Serial.println("S DONE");
  return; 
 }

void printUs()
 {
  Serial.println("U ok");
  if(firstletter==1)
  {
    goto no_left8;
  }
  norths();
  no_left8:
  sprayl(1200);
  easts();
  moveF(600);
  souths();
  sprayl(600);
  wests();
  sprayl(600);
  easts();
  moveF(1200);
  delay(1000);
  Serial.println("U DONE");
  return; 
 }

void printVs()
 {
  Serial.println("V ok");
  if(firstletter==1)
  {
    goto no_left8;
  }
  norths();
  no_left8:
  moveF(1200);
  vds();
  sprayl(1236);
  vus();
  cutl(1236);
  souths();
  moveF(1200);
  eastsr();
  moveF(600);
  delay(1000);
  Serial.println("U DONE");
  return; 
 }

void printWs()
 {
  Serial.println("W ok");
   if(firstletter==1)
  {
    goto no_left8;
  }
  north();
  no_left8:
  sprayl(1200);
  easts();
  moveF(600);
  souths();
  sprayl(1200);
  lnws();
  sprayl(670);
  lswsr();
  sprayl(670);
  eastsr();
  moveF(1200);
  delay(1000);
  Serial.println("U DONE");
  return; 
 }

 void printXs()
 {
  Serial.println("X ok");
  if(firstletter==1)
  {
    goto no_left8;
  }
  norths();
  no_left8:
  moveF(1200);
  lses();
  sprayl(1341);
  northsr();
  moveF(1200);
  lswsr();
  sprayl(1341);
  eastsr();
  moveF(1200);
  delay(1000);
  Serial.println("U DONE");
  return; 
 }

void printYs()
 {
  Serial.println("Y ok");
  if(firstletter==1)
  {
    goto no_left8;
  }
  norths();
  no_left8:
  moveF(1200);
  easts();
  moveF(600);
  lsws();
  sprayl(1341);
  norths();
  moveF(1200);
  lses();
  sprayl(670);
  moveF(671);
  eastsr();
  moveF(600);
  Serial.println("U DONE");
  return; 
 }

void printZs()
 {
  Serial.println("Z ok");
   if(firstletter==1)
  {
    goto no_left8;
  }
  norths();
  no_left8:
  moveF(1200);
  easts();
  sprayl(600);
  lsws();
  sprayl(1341);
  eastsr();
  sprayl(600);
  moveF(600);
  delay(1000);
  return;
 }
