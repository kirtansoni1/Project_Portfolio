#include <DFRobot_SIM808.h>
#include <sim808.h>
#include <Wire.h>
#include <Servo.h>
#include <Robojax_WCS.h>
#include <PS2X_lib.h>

//Slave address of arduino Mega on R-Pi4
#define Slave_Addr 0x69

//Command to store what the RPi sends
byte byte_receive_Rpi;

//Change to high for debugging mode(Serial monitor)
bool Debug = LOW;

//OUTPUT PINS --------------------------------------------------------------------------------------------------------------------------

//Defining 8 Base Motor Pins ------ (D-->Digital Pins, P-->PWM Pins)
#define BM_D1 23
#define BM_P1 2
#define BM_D2 24
#define BM_P2 3
#define BM_D3 25
#define BM_P3 4
#define BM_D4 26
#define BM_P4 5
#define BM_D5 27
#define BM_P5 6
#define BM_D6 28
#define BM_P6 7
#define BM_D7 29
#define BM_P7 8
#define BM_D8 30
#define BM_P8 9

//Defining 2 Cutter mechanism motor pins ------ (D-->Digital Pins, P-->PWM Pins)
#define L_CMM_D1 31  //Linear Cutter mechanism motor forward/backward
#define L_CMM_P1 10
#define CMM_D2 32  //Cutter mechanism motor up/down
#define CMM_P2 11

//Defining 1 Scissor mechanism motor pins ------ (D-->Digital Pin, P-->PWM Pin)
#define SMM_D1 33
#define SMM_P1 12

//Defining 1 Cutter motor pins ------ (P-->PWM Pin)
#define CM_P 13

//Defining Compressor output pins
#define CR_1 42
#define CR_2 43

//Defining solenoid valve output pins
#define T_M 44
#define L_S 45
#define L_E 46
#define R_S 47
#define R_E 48

//Defining Robot LED pin
#define LED 1

//Defining robot mode led indicator
#define RC_LED 52
#define AUTO_LED 51

//INPUT PINS------------------------------------------------------------------------------------------------------------------------------

//Defining Proxymity sensors input pins
#define S_D1 34  //For Scissor Mechanism
#define S_D2 35  //Extra Slot
#define S_D3 36  //For Linear Cutter Mechanism
#define S_D4 37  //For Cutter pivot Mechanism
#define S_D5 38  //For Right Leg Home
#define S_D6 39  //For Left Leg Home
#define S_D7 40  //Overload Dtection 1
#define S_D8 41  //Overload Detectioin 2

//Defining Voltage sensor input pin
#define V_Read A0

//Defining Current sensor input pin
#define CS_A A1
#define CS_D 22

//Defining robot mode input pin
#define MODE 53

//GLOBAL VARIABLES--------------------------------------------------------------------------------------------------------------------------

//Defining Stop signal for all motors (PLEASE DO NOT CAHNGE IT'S VALUE)
const int Motor_Stop = 0;

//Speed and Direction set of Base Motor (Speed ----> 0->STOP - 255->HIGHEST and if direction wrong then change BM_Motor_Direction)
const int BM_Full_Speed = 220; //Set the Full speed of base motors as per your need (190-255)
const int BM_Half_Speed = 127; //Set the Half speed of base motors as per your need (100-154)
const bool BM_Motor_Direction = LOW;  //Change here to correct directions (LOW/HIGH)
//******************************************************************************************************************************************


//Linear Cutter Mechanism Motor (Speed ----> 0->STOP - 255->HIGHEST and if direction wrong then change L_CMM_Direction)
const int L_CMM_Speed = 150;
const int L_CMM_Direction = LOW;
bool L_CMM_Error = LOW;
const int L_CMMM_Error_Timeout = 20000; //Change to set the error trigger delay of linear cutter mechanism
//******************************************************************************************************************************************


//Cutter Mechanism Motor (Speed ----> 0->STOP - 255->HIGHEST and if direction wrong then change CMM_Direction)
const int CMM_Speed = 150;
const int CMM_Direction = LOW;
//******************************************************************************************************************************************


//Scissor Mechanism Motor (Speed ----> 0->STOP - 255->HIGHEST and if direction wrong then change SMM_Direction)
const int SMM_Speed = 150;
const int SMM_Direction = LOW;
bool SMM_Error = LOW;
const int SMM_Error_Timeout = 20000; //Change to set the error trigger delay of Scissor mechanism
//******************************************************************************************************************************************

//Cutter motor settings (1750->Full reverse speed - 1250->Full forward speed 1500->Cutter motor stop) & Current sensor settings
Servo CM;
const int CM_Full_Forward_Speed = 1250; //Full forward speed
const int CM_Full_Reverse_Speed = 1750;  //Full reverse speed
const int CM_Stop = 1500; //Cutter motor stop
bool CM_Error_1 = LOW;
bool CM_Error_2 = LOW;
bool CM_Error_1_Direction = LOW;
#define MODEL 9
#define ZERO_CURRENT_WAIT_TIME 5000 //wait for 5 seconds to allow zero current measurement
#define CORRECTION_VALUE 164 //mA
#define MEASUREMENT_ITERATION 100
#define VOLTAGE_REFERENCE  5000.0 //5000mv is for 5V
#define BIT_RESOLUTION 10
#define DEBUT_ONCE true
#define SENSOR_VCC_PIN 49
#define ZERO_CURRENT_LED_PIN 50
Robojax_WCS sensor(
  MODEL, CS_A, SENSOR_VCC_PIN,
  ZERO_CURRENT_WAIT_TIME, ZERO_CURRENT_LED_PIN,
  CORRECTION_VALUE, MEASUREMENT_ITERATION, VOLTAGE_REFERENCE,
  BIT_RESOLUTION, DEBUT_ONCE);
bool Cutting_Motor_ON = LOW; //Trun ON and OFF cutter motor loop
bool CM_Error_1_delay = LOW; //To add delay when motor changes direction
int CM_Error_1_Count = 0; //To Make sure that the current is above threshold
int CM_Error_1_Forward_Count = 0; //To Make sure that the current is above threshold for detection Error 2 when motor is rotating forward
int CM_Error_1_Reverse_Count = 0; //To Make sure that the current is above threshold for detection Error 2 when motor is rotating reverse
bool CM_Error_1_OFF = LOW; //To trun off the error 1 loop when error 2 is occuring
int CM_Error_1_OFF_Count = 0; //To deactivate the error 1 if it is resolved
bool CM_Error_2_OFF = LOW; //To activate the pivot mechanism only thrice before generating Motor stuck error
//*******************************************************************************************************************************************

//Tilting mechanism settings
bool T_M_State = LOW; //To ON and OFF the GPIO of tilting mechanism
bool T_M_Trigger = LOW; //To change the tilting mechanism state only once
int T_M_Count = 0; //To make sure that the Cutter motor is stuck for some cycles before giving indication
int T_M_Count_Low = 0; //To deactivate the error 2 if it is resolved
int T_M_Trigger_Count = 0; //To count the number of times pivot is activated
//*******************************************************************************************************************************************

//Leg Extension mechanism
bool R_E_Error = LOW;
const int R_E_Error_Timeout = 10000; //Change to set the error trigger delay of Right leg extension
bool L_E_Error = LOW;
const int L_E_Error_Timeout = 10000; //Change to set the error trigger delay of Left leg extension
//*******************************************************************************************************************************************


