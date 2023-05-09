#include <Filters.h>                            //Library to use
#include<Wire.h>

float testFrequency = 50;                     // test signal frequency (Hz)
float windowLength = 100/testFrequency;       // how long to average the signal, for statistist, changing this can have drastic effect
int RawValueR = 0;
int RawValueY = 0;
int RawValueB = 0;    
float R_Volts;
float Y_Volts;
float B_Volts;
float interceptR = -2.9;  // to be adjusted based on calibration testin
float slopeR = 1.080; 
float interceptY = -2.9;  // to be adjusted based on calibration testin
float slopeY = 1.060;
float interceptB = -2.9;  // to be adjusted based on calibration testin
float slopeB = 1.030;

char R[8];
char Y[7];
char B[7];

unsigned long printPeriod = 1000; //Measuring frequency, every 1s, can be changed
unsigned long previousMillis = 0;

RunningStatistics inputStatsR; //This class collects the value so we can apply some functions
RunningStatistics inputStatsY;
RunningStatistics inputStatsB;

void setup() {
  Serial.begin(9600);    // start the serial port
  inputStatsR.setWindowSecs(windowLength);
  inputStatsY.setWindowSecs(windowLength);
  inputStatsB.setWindowSecs(windowLength);
  Wire.begin(2);
  Wire.onRequest(Request);
}

void loop() {
                 
     ReadVoltage();  //The only function I'm running, be careful when using with this kind of boards
}

float ReadVoltage(){
    RawValueR = analogRead(A0);  // read the analog in value:
    RawValueY = analogRead(A1);
    RawValueB = analogRead(A2);
    inputStatsR.input(RawValueR);       // log to Stats function
    inputStatsY.input(RawValueY);
    inputStatsB.input(RawValueB);
        
    if((unsigned long)(millis() - previousMillis) >= printPeriod) { //We calculate and display every 1s
      previousMillis = millis();   // update time
      
      R_Volts = inputStatsR.sigma()* slopeR + interceptR;
      Y_Volts = inputStatsY.sigma()* slopeY + interceptY;
      B_Volts = inputStatsB.sigma()* slopeB + interceptB;
      
//      Serial.print("VoltsR: ");
//      Serial.println(R_Volts);
//      Serial.print("VoltsY: ");
//      Serial.println(Y_Volts);
//      Serial.print("VoltsB: ");
//      Serial.println(B_Volts);
  }
}

void Request(){
  
  //Converting float value to char
  //The format (float, bytes, numbers of numbers after the decimal, char variable)
  dtostrf(R_Volts, sizeof(R_Volts), 2, R);
  delay(50);
  Wire.write(R);
  Wire.write(",");
  dtostrf(Y_Volts, sizeof(Y_Volts), 2, Y);
  delay(50);
  Wire.write(Y);
  Wire.write(",");
  dtostrf(B_Volts, sizeof(B_Volts), 2, B);
  delay(50);
  Wire.write(B);
  Wire.write("!");
}