//Voltage sensor settings
#define Sample_Number_Voltage 10 //Number of samples taken for voltage measurement
float Voltage_Sum = 0.0;
unsigned int Sample_Count_Voltage = 0; //Count int for samples
float Voltage = 0.0;
bool Low_Voltage = LOW;
const float Voltage_Reference = 11.0; //Change to calibrate the voltage sensor
const float Low_Voltage_Trigger = 24; //Change to set the low voltage trigger value
const float Full_Battery_Trigger = 29.2; //Change to set the full battery voltage trigger value
const int Voltage_Read_Interval = 30000; //Frequency of measuring voltage in miliseconds
unsigned long current_time_voltage = 0;
unsigned long last_time_voltage = 0;
//*******************************************************************************************************************************************

//GSM & GPS Settings
#define Incoming_Message_Length 160
char Incoming_Message[Incoming_Message_Length];
int Incoming_Message_Index = 0;
char Send_Message[300];
char Latitude[12];
char Longitude[12];
char Master_Phone[16];
char datetime[24];
DFRobot_SIM808 sim808(&Serial1);//Connect RX,TX
unsigned long GPS_Current_Time = 0;
unsigned long GPS_Last_Time = 0;
const int GPS_Send_Interval = 300000; //Sending GPS coordinates to the master server in milliseconds
//*******************************************************************************************************************************************

//Overload Detection
int Overload = 0; //To detect overload on the robot
const int Overload_Tolerance_Cycles = 50; //Overload tolerance timeout
int Pivot_Overload = 0; //To detect overload in the cutter mechanism

//Remote control settings
PS2X ps2x;
int error = 0;
byte type = 0;
byte vibrate = 255;

bool Remote_Activated = LOW;
unsigned long last_time = 0;
unsigned long current_time = 0;

bool Blue_State = LOW;
bool Red_State = LOW;
bool Green_State = LOW;
bool ROBOT_MODE = LOW; //HIGH = Remote control mode & LOW = Automatic mode
bool Respond_Once_RPI = LOW; //Used to respond only once when the mode is switched
bool Last_MODE = LOW;
bool Remote_Init = LOW;

int Remote_Extension_Wait_Time = 5; //Time to wait before deploying supports in milliseconds, when in remote control mode

//Speed ----> 0->STOP - 255->HIGHEST, you can set mechanism speed when in remote control mode
const int CMM_Speed_Remote = 150;
const int L_CMM_Speed_Remote = 150;
const int SMM_Speed_Remote = 150;
//********************************************************************************************************************************************

void setup() {
  Serial.begin(57600);
  Serial1.begin(9600);

  Wire.begin(Slave_Addr); //Enables I2C between Rpi(Master) and Arduino(Slave)
  Wire.onReceive(Received_Event); //Interrupt for reading what Rpi has to send
  Wire.onRequest(Respond);  //Interrupt for sending bytes when Rpi asks

  CM.attach(CM_P);  //Attaches cutter motor pin to Ardunio
  Read_Battery_Voltage();  //Measures battery voltage
  sensor.start();  //Initiates the current sensor

  //******** Initialize remote control interface *************
  for (int i = 0; i < 5; i++) { //Connects to the PS2 controller and returns the error occured
    error = ps2x.config_gamepad(17, 15, 16, 14, false, true);
  }  //PS2 controller (clock, command, attention, data, Pressures?, Vibration?)

  if (error == 0) {
    if (Debug == HIGH) {
      Serial.print("Found Controller, configured successful ");
    }
  }
  else if (error == 1) {
    if (Debug == HIGH) {
      Serial.println("No controller found, check wiring.");
    }
  }
  else if (error == 2) {
    if (Debug == HIGH) {
      Serial.println("Controller found but not accepting commands.");
    }
  }
  else if (error == 3) {
    if (Debug == HIGH) {
      Serial.println("Controller refusing to enter Pressures mode, may not support it. ");
    }
  }

  type = ps2x.readType(); //To detect the type of controller
  switch (type) {
    case 0:
      Serial.println("Unknown Controller type");
      break;
    case 1:
      Serial.println("DualShock Controller Found");
      break;
    case 2:
      Serial.println("Please attach DualShock Controller");
      break;
  }

  //******** Initialize sim808 module *************
  while (!sim808.init())
  {
    if (Debug == HIGH) {
      Serial.print("Sim808 init error\r\n");
    }
    delay(1000);
  }
  delay(3000);
  if (sim808.attachGPS()) {
    if (Debug == HIGH) {
      Serial.println("Open the GPS power success");
    }
  }
  else {
    if (Debug == HIGH) {
      Serial.println("Open the GPS power failure");
    }
  }

  //Stopping everything and returning robot to the original postion
  Stop_Everything();

  //Defining the pin modes for all the outputs
  pinMode(BM_D1, OUTPUT);
  pinMode(BM_P1, OUTPUT);
  pinMode(BM_D2, OUTPUT);
  pinMode(BM_P2, OUTPUT);
  pinMode(BM_D3, OUTPUT);
  pinMode(BM_P3, OUTPUT);
  pinMode(BM_D4, OUTPUT);
  pinMode(BM_P4, OUTPUT);
  pinMode(BM_D5, OUTPUT);
  pinMode(BM_P5, OUTPUT);
  pinMode(BM_D6, OUTPUT);
  pinMode(BM_P6, OUTPUT);
  pinMode(BM_D7, OUTPUT);
  pinMode(BM_P7, OUTPUT);
  pinMode(BM_D8, OUTPUT);
  pinMode(BM_P8, OUTPUT);
  pinMode(L_CMM_D1, OUTPUT);
  pinMode(L_CMM_P1, OUTPUT);
  pinMode(CMM_D2, OUTPUT);
  pinMode(CMM_P2, OUTPUT);
  pinMode(SMM_D1, OUTPUT);
  pinMode(SMM_P1, OUTPUT);
  pinMode(CM_P, OUTPUT);
  pinMode(CR_1, OUTPUT);
  pinMode(CR_2, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(RC_LED, OUTPUT);
  pinMode(AUTO_LED, OUTPUT);

  //Defining the pin modes for all the inputs
  pinMode(S_D1, INPUT);
  pinMode(S_D2, INPUT);
  pinMode(S_D3, INPUT);
  pinMode(S_D4, INPUT);
  pinMode(S_D5, INPUT);
  pinMode(S_D6, INPUT);
  pinMode(S_D7, INPUT);
  pinMode(S_D8, INPUT);
  pinMode(CS_D, INPUT);
  pinMode(MODE, INPUT);

  Last_MODE = digitalRead(MODE); //To save the state of robot mode

  if (Debug == HIGH) {
    Serial.println("Starting UGV...");
  }
  delay(2000);
}

void loop() {

  if (digitalRead(MODE) != Last_MODE) { //For sending the response of mode to raspberry only once when the mode is switched
    Respond_Once_RPI = LOW;
    Last_MODE = digitalRead(MODE);
  }

  if (digitalRead(MODE) == HIGH) { //Turn ON remote control mode
    ROBOT_MODE = HIGH;
    digitalWrite(RC_LED, HIGH);
    digitalWrite(AUTO_LED, LOW);
    if (Respond_Once_RPI == LOW) {
      Respond(0x04);
      Respond_Once_RPI = HIGH;
    }
  }
  else if (digitalRead(MODE) == LOW) { //Turn OFF remote control mode (Switch to automatic mode)
    ROBOT_MODE = LOW;
    digitalWrite(RC_LED, LOW);
    digitalWrite(AUTO_LED, HIGH);
    if (Respond_Once_RPI == LOW) {
      Respond(0x28);
      Respond_Once_RPI = HIGH;
    }
  }

  if (type == 1 && ROBOT_MODE == HIGH) { //Reading the controller if remote control mode is activated
    Read_PS2();
  }

  //Continuously keep track of Scissor mechanism if sensor output is triggered then stop
  if (digitalRead(S_D1) == HIGH) {
    analogWrite(SMM_P1, Motor_Stop);
  }
  //Continuously keep track of Linear cutter mechanism if sensor output is triggered then stop
  if (digitalRead(S_D3) == HIGH) {
    analogWrite(L_CMM_P1, Motor_Stop);
  }

  //To keep track of the battery voltage levels
  current_time_voltage = millis();
  if (current_time_voltage - last_time_voltage > Voltage_Read_Interval) {
    last_time_voltage = current_time_voltage;
    Read_Battery_Voltage();
    if (Voltage > 24) {
      Low_Voltage = LOW;
    }
  }

  //To send GPS coordinates to the master server only if Remote control mode is deactivated
  GPS_Current_Time = millis();
  if (GPS_Current_Time - GPS_Last_Time > GPS_Send_Interval && ROBOT_MODE == LOW) {
    GPS_Last_Time = GPS_Current_Time;
    Send_GPS();
  }

  //Measures current running from the cutter motor
  if(ROBOT_MODE == LOW){
  sensor.readCurrent();
  if (Debug == HIGH) {
    sensor.printCurrent();
  }
 }

  //Calls cutter motor ON function if RPi sends command
  if (Cutting_Motor_ON == HIGH && ROBOT_MODE == LOW) {
    CM_ON(); //Call the cutting motor loop
    Respond(0x01);
  }

  //Check for robot overload, but only when the robot is in automatic mode
  if (digitalRead(S_D7) == HIGH && digitalRead(S_D8) == HIGH && ROBOT_MODE == LOW) {
    Overload++;
    if (Overload > Overload_Tolerance_Cycles) { //Make sure the robot is overloading for more than several cycles
      Send_Error(0x2E); //Send error that robot is overloading
      Overload = 0;
    }
  }

  if (digitalRead(S_D4) == HIGH && ROBOT_MODE == LOW) {
    //check if pivot mechanism is overloding only when robot is in automatic mode
    Pivot_Overload++;
    if (Pivot_Overload > Overload_Tolerance_Cycles) {
      Send_Error(0x2F);
      Pivot_Overload = 0;
    }
  }
  if (Low_Voltage == HIGH) {
    Low_Voltage = LOW;
    Stop_Everything(); //Stop everything when the battery is low
  }
}

void Received_Event(int bytecount) {

  //To clear the bytes stored before storing new
  byte_receive_Rpi = 0x00;

  for (int i = 0; i < bytecount; i++) {
    byte_receive_Rpi = Wire.read();
  }

  if (Debug == HIGH) {
    Serial.print("Recieved command from R-Pi:");
    Serial.println(byte_receive_Rpi);
  }

  if(ROBOT_MODE == LOW){
    switch (byte_receive_Rpi) {
      case 0x05:
        Full_Forward();
        break;
      case 0x06:
        Half_Forward();
        break;
      case 0x07:
        Full_Backward();
        break;
      case 0x08:
        Half_Backward();
        break;
      case 0x09:
        Left_Full_Forward();
        break;
      case 0x0A:
        Right_Full_Forward();
        break;
      case 0x0B:
        Left_Half_Forward();
        break;
      case 0x0C:
        Right_Half_Forward();
        break;
      case 0x0D:
        Cutting_ON();
        break;
      case 0x0E:
        Stop_Cutting();
        break;
      case 0x0F:
        Stop_Motion();
        break;
      case 0x10:
        Stop_Everything();
        break;
      case 0x11:
        Right_Compressor_ON();
        break;
      case 0x12:
        Left_Compressor_ON();
        break;
      case 0x13:
        Right_Compressor_OFF();
        break;
      case 0x14:
        Left_Compressor_OFF();
        break;
      case 0x15:
        Right_Support_Deploy();
        break;
      case 0x16:
        Left_Support_Deploy();
        break;
      case 0x17:
        Right_Support_Home();
        break;
      case 0x18:
        Left_Support_Home();
        break;
      case 0x19:
        Right_Extension_Deploy();
        break;
      case 0x1A:
        Left_Extension_Deploy();
        break;
      case 0x1B:
        Right_Extension_Home();
        break;
      case 0x1C:
        Left_Extension_Home();
        break;
      case 0x1D:
        Scissor_Mech_UP();
        break;
      case 0x1E:
        Scissor_Mech_DOWN();
        break;
      case 0x1F:
        Scissor_Mech_Home();
        break;
      case 0x20:
        Scissor_Mech_Stop();
        break;
      case 0x21:
        Linear_Cutter_Mech_Forward();
        break;
      case 0x22:
        Linear_Cutter_Mech_Backward();
        break;
      case 0x23:
        Linear_Cutter_Mech_Home();
        break;
      case 0x24:
        Linear_Cutter_Mech_Stop();
        break;
      case 0x25:
        Cutter_Mech_UP();
        break;
      case 0x26:
        Cutter_Mech_DOWN();
        break;
      case 0x27:
        Cutter_Mech_Stop();
        break;
    }
  }
}

//Sending response to the R-Pi
void Respond(byte message)
{
  if (Debug == HIGH) {
    Serial.println("Responding to RPi...");
  }
  Wire.write(message);
  delay(100);
}

void Send_GSM(byte code) {
  sim808.sendSMS(Master_Phone, code); //Sending message to the master server
  delay(1000);
}

void Full_Forward() {
  //Setting the speed of Base motors
  digitalWrite(BM_D1, BM_Motor_Direction);
  digitalWrite(BM_D2, BM_Motor_Direction);
  digitalWrite(BM_D3, BM_Motor_Direction);
  digitalWrite(BM_D4, BM_Motor_Direction);
  digitalWrite(BM_D5, BM_Motor_Direction);
  digitalWrite(BM_D6, BM_Motor_Direction);
  digitalWrite(BM_D7, BM_Motor_Direction);
  digitalWrite(BM_D8, BM_Motor_Direction);
  analogWrite(BM_P1, BM_Full_Speed);
  analogWrite(BM_P2, BM_Full_Speed);
  analogWrite(BM_P3, BM_Full_Speed);
  analogWrite(BM_P4, BM_Full_Speed);
  analogWrite(BM_P5, BM_Full_Speed);
  analogWrite(BM_P6, BM_Full_Speed);
  analogWrite(BM_P7, BM_Full_Speed);
  analogWrite(BM_P8, BM_Full_Speed);
  Respond(0x01);
}

void Half_Forward() {
  //Setting the speed of Base motors
  digitalWrite(BM_D1, BM_Motor_Direction);
  digitalWrite(BM_D2, BM_Motor_Direction);
  digitalWrite(BM_D3, BM_Motor_Direction);
  digitalWrite(BM_D4, BM_Motor_Direction);
  digitalWrite(BM_D5, BM_Motor_Direction);
  digitalWrite(BM_D6, BM_Motor_Direction);
  digitalWrite(BM_D7, BM_Motor_Direction);
  digitalWrite(BM_D8, BM_Motor_Direction);
  analogWrite(BM_P1, BM_Half_Speed);
  analogWrite(BM_P2, BM_Half_Speed);
  analogWrite(BM_P3, BM_Half_Speed);
  analogWrite(BM_P4, BM_Half_Speed);
  analogWrite(BM_P5, BM_Half_Speed);
  analogWrite(BM_P6, BM_Half_Speed);
  analogWrite(BM_P7, BM_Half_Speed);
  analogWrite(BM_P8, BM_Half_Speed);
  Respond(0x01);
}

void Full_Backward() {
  //Setting the speed of Base motors
  digitalWrite(BM_D1, !BM_Motor_Direction);
  digitalWrite(BM_D2, !BM_Motor_Direction);
  digitalWrite(BM_D3, !BM_Motor_Direction);
  digitalWrite(BM_D4, !BM_Motor_Direction);
  digitalWrite(BM_D5, !BM_Motor_Direction);
  digitalWrite(BM_D6, !BM_Motor_Direction);
  digitalWrite(BM_D7, !BM_Motor_Direction);
  digitalWrite(BM_D8, !BM_Motor_Direction);
  analogWrite(BM_P1, BM_Full_Speed);
  analogWrite(BM_P2, BM_Full_Speed);
  analogWrite(BM_P3, BM_Full_Speed);
  analogWrite(BM_P4, BM_Full_Speed);
  analogWrite(BM_P5, BM_Full_Speed);
  analogWrite(BM_P6, BM_Full_Speed);
  analogWrite(BM_P7, BM_Full_Speed);
  analogWrite(BM_P8, BM_Full_Speed);
  Respond(0x01);
}

void Half_Backward() {
  //Setting the speed of Base motors
  digitalWrite(BM_D1, !BM_Motor_Direction);
  digitalWrite(BM_D2, !BM_Motor_Direction);
  digitalWrite(BM_D3, !BM_Motor_Direction);
  digitalWrite(BM_D4, !BM_Motor_Direction);
  digitalWrite(BM_D5, !BM_Motor_Direction);
  digitalWrite(BM_D6, !BM_Motor_Direction);
  digitalWrite(BM_D7, !BM_Motor_Direction);
  digitalWrite(BM_D8, !BM_Motor_Direction);
  analogWrite(BM_P1, BM_Half_Speed);
  analogWrite(BM_P2, BM_Half_Speed);
  analogWrite(BM_P3, BM_Half_Speed);
  analogWrite(BM_P4, BM_Half_Speed);
  analogWrite(BM_P5, BM_Half_Speed);
  analogWrite(BM_P6, BM_Half_Speed);
  analogWrite(BM_P7, BM_Half_Speed);
  analogWrite(BM_P8, BM_Half_Speed);
  Respond(0x01);
}

void Left_Full_Forward() {
  //Setting the speed of Base motors
  digitalWrite(BM_D1, BM_Motor_Direction);
  digitalWrite(BM_D2, BM_Motor_Direction);
  digitalWrite(BM_D3, BM_Motor_Direction);
  digitalWrite(BM_D4, BM_Motor_Direction);
  digitalWrite(BM_D5, !BM_Motor_Direction);
  digitalWrite(BM_D6, !BM_Motor_Direction);
  digitalWrite(BM_D7, !BM_Motor_Direction);
  digitalWrite(BM_D8, !BM_Motor_Direction);
  analogWrite(BM_P1, BM_Full_Speed);
  analogWrite(BM_P2, BM_Full_Speed);
  analogWrite(BM_P3, BM_Full_Speed);
  analogWrite(BM_P4, BM_Full_Speed);
  analogWrite(BM_P5, BM_Full_Speed);
  analogWrite(BM_P6, BM_Full_Speed);
  analogWrite(BM_P7, BM_Full_Speed);
  analogWrite(BM_P8, BM_Full_Speed);
  Respond(0x01);
}

void Right_Full_Forward() {
  //Setting the speed of Base motors
  digitalWrite(BM_D1, !BM_Motor_Direction);
  digitalWrite(BM_D2, !BM_Motor_Direction);
  digitalWrite(BM_D3, !BM_Motor_Direction);
  digitalWrite(BM_D4, !BM_Motor_Direction);
  digitalWrite(BM_D5, BM_Motor_Direction);
  digitalWrite(BM_D6, BM_Motor_Direction);
  digitalWrite(BM_D7, BM_Motor_Direction);
  digitalWrite(BM_D8, BM_Motor_Direction);
  analogWrite(BM_P1, BM_Full_Speed);
  analogWrite(BM_P2, BM_Full_Speed);
  analogWrite(BM_P3, BM_Full_Speed);
  analogWrite(BM_P4, BM_Full_Speed);
  analogWrite(BM_P5, BM_Full_Speed);
  analogWrite(BM_P6, BM_Full_Speed);
  analogWrite(BM_P7, BM_Full_Speed);
  analogWrite(BM_P8, BM_Full_Speed);
  Respond(0x01);
}

void Left_Half_Forward() {
  //Setting the speed of Base motors
  digitalWrite(BM_D1, BM_Motor_Direction);
  digitalWrite(BM_D2, BM_Motor_Direction);
  digitalWrite(BM_D3, BM_Motor_Direction);
  digitalWrite(BM_D4, BM_Motor_Direction);
  digitalWrite(BM_D5, !BM_Motor_Direction);
  digitalWrite(BM_D6, !BM_Motor_Direction);
  digitalWrite(BM_D7, !BM_Motor_Direction);
  digitalWrite(BM_D8, !BM_Motor_Direction);
  analogWrite(BM_P1, BM_Half_Speed);
  analogWrite(BM_P2, BM_Half_Speed);
  analogWrite(BM_P3, BM_Half_Speed);
  analogWrite(BM_P4, BM_Half_Speed);
  analogWrite(BM_P5, BM_Half_Speed);
  analogWrite(BM_P6, BM_Half_Speed);
  analogWrite(BM_P7, BM_Half_Speed);
  analogWrite(BM_P8, BM_Half_Speed);
  Respond(0x01);
}

void Right_Half_Forward() {
  //Setting the speed of Base motors
  digitalWrite(BM_D1, !BM_Motor_Direction);
  digitalWrite(BM_D2, !BM_Motor_Direction);
  digitalWrite(BM_D3, !BM_Motor_Direction);
  digitalWrite(BM_D4, !BM_Motor_Direction);
  digitalWrite(BM_D5, BM_Motor_Direction);
  digitalWrite(BM_D6, BM_Motor_Direction);
  digitalWrite(BM_D7, BM_Motor_Direction);
  digitalWrite(BM_D8, BM_Motor_Direction);
  analogWrite(BM_P1, BM_Half_Speed);
  analogWrite(BM_P2, BM_Half_Speed);
  analogWrite(BM_P3, BM_Half_Speed);
  analogWrite(BM_P4, BM_Half_Speed);
  analogWrite(BM_P5, BM_Half_Speed);
  analogWrite(BM_P6, BM_Half_Speed);
  analogWrite(BM_P7, BM_Half_Speed);
  analogWrite(BM_P8, BM_Half_Speed);
  Respond(0x01);
}

void Cutting_ON() {
  //Setting the speed of Cutter motor
  Cutting_Motor_ON = HIGH;
}

void CM_ON() {
  if (CM_Error_1_OFF == LOW && sensor.getCurrent() >= 190) { //Check whether the current is above 190A
    CM_Error_1_Count++;
    if (CM_Error_1_Count > 7) { //Driection is changed only when the current is above 190A for more than 7 cycles
      CM_Error_1_Direction = !CM_Error_1_Direction;  //Change Direction of Cutting motor
      CM_Error_1_delay = HIGH;  //Add delay once when the direction is changed
      CM_Error_1 = HIGH;
      CM_Error_1_Count = 0;
    }
  }
  if (sensor.getCurrent() < 190) { //Count zero if false/temporary increment
    CM_Error_1_Count = 0;
  }

  if (CM_Error_1_Direction == LOW) { //CM_Error_1 = LOW then forward direction
    if (CM_Error_1_delay == HIGH) {
      Stop_Cutting();
      delay(2000);
      CM_Error_1_delay = LOW;
    }
    CM.writeMicroseconds(CM_Full_Forward_Speed);  //Cutter motor in forward direction
    if (CM_Error_1 == HIGH & sensor.getCurrent() >= 190) { //Check if the current is still above 190A
      CM_Error_1_Forward_Count++;
      CM_Error_1_OFF = HIGH;
      if (CM_Error_1_Forward_Count > 7) { //Pivot mechanism will only be triggered when the current is above 190A for more than 7 cycles
        CM_Error_1_Forward_Count = 0;
        CM_Error_2 = HIGH;
        if (CM_Error_2_OFF == LOW) {
          T_M_Trigger = HIGH;
        }
      }
    }
    if (CM_Error_1 == HIGH && sensor.getCurrent() < 190) { //If the error has solved then deactivate the Error 1
      CM_Error_1_OFF_Count++;
      if (CM_Error_1_OFF_Count > 5) {
        CM_Error_1 = LOW;
        CM_Error_1_OFF_Count = 0;
        CM_Error_1_OFF = LOW;
        CM_Error_2_OFF = LOW;
      }
    }
  }

  if (CM_Error_1_Direction == HIGH) { //CM_Error_1 = HIGH then reverse direction
    if (CM_Error_1_delay == HIGH) {
      Stop_Cutting();
      delay(2000);
      CM_Error_1_delay = LOW;
    }
    CM.writeMicroseconds(CM_Full_Reverse_Speed);
    if (CM_Error_1 == HIGH & sensor.getCurrent() >= 190) { //Check if the current is still above 190A
      CM_Error_1_Reverse_Count++;
      CM_Error_1_OFF = HIGH;
      if (CM_Error_1_Reverse_Count > 5) { //Pivot mechanism will only be triggered when the current is above 190A for more than 5 cycles
        CM_Error_1_Reverse_Count = 0;
        CM_Error_2 = HIGH;
        if (CM_Error_2_OFF == LOW) {
          T_M_Trigger = HIGH;
        }
      }
    }
    if (CM_Error_1 == HIGH && sensor.getCurrent() < 190) { //If the error has solved then deactivate the Error 1
      CM_Error_1_OFF_Count++;
      if (CM_Error_1_OFF_Count > 5) {
        CM_Error_1 = LOW;
        CM_Error_1_OFF = LOW;
        CM_Error_1_OFF_Count = 0;
        CM_Error_2_OFF = LOW;
      }
    }
  }
  if (CM_Error_2 == HIGH) { //If error 2 is raised then change the state of pivot mechanism
    if (T_M_Trigger == HIGH) {
      T_M_State = !T_M_State;
      digitalWrite(T_M, T_M_State);
      T_M_Trigger_Count++;
      T_M_Trigger = LOW;
    }
    if (T_M_Trigger_Count > 2) { //If error 2 is still there then send message to master server
      CM_Error_2_OFF = HIGH;
      T_M_Trigger = LOW;
      if (sensor.getCurrent() > 190) {
        T_M_Count++;
        if (T_M_Count > 7) {
          Send_Error(0x2D); //Cutter motor stuck, sending error
          T_M_Count = 0;
          T_M_Trigger_Count = 0;
        }
      }
    }
    if (sensor.getCurrent() < 190) { //If error 2 is resolved
      T_M_Count_Low++;
      if (T_M_Count_Low > 7) {
        T_M_Count_Low = 0;
        CM_Error_2 = LOW;
        T_M_Trigger_Count = 0;
        CM_Error_2_OFF = LOW;
      }
    }
  }
}

void Stop_Cutting() {
  //Stopping the Cutter motor
  Cutting_Motor_ON = LOW;
  CM.writeMicroseconds(CM_Stop);
  Respond(0x01);
}

void Stop_Motion() { //Stopping all the base motors
  //Stopping/Breaking the Base motors
  analogWrite(BM_P1, Motor_Stop);
  analogWrite(BM_P2, Motor_Stop);
  analogWrite(BM_P3, Motor_Stop);
  analogWrite(BM_P4, Motor_Stop);
  analogWrite(BM_P5, Motor_Stop);
  analogWrite(BM_P6, Motor_Stop);
  analogWrite(BM_P7, Motor_Stop);
  analogWrite(BM_P8, Motor_Stop);
  Respond(0x01);
}

void Stop_Everything() { //Stop every motor on the robot
  //Stopping/Breaking the Base motors
  analogWrite(BM_P1, Motor_Stop);
  analogWrite(BM_P2, Motor_Stop);
  analogWrite(BM_P3, Motor_Stop);
  analogWrite(BM_P4, Motor_Stop);
  analogWrite(BM_P5, Motor_Stop);
  analogWrite(BM_P6, Motor_Stop);
  analogWrite(BM_P7, Motor_Stop);
  analogWrite(BM_P8, Motor_Stop);

  //Cutter motor Stop
  CM.writeMicroseconds(CM_Stop);

  //Compressor OFF
  digitalWrite(CR_1, LOW);
  digitalWrite(CR_2, LOW);

  //Scissor mechanism stop
  analogWrite(SMM_P1, Motor_Stop);

  //Linear Cutter mechanism stop
  analogWrite(L_CMM_P1, Motor_Stop);

  //Cutter mechanism stop
  analogWrite(CMM_P2, Motor_Stop);

}

void Send_Error(byte Error) { //Sending error to the master server after the robot is stuck somewhere with the coordinates

  //Sending coordinates to master server
  Send_GPS();
  //Send Error via GSM
  Send_GSM(Error);
}

void Right_Compressor_ON() {
  //Turning on the Right compressor
  digitalWrite(CR_1, HIGH);
  Respond(0x01);
}

void Left_Compressor_ON() {
  //Turning on the Right compressor
  digitalWrite(CR_2, HIGH);
  Respond(0x01);
}

void Right_Compressor_OFF() {
  //Turning off the Right compressor
  digitalWrite(CR_1, LOW);
  Respond(0x01);
}

void Left_Compressor_OFF() {
  //Turning off the Right compressor
  digitalWrite(CR_2, LOW);
  Respond(0x01);
}

void Right_Support_Deploy() {
  //Turning on the Right Leg support solenoid valve
  digitalWrite(R_S, HIGH);
  Respond(0x01);
}

void Left_Support_Deploy() {
  //Turning on the Left Leg support solenoid valve
  digitalWrite(L_S, HIGH);
  Respond(0x01);
}

void Right_Support_Home() {
  //Turning off the Right Leg support solenoid valve
  digitalWrite(R_S, LOW);
  Respond(0x01);
}

void Left_Support_Home() {
  //Turning off the Left Leg support solenoid valve
  digitalWrite(L_S, LOW);
  Respond(0x01);
}

void Right_Extension_Deploy() {
  //Turning on the Right Leg Extension solenoid valve
  digitalWrite(R_E, HIGH);
  Respond(0x01);
}

void Left_Extension_Deploy() {
  //Turning on the Left Leg Extension solenoid valve
  digitalWrite(L_E, HIGH);
  Respond(0x01);
}

void Right_Extension_Home() {
  //Turning off the Right Leg Extension solenoid valve
  digitalWrite(R_E, LOW);
  bool period = 1;  //For assigining last time once
  R_E_Error = LOW;
  while (digitalRead(S_D5) == LOW) {
    unsigned long current_time = millis();
    unsigned long last_time;
    if (period == 1) {
      last_time = current_time;
      period = 0;
    }
    if (current_time - last_time > R_E_Error_Timeout && ROBOT_MODE == LOW) { //Wait and try to return to home position, if not then trigger error and break the loop.
      R_E_Error = HIGH;
      Send_Error(0x2C); //Sending error when right extension is stuck
      break;
    }
  }
  if (R_E_Error == LOW) {
    Respond(0x01);
  }
}

void Left_Extension_Home() {
  //Turning off the Left Leg Extension solenoid vavle
  digitalWrite(L_E, LOW);
  bool period = 1;  //For assigining last time once
  L_E_Error = LOW;
  while (digitalRead(S_D6) == LOW) {
    unsigned long current_time = millis();
    unsigned long last_time;
    if (period == 1) {
      last_time = current_time;
      period = 0;
    }
    if (current_time - last_time > L_E_Error_Timeout && ROBOT_MODE == LOW) { //Wait and try to return to home position, if not then trigger error and break the loop.
      L_E_Error = HIGH;
      Send_Error(0x2B); //Sending error when left extension is stuck
      break;
    }
  }
  if (L_E_Error == LOW) {
    Respond(0x01);
  }
}

void Scissor_Mech_UP() {
  //Setting Scissor mechanism speed
  digitalWrite(SMM_D1, SMM_Direction);
  analogWrite(SMM_P1, SMM_Speed);
  Respond(0x01);
}

void Scissor_Mech_DOWN() {
  if (digitalRead(S_D1) == LOW) {
    //Setting Scissor mechanism speed
    digitalWrite(SMM_D1, !SMM_Direction);
    analogWrite(SMM_P1, SMM_Speed);
    Respond(0x01);
  }
  else {
    //Stopping/Breaking the SMM if the sensor is already triggered
    Scissor_Mech_Stop();
  }
}

void Scissor_Mech_Home() {
  bool period = 1;  //For assigining last time once
  SMM_Error = LOW;
  while (digitalRead(S_D1) == LOW) {
    //Setting Scissor mechanism speed
    digitalWrite(SMM_D1, !SMM_Direction);
    analogWrite(SMM_P1, SMM_Speed);
    unsigned long current_time = millis();
    unsigned long last_time;
    if (period == 1) {
      last_time = current_time;
      period = 0;
    }
    if (current_time - last_time > SMM_Error_Timeout && ROBOT_MODE == LOW) { //Wait and try to return to home position, if not then stop the motor and break the loop.
      analogWrite(SMM_P1, Motor_Stop);
      SMM_Error = HIGH;
      Send_Error(0x2A); //Sending error when Scissor mechanism is stuck
      break;
    }
  }
  if (SMM_Error == LOW) {
    Respond(0x01);
  }
}

void Scissor_Mech_Stop() {
  //Stopping/Breaking the SMM
  analogWrite(SMM_P1, Motor_Stop);
  Respond(0x01);
}

void Linear_Cutter_Mech_Forward() {
  //Setting Linear Cutter mechanism speed
  digitalWrite(L_CMM_D1, L_CMM_Direction);
  analogWrite(L_CMM_P1, L_CMM_Speed);
  Respond(0x01);
}

void Linear_Cutter_Mech_Backward() {
  //Setting Linear Cutter mechanism speed
  digitalWrite(L_CMM_D1, !L_CMM_Direction);
  analogWrite(L_CMM_P1, L_CMM_Speed);
  Respond(0x01);
}

void Linear_Cutter_Mech_Home() {
  bool period = 1;
  L_CMM_Error = LOW;
  while (digitalRead(S_D3) == LOW) {
    //Setting Scissor mechanism speed
    digitalWrite(L_CMM_D1, !L_CMM_Direction);
    analogWrite(L_CMM_P1, L_CMM_Speed);
    unsigned long current_time = millis();
    unsigned long last_time;
    if (period == 1) {
      last_time = current_time;
      period = 0;
    }
    if (current_time - last_time > L_CMMM_Error_Timeout && ROBOT_MODE == LOW) { //Wait and try to return to home position, if not then stop the motor and break the loop.
      analogWrite(L_CMM_P1, Motor_Stop);
      L_CMM_Error = HIGH;
      Send_Error(0x29); //Sending error when linear cutter motor mechanism is stuck
      break;
    }
  }
  if (L_CMM_Error == LOW) {
    Respond(0x01);
  }
}

void Linear_Cutter_Mech_Stop() {
  //Stopping/Breaking the Linear Cutter Mechanism Motor
  analogWrite(L_CMM_P1, Motor_Stop);
  Respond(0x01);
}

void Cutter_Mech_UP() {
  //Setting Cutter mechanism speed
  digitalWrite(CMM_D2, CMM_Direction);
  analogWrite(CMM_P2, CMM_Speed);
  Respond(0x01);
}

void Cutter_Mech_DOWN() {
  //Setting Cutter mechanism speed
  digitalWrite(CMM_D2, !CMM_Direction);
  analogWrite(CMM_P2, CMM_Speed);
  Respond(0x01);
}

void Cutter_Mech_Stop() {
  //Stopping/Breaking the Cutter Mechanism Motor
  analogWrite(CMM_P2, Motor_Stop);
  Respond(0x01);
}

void Read_Battery_Voltage() {
  while (Sample_Count_Voltage < Sample_Number_Voltage) {
    Voltage_Sum += analogRead(V_Read);
    Sample_Count_Voltage++;
    delay(10);
  }
  Voltage = (Voltage_Sum / Sample_Number_Voltage * 5.015) / 1024.0;
  Voltage = Voltage * Voltage_Reference;
  Sample_Count_Voltage = 0;
  Voltage_Sum = 0;
  if (Debug == HIGH) {
    Serial.print(Voltage);
    Serial.println (" V");
  }
  if (Voltage < Low_Voltage_Trigger) { //Signals when battery voltage is low
    Low_Voltage = HIGH;
    Respond(0x02);
  }
  if (Voltage >= Full_Battery_Trigger) { //Signals when battery is fully charged
    Low_Voltage = LOW;
    Respond(0x03);
  }
}

void Send_GPS() {
  //Turning ON the GPS
  if (sim808.attachGPS()) {
    if (Debug == HIGH) {
      Serial.println("GPS power open success");
    }
  }
  else {
    if (Debug == HIGH) {
      Serial.println("GPS power open failure");
      Send_GSM(0x28);
    }
  } //Sending GPS Error to the master server

  delay(500);

  while (!sim808.getGPS())
  {
    if (Debug == HIGH) {
      Serial.println("GPS Error, Please check the GPS module...");
    }
  }
  if (Debug == HIGH) {
    Serial.println("Sending GPS Coordinates to master server");
  }
  float la = sim808.GPSdata.lat;  //Getting latitude data
  float lo = sim808.GPSdata.lon;  //Getting longitude data
  dtostrf(la, 6, 2, Latitude); //put float value of la into char array of Latitude. 6 = number of digits before decimal sign. 2 = number of digits after the decimal sign.
  dtostrf(lo, 6, 2, Longitude); //put float value of lo into char array of Longitude
  sprintf(Send_Message, "Latitude: %s\nLongitude: %s", Latitude, Longitude); //Getting the message ready
  sim808.sendSMS(Master_Phone, Send_Message); //Sending message to the master server
  //Turning OFF the GPS
  sim808.detachGPS();
}

void Read_PS2(){
  ps2x.read_gamepad(false, vibrate);
  if(Remote_Init == HIGH){
    delay(500);
    vibrate = 0;
    Remote_Init = LOW;
   }

  if(ps2x.ButtonPressed(PSB_START)){ //Remote communication will only be activated if START button is pressed for more than 5 seconds
    current_time = millis();
    last_time = current_time;
    while(ps2x.Button(PSB_START)){
      ps2x.read_gamepad(false, vibrate);
      current_time = millis();
      if(ps2x.ButtonReleased(PSB_START)){
        Serial.println("Activation/Deactivation Cancelled");
        break;
      }
      if(current_time - last_time > 5000 && Remote_Activated == LOW){
        vibrate = 255;
        ps2x.read_gamepad(false, vibrate);
        delay(1000);
        vibrate = 0;
        ps2x.read_gamepad(false, vibrate);
        Serial.println("Remote mode activated");
        Remote_Activated = HIGH;
        break;
      }
      if(current_time - last_time > 5000 && Remote_Activated == HIGH){
        vibrate = 255;
        ps2x.read_gamepad(false, vibrate);
        delay(1000);
        vibrate = 0;
        ps2x.read_gamepad(false, vibrate);
        Serial.println("Remote mode Deactivated");
        Remote_Activated = LOW;
        break;
      }
    }
  }
  
  if(Remote_Activated == HIGH){
  if(ps2x.ButtonPressed(PSB_BLUE)){
    Blue_State = !Blue_State;
    if(Blue_State == HIGH){
      if (Debug == HIGH) {
        Serial.println("Cutter motor ON");}
      CM.writeMicroseconds(CM_Full_Forward_Speed);  //Cutter motor in forward direction
     }
    if(Blue_State == LOW){
      if (Debug == HIGH) {
        Serial.println("Cutter motor OFF");}
      CM.writeMicroseconds(CM_Stop);  //Stop Cutter motor
    }
  }
  
  if(ps2x.ButtonPressed(PSB_RED)){
    Red_State = !Red_State;
    if(Red_State == HIGH){
      digitalWrite(R_E, HIGH); //Extensions deployed
      digitalWrite(L_E, HIGH);
      int k=0;
      while(k < Remote_Extension_Wait_Time){ //Wait for some seconds
        delay(1000);
        k++;
        ps2x.read_gamepad(false,vibrate); 
        }
      digitalWrite(R_S, HIGH);  //Supports deployed
      digitalWrite(L_S, HIGH);
      if (Debug == HIGH) {
      Serial.println("Support Deployed");}
      }
    if(Red_State == LOW){
      digitalWrite(R_S, LOW);  //Supports Home
      digitalWrite(L_S, LOW);
      int k=0;
      while(k < Remote_Extension_Wait_Time){  //Wait for some seconds
        delay(1000);
        k++;
        ps2x.read_gamepad(false,vibrate); 
        }
      digitalWrite(R_E, LOW);  //Extensions Home
      digitalWrite(L_E, LOW);
      if (Debug == HIGH) {
      Serial.println("Support Home");}
      }
  }
  
  if(ps2x.ButtonPressed(PSB_GREEN)){
    Green_State = !Green_State;
    if(Green_State == HIGH){
      digitalWrite(LED, HIGH); //Robot lights ON
      if (Debug == HIGH) {
      Serial.println("Lights ON");}}
    if(Green_State == LOW){
      digitalWrite(LED, LOW);  //Robot lights OFF
      if (Debug == HIGH) {
      Serial.println("Lights OFF");}}
  }
  
  if(ps2x.ButtonPressed(PSB_L1)){  //Cutter mechnism UP when L1 Pressed
    digitalWrite(CMM_D2, CMM_Direction);
    analogWrite(CMM_P2, CMM_Speed_Remote);
    if (Debug == HIGH) {
    Serial.println("Cutter mechanism UP");}  
  }
  if(ps2x.ButtonReleased(PSB_L1) || ps2x.ButtonReleased(PSB_L2)){ //Cutter mechnism STOP when L1 or L2 Released
    analogWrite(CMM_P2, Motor_Stop);
    if (Debug == HIGH) {
    Serial.println("Cutter mechanism STOP");}  
  }
  if(ps2x.Button(PSB_L2)){  //Cutter mechnism DOWN when L2 Pressed
    digitalWrite(CMM_D2, !CMM_Direction);
    analogWrite(CMM_P2, CMM_Speed_Remote);
    if (Debug == HIGH) {
    Serial.println("Cutter mechanism DOWN");} 
  }
  
  if(ps2x.ButtonPressed(PSB_PAD_UP)) {  //Scissor mechnism UP when PAD_UP Pressed
    digitalWrite(SMM_D1, SMM_Direction);
    analogWrite(SMM_P1, SMM_Speed_Remote);
    if (Debug == HIGH) {    
    Serial.println("Scissor mechanism UP");}
  }
  if(ps2x.ButtonReleased(PSB_PAD_UP) || ps2x.ButtonReleased(PSB_PAD_DOWN)) {  //Scissor mechnism UP when PAD_UP or PAD_DOWN Released
    analogWrite(SMM_P1, Motor_Stop);
    if (Debug == HIGH) {    
    Serial.println("Scissor mechanism STOP");}
  }
  if(ps2x.ButtonPressed(PSB_PAD_DOWN)) {  //Scissor mechnism DOWN when PAD_DOWN Pressed
    if(digitalRead(S_D1) == LOW){
    digitalWrite(SMM_D1, !SMM_Direction);
    analogWrite(SMM_P1, SMM_Speed_Remote);}
    else{
      analogWrite(SMM_P1, Motor_Stop);
    }
    if (Debug == HIGH) {     
    Serial.println("Scissor mechanism DOWN");}
  }
  
  if(ps2x.ButtonPressed(PSB_PAD_RIGHT)) {  //Linear cutter mechnism forward when PAD_RIGHT Pressed
    digitalWrite(L_CMM_D1, L_CMM_Direction);
    analogWrite(L_CMM_P1, L_CMM_Speed_Remote);
    if (Debug == HIGH) {     
    Serial.println("Linear cutter mechanism forward");}
  }
  if(ps2x.ButtonReleased(PSB_PAD_RIGHT) || ps2x.ButtonReleased(PSB_PAD_LEFT)) {  //Linear cutter mechnism STOP when PAD_RIGHT or PAD_LEFT Released
    analogWrite(L_CMM_P1, Motor_Stop);
    if (Debug == HIGH) {     
    Serial.println("Linear cutter mechanism STOP");}
  }
  if(ps2x.ButtonPressed(PSB_PAD_LEFT)) {  //Linear cutter mechnism backward when PAD_LEFT Pressed
    if(digitalRead(S_D3) == LOW){
    digitalWrite(L_CMM_D1, !L_CMM_Direction);
    analogWrite(L_CMM_P1, L_CMM_Speed_Remote);}
    else{
      analogWrite(L_CMM_P1, Motor_Stop);}
    if (Debug == HIGH) {  
    Serial.println("Linear cutter mechanism backward");}
  }
  
  if(ps2x.ButtonPressed(PSB_SELECT)){  // To stop every motor on the robot
    Stop_Everything();
    if (Debug == HIGH) {
    Serial.println("Emergency STOP!");}
  }

  if(ps2x.Analog(PSS_LY) > 130){  //Robot forward movement
    int val = map(ps2x.Analog(PSS_LY), 127, 255, 0, 255);
    if (Debug == HIGH) {
      Serial.print("Robot Forward Speed: ");
      Serial.println(val);}
    digitalWrite(BM_D1, BM_Motor_Direction);
    digitalWrite(BM_D2, BM_Motor_Direction);
    digitalWrite(BM_D3, BM_Motor_Direction);
    digitalWrite(BM_D4, BM_Motor_Direction);
    digitalWrite(BM_D5, BM_Motor_Direction);
    digitalWrite(BM_D6, BM_Motor_Direction);
    digitalWrite(BM_D7, BM_Motor_Direction);
    digitalWrite(BM_D8, BM_Motor_Direction);
    analogWrite(BM_P1, val);
    analogWrite(BM_P2, val);
    analogWrite(BM_P3, val);
    analogWrite(BM_P4, val);
    analogWrite(BM_P5, val);
    analogWrite(BM_P6, val);
    analogWrite(BM_P7, val);
    analogWrite(BM_P8, val);
  }
  if(ps2x.Analog(PSS_LY) < 124){ //Robot backwards movement
    int val1 = map(ps2x.Analog(PSS_LY), 127, 0, 0, 255);
    if (Debug == HIGH) {
    Serial.print("Robot Backward Speed: ");
    Serial.println(val1);}
    digitalWrite(BM_D1, !BM_Motor_Direction);
    digitalWrite(BM_D2, !BM_Motor_Direction);
    digitalWrite(BM_D3, !BM_Motor_Direction);
    digitalWrite(BM_D4, !BM_Motor_Direction);
    digitalWrite(BM_D5, !BM_Motor_Direction);
    digitalWrite(BM_D6, !BM_Motor_Direction);
    digitalWrite(BM_D7, !BM_Motor_Direction);
    digitalWrite(BM_D8, !BM_Motor_Direction);
    analogWrite(BM_P1, val1);
    analogWrite(BM_P2, val1);
    analogWrite(BM_P3, val1);
    analogWrite(BM_P4, val1);
    analogWrite(BM_P5, val1);
    analogWrite(BM_P6, val1);
    analogWrite(BM_P7, val1);
    analogWrite(BM_P8, val1);
  }
  if(ps2x.Analog(PSS_LY)>124 && ps2x.Analog(PSS_LY)<130 && ps2x.Analog(PSS_LX)>124 && ps2x.Analog(PSS_LX)<130){  //Robot Stop if the joystick is in default position
    analogWrite(BM_P1, Motor_Stop);
    analogWrite(BM_P2, Motor_Stop);
    analogWrite(BM_P3, Motor_Stop);
    analogWrite(BM_P4, Motor_Stop);
    analogWrite(BM_P5, Motor_Stop);
    analogWrite(BM_P6, Motor_Stop);
    analogWrite(BM_P7, Motor_Stop);
    analogWrite(BM_P8, Motor_Stop);
  }

  if(ps2x.Analog(PSS_LX) > 130){  //Robot Right movement
    int val = map(ps2x.Analog(PSS_LX), 127, 255, 0, 255);
    if (Debug == HIGH) {
      Serial.print("Robot Forward Speed: ");
      Serial.println(val);}
    digitalWrite(BM_D1, !BM_Motor_Direction);
    digitalWrite(BM_D2, !BM_Motor_Direction);
    digitalWrite(BM_D3, !BM_Motor_Direction);
    digitalWrite(BM_D4, !BM_Motor_Direction);
    digitalWrite(BM_D5, BM_Motor_Direction);
    digitalWrite(BM_D6, BM_Motor_Direction);
    digitalWrite(BM_D7, BM_Motor_Direction);
    digitalWrite(BM_D8, BM_Motor_Direction);
    analogWrite(BM_P1, val);
    analogWrite(BM_P2, val);
    analogWrite(BM_P3, val);
    analogWrite(BM_P4, val);
    analogWrite(BM_P5, val);
    analogWrite(BM_P6, val);
    analogWrite(BM_P7, val);
    analogWrite(BM_P8, val);
  }
  if(ps2x.Analog(PSS_LX) < 124){ //Robot left movement
    int val1 = map(ps2x.Analog(PSS_LX), 127, 0, 0, 255);
    if (Debug == HIGH) {
    Serial.print("Robot Left Speed: ");
    Serial.println(val1);}
    digitalWrite(BM_D1, BM_Motor_Direction);
    digitalWrite(BM_D2, BM_Motor_Direction);
    digitalWrite(BM_D3, BM_Motor_Direction);
    digitalWrite(BM_D4, BM_Motor_Direction);
    digitalWrite(BM_D5, !BM_Motor_Direction);
    digitalWrite(BM_D6, !BM_Motor_Direction);
    digitalWrite(BM_D7, !BM_Motor_Direction);
    digitalWrite(BM_D8, !BM_Motor_Direction);
    analogWrite(BM_P1, val1);
    analogWrite(BM_P2, val1);
    analogWrite(BM_P3, val1);
    analogWrite(BM_P4, val1);
    analogWrite(BM_P5, val1);
    analogWrite(BM_P6, val1);
    analogWrite(BM_P7, val1);
    analogWrite(BM_P8, val1);
  }
 }
}
