#include "HX711.h"
#include<AccelStepper.h>

//Defining machine variables
volatile int MACHINE_STATE = 0; //to control the machine, MACHINE_STATE = 0/1/2 (INIT/ON/STOP)
#define ON_BUTTON_PIN 20 //ON button pin
#define STOP_BUTTON_PIN 21 //OFF button pin

volatile int MACHINE_CURRENT_STATE = 0; 
/*  0 -> Starting state of the machine
    1 -> Platform origin to extend, Suction extend to origin, Suction origin to extend and Suction extend to origin
    2 -> Container O to E, UShape E to O, Start Joint roll motor, Glass tip O to E, MixPlate O to E
    3 -> Glass tip E to O, Container E to O, UShape O to E, Stop Joint roll motor
    4 -> Platform E to O, Suction O to E */

//Defining vibration/vaccum motor, valve and tower light pin numbers
#define Hopper_Vib_Motor 0
#define FastSlider_Vib_Motor1 1
#define FastSlider_Vib_Motor2 8
#define SlowSlider_Vib_Motor 9
#define Vaccum_pump 13
#define Vaccum_valve 10
#define Red_Towerlight 4
#define Green_Towerlight 5
#define Yellow_Towerlight 6
#define Buzzer_Towerlight 7

//Defining sensor variables
#define Platform_Origin 19
#define Platform_Extend 18
#define LoadCell_DATA 12
#define LoadCell_CLK 11
#define Paper_Sensor 2 //Capacitor sensor for joint paper roll finish indication
#define Weight_Plate_Home 50
#define Container_Home 51
#define Mix_Plate_Home 52
#define U_Shape_Home 53
#define Glass_Tip_Home A15
#define Suction_Home A14

//Defining servo motor variables
#define WeightPlate_M1_STEP 14
#define WeightPlate_M1_DIR 15
#define WeightPlate_M2_STEP 16
#define WeightPlate_M2_DIR 17
#define JointCutter_M1_STEP 22
#define JointCutter_M2_STEP 23
#define JointRoll_M1_STEP 24
#define JointRoll_M2_STEP 25
#define ClothRoll_M1_STEP 26
#define ClothRoll_M2_STEP 27
#define Grinding_M1_STEP 28
#define Grinding_M2_STEP 29
#define GlassTip_M1_STEP 30
#define GlassTip_M1_DIR 31
#define GlassTip_M2_STEP 32
#define GlassTip_M2_DIR 33
#define MixPlate_STEP 34
#define MixPlate_DIR 35
#define Cointainer_M1_STEP 36
#define Cointainer_M1_DIR 37
#define Cointainer_M2_STEP 38
#define Cointainer_M2_DIR 39
#define UShape_STEP 40
#define UShape_DIR 41
#define Suction_M1_STEP 42
#define Suction_M1_DIR 43
#define Suction_M2_STEP 44
#define Suction_M2_DIR 45
#define Platform_M1_STEP 46
#define Platform_M1_DIR 47
#define Platform_M2_STEP 48
#define Platform_M2_DIR 49

//Load cell initialization
HX711 scale;
float weight;
bool weight_state = 0;
float weight_set = 0.0;
float calibration_factor = 0; //Change this value after calibrating the load cell
#define LOADCELL_DOUT_PIN  12
#define LOADCELL_SCK_PIN  11

//Stepper motors initialization with no need for parallel operation
AccelStepper Joint_Cutter_M1(1, JointCutter_M1_STEP, false); //(Type of driver, STEP, DIR)
AccelStepper Joint_Cutter_M2(1, JointCutter_M2_STEP, false);
AccelStepper Grinding_M1(1, Grinding_M1_STEP, false);
AccelStepper Grinding_M2(1, Grinding_M2_STEP, false);
AccelStepper Cloth_M1(1, ClothRoll_M1_STEP, false);
AccelStepper Cloth_M2(1, ClothRoll_M2_STEP, false);
//Stepper motor initialization
AccelStepper WeightPlate_M1(1, WeightPlate_M1_STEP, WeightPlate_M1_DIR);
AccelStepper WeightPlate_M2(1, WeightPlate_M2_STEP, WeightPlate_M2_DIR);
AccelStepper JointRoll_M1(1, JointRoll_M1_STEP, false);
AccelStepper JointRoll_M2(1, JointRoll_M2_STEP, false);
AccelStepper GlassTip_M1(1, GlassTip_M1_STEP, GlassTip_M1_DIR);
AccelStepper GlassTip_M2(1, GlassTip_M2_STEP, GlassTip_M2_DIR);
AccelStepper MixPlate(1, MixPlate_STEP, MixPlate_DIR);
AccelStepper Cointainer_M1(1, Cointainer_M1_STEP, Cointainer_M1_DIR);
AccelStepper Cointainer_M2(1, Cointainer_M2_STEP, Cointainer_M2_DIR);
AccelStepper UShape(1, UShape_STEP, UShape_DIR);
AccelStepper Suction_M1(1, Suction_M1_STEP, Suction_M1_DIR);
AccelStepper Suction_M2(1, Suction_M2_STEP, Suction_M2_DIR);
AccelStepper Platform_M1(1, Platform_M1_STEP, Platform_M1_DIR);
AccelStepper Platform_M2(1, Platform_M2_STEP, Platform_M2_DIR);

volatile int phase = 0;
bool joint_roll_state = 0; //To turn ON/OFF the joint paper roll motor
long curmillis = 0;
long prevmillis = 0;
int init_UShape = 0;
bool joint_paper_finish = 0;

void setup(){
    //Defining all the inputs
    pinMode(Platform_Origin, INPUT);
    pinMode(Platform_Extend, INPUT);
    pinMode(Paper_Sensor, INPUT);
    pinMode(ON_BUTTON_PIN, INPUT);
    pinMode(STOP_BUTTON_PIN, INPUT);
    pinMode(Weight_Plate_Home, INPUT);
    pinMode(Container_Home, INPUT);
    pinMode(Mix_Plate_Home, INPUT);
    pinMode(U_Shape_Home, INPUT);
    pinMode(Glass_Tip_Home, INPUT);
    pinMode(Suction_Home, INPUT);

    //Defining all the outputs
    pinMode(Hopper_Vib_Motor,OUTPUT);
    pinMode(FastSlider_Vib_Motor1,OUTPUT);
    pinMode(FastSlider_Vib_Motor2,OUTPUT);
    pinMode(SlowSlider_Vib_Motor,OUTPUT);
    pinMode(Vaccum_pump,OUTPUT);
    pinMode(Vaccum_valve,OUTPUT);
    pinMode(Red_Towerlight,OUTPUT);
    pinMode(Green_Towerlight,OUTPUT);
    pinMode(Yellow_Towerlight,OUTPUT);
    pinMode(Buzzer_Towerlight,OUTPUT);
    pinMode(WeightPlate_M1_STEP, OUTPUT);
    pinMode(WeightPlate_M1_DIR, OUTPUT);
    pinMode(WeightPlate_M2_STEP, OUTPUT);
    pinMode(WeightPlate_M2_DIR, OUTPUT);
    pinMode(JointCutter_M1_STEP, OUTPUT);
    pinMode(JointCutter_M2_STEP, OUTPUT);
    pinMode(JointRoll_M1_STEP, OUTPUT);
    pinMode(JointRoll_M2_STEP, OUTPUT);
    pinMode(ClothRoll_M1_STEP, OUTPUT);
    pinMode(ClothRoll_M2_STEP, OUTPUT);
    pinMode(Grinding_M1_STEP, OUTPUT);
    pinMode(Grinding_M2_STEP, OUTPUT);
    pinMode(GlassTip_M1_STEP, OUTPUT);
    pinMode(GlassTip_M1_DIR, OUTPUT);
    pinMode(GlassTip_M2_STEP, OUTPUT);
    pinMode(GlassTip_M2_DIR, OUTPUT);
    pinMode(MixPlate_STEP, OUTPUT);
    pinMode(MixPlate_DIR, OUTPUT);
    pinMode(Cointainer_M1_STEP, OUTPUT);
    pinMode(Cointainer_M1_DIR, OUTPUT);
    pinMode(Cointainer_M2_STEP, OUTPUT);
    pinMode(Cointainer_M2_DIR, OUTPUT);
    pinMode(UShape_STEP, OUTPUT);
    pinMode(UShape_DIR, OUTPUT);
    pinMode(Suction_M1_STEP, OUTPUT);
    pinMode(Suction_M1_DIR, OUTPUT);
    pinMode(Suction_M2_STEP, OUTPUT);
    pinMode(Suction_M2_DIR, OUTPUT);
    pinMode(Platform_M1_STEP, OUTPUT);
    pinMode(Platform_M1_DIR, OUTPUT);
    pinMode(Platform_M2_STEP, OUTPUT);
    pinMode(Platform_M2_DIR, OUTPUT);

    //Defining all the interrupts
    attachInterrupt(digitalPinToInterrupt(ON_BUTTON_PIN), Machine_start_interrupt, RISING);
    attachInterrupt(digitalPinToInterrupt(STOP_BUTTON_PIN), Machine_stop_interrupt, RISING);
    attachInterrupt(digitalPinToInterrupt(Paper_Sensor), Joint_paper_interrupt, RISING);

    //Machine initialization

    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN); //Initiating load cell sensor
    scale.set_scale(calibration_factor);
    scale.tare(); //Taring the loadcell
    delay(1000);

    digitalWrite(Hopper_Vib_Motor, LOW); //Turning off all the vibration motors
    digitalWrite(FastSlider_Vib_Motor1, LOW);
    digitalWrite(FastSlider_Vib_Motor2, LOW);
    digitalWrite(SlowSlider_Vib_Motor, LOW);
    digitalWrite(Vaccum_pump, LOW); //Turning off vaccum pump
    digitalWrite(Vaccum_valve, LOW); //Turning off the solenoid valve
    digitalWrite(Red_Towerlight, LOW); //Turning off tower lights
    digitalWrite(Green_Towerlight, LOW);
    digitalWrite(Yellow_Towerlight, LOW);
    digitalWrite(Buzzer_Towerlight, LOW);

    //All Stepper motor maximum speed initialization (DO NOT CHANGE THIS)
    Joint_Cutter_M1.setMaxSpeed(4000);
    Joint_Cutter_M2.setMaxSpeed(4000);
    Grinding_M1.setMaxSpeed(4000);
    Grinding_M2.setMaxSpeed(4000);
    Cloth_M1.setMaxSpeed(4000);
    Cloth_M2.setMaxSpeed(4000);
    WeightPlate_M1.setMaxSpeed(100);
    WeightPlate_M2.setMaxSpeed(100);
    MixPlate.setMaxSpeed(300);
    Platform_M1.setMaxSpeed(4000);
    Platform_M2.setMaxSpeed(4000);
    Cointainer_M1.setMaxSpeed(2000);
    Cointainer_M2.setMaxSpeed(2000);
    UShape.setMaxSpeed(2000);
    GlassTip_M1.setMaxSpeed(2000);
    GlassTip_M2.setMaxSpeed(2000);
    Suction_M1.setMaxSpeed(2000);
    Suction_M2.setMaxSpeed(2000);
    JointRoll_M1.setMaxSpeed(4000);
    JointRoll_M2.setMaxSpeed(4000);

    //All stepper motor speed set
    Joint_Cutter_M1.setSpeed(4000);
    Joint_Cutter_M2.setSpeed(4000);
    Grinding_M1.setSpeed(3000);
    Grinding_M2.setSpeed(3000);
    Cloth_M1.setSpeed(2500);
    Cloth_M2.setSpeed(2500);

    normal_motors_off(); //Turning off all the motors with normal operations
    other_motor_off(); // Turning off all the motors with other operations
    turn_towerlight_on_off(1,0,0,0); //Turning on red tower light
    vib_motors_off(); //Turning off all the vibration motors

    Machine_Home_Position(); //Setting all the machine components to home position
    Set_motor_init_pos(); //Setting current position of the motors to 0

    while(MACHINE_STATE == 0 || MACHINE_STATE == 2){
        //Stay here till the on button is pressed
        turn_towerlight_on_off(0,1,0,0); //YELLOW light ON
    }

    if (MACHINE_STATE == 1){
        turn_towerlight_on_off(0,0,1,0); //Green light ON if ON button is pressed
        MACHINE_CURRENT_STATE = 1;
        digitalWrite(Vaccum_pump, HIGH); //Turn ON Vaccum pump
    }
    while(Suction_M1.currentPosition() <= 6666 ){
        Suction_M1.move(6666);
        Suction_M2.move(6666);
        Suction_M1.setSpeed(2000);
        Suction_M2.setSpeed(2000);
        Suction_M1.runSpeedToPosition();
        Suction_M2.runSpeedToPosition();
    }
    Suction_M1.stop();
    Suction_M2.stop();

    delay(500);
}

void loop(){
    paper_finish:

    if (MACHINE_STATE == 1){

        turn_towerlight_on_off(0,0,1,0);

        if(joint_paper_finish == 1 || MACHINE_STATE == 2){
            MACHINE_STOP();
        }

        if(weight_state == 0){
            while(scale.get_units() <= weight_set){
                vib_motors_on();}
                weight_state = 1;
                vib_motors_off();
            while(WeightPlate_M1.currentPosition() <= 49){
                WeightPlate_M1.move(50);
                WeightPlate_M2.move(-50);
                WeightPlate_M1.setSpeed(100);
                WeightPlate_M2.setSpeed(100);
                WeightPlate_M1.runSpeedToPosition();
                WeightPlate_M2.runSpeedToPosition();
            }
            delay(500);
            while(digitalRead(Weight_Plate_Home)==HIGH){
                WeightPlate_M1.move(-50);
                WeightPlate_M2.move(50);
                WeightPlate_M1.setSpeed(100);
                WeightPlate_M2.setSpeed(100);
                WeightPlate_M1.runSpeedToPosition();
                WeightPlate_M2.runSpeedToPosition();
            }
        }

        normal_motors_on();

        if (MACHINE_CURRENT_STATE == 1){
            if (Suction_M1.currentPosition() >= 0){
                if (phase == 0 || phase == 1){
                    Suction_M1.move(-6666);
                    Suction_M2.move(-6666);
                    Suction_M1.setSpeed(2000);
                    Suction_M2.setSpeed(2000);
                    Suction_M1.runSpeedToPosition();
                    Suction_M2.runSpeedToPosition();}
                }
            if(Suction_M1.currentPosition() <= 4665 && phase == 0){ 
                phase = 1;
            }
            if (Platform_M1.currentPosition() <= 7760){
                if (phase == 1 || phase == 2){
                    Platform_M1.move(7760);
                    Platform_M2.move(7760);
                    Platform_M1.setSpeed(4000);
                    Platform_M2.setSpeed(4000);
                    Platform_M1.runSpeedToPosition();
                    Platform_M2.runSpeedToPosition();}
            }
            if(Suction_M1.currentPosition() <= 0 && phase == 1){
                phase = 2;
                Suction_M1.stop();
                Suction_M2.stop();
            }
            if(Platform_M1.currentPosition() >= 7760 && phase == 2){
                phase = 3;
                Platform_M1.stop();
                Platform_M2.stop();
            }
            if(phase == 3 && Suction_M1.currentPosition() <= 6666){
                Suction_M1.move(6666);
                Suction_M2.move(6666);
                Suction_M1.setSpeed(2000);
                Suction_M2.setSpeed(2000);
                Suction_M1.runSpeedToPosition();
                Suction_M2.runSpeedToPosition();
            }
            if (phase == 3 && Suction_M1.currentPosition() >= 6666){
                phase = 4;
                Suction_M1.stop();
                Suction_M2.stop();
            }
            if (phase == 4 && Suction_M1.currentPosition() >= 0){
                digitalWrite(Vaccum_valve, LOW);
                Suction_M1.move(-6666);
                Suction_M2.move(-6666);
                Suction_M1.setSpeed(2000);
                Suction_M2.setSpeed(2000);
                Suction_M1.runSpeedToPosition();
                Suction_M2.runSpeedToPosition();
            }
            if (phase == 4 && Suction_M1.currentPosition() <= 0){
                phase = 5;
                MACHINE_CURRENT_STATE = 2;
                Suction_M1.stop();
                Suction_M1.stop();
            }
        }

        if (MACHINE_CURRENT_STATE == 2){
            if(phase == 5 && Cointainer_M1.currentPosition() <= 6666){
                Cointainer_M1.move(6666);
                Cointainer_M2.move(6666);
                Cointainer_M1.setSpeed(2000);
                Cointainer_M2.setSpeed(2000);
                Cointainer_M1.runSpeedToPosition();
                Cointainer_M2.runSpeedToPosition();
                if (UShape.currentPosition() >= 0 && init_UShape == 1){
                UShape.move(-2000);
                UShape.setSpeed(2000);
                UShape.runSpeedToPosition();}
                else{
                    UShape.stop();
                }
            }
            if(phase == 5 && Cointainer_M1.currentPosition() >= 6666){
                Cointainer_M1.stop();
                Cointainer_M2.stop();
                joint_roll_state = 1;
                phase = 6;
            }
            if(joint_roll_state == 1){
                JointRoll_M1.runSpeed();
                JointRoll_M2.runSpeed();
                if(GlassTip_M1.currentPosition() <= 7874){
                    GlassTip_M1.move(7874);
                    GlassTip_M2.move(-7874);
                    GlassTip_M1.setSpeed(2000);
                    GlassTip_M2.setSpeed(2000);
                    GlassTip_M1.runSpeedToPosition();
                    GlassTip_M2.runSpeedToPosition();
                }
                else{
                    GlassTip_M1.stop();
                    GlassTip_M2.stop();
                    MACHINE_CURRENT_STATE = 3;
                    goto MS3;
                }
            }
            if(phase == 6 && MixPlate.currentPosition() <= 100){
                MixPlate.move(100);
                MixPlate.setSpeed(300);
                MixPlate.runSpeedToPosition();
            }
            if(phase == 6 && MixPlate.currentPosition() >= 100){
                MixPlate.stop();
                curmillis = millis();
                if (prevmillis == 0){
                    prevmillis == curmillis;
                }
            }
            if (curmillis - prevmillis > 500){
                phase = 7;
                if(phase == 7 && MixPlate.currentPosition() >=0){
                    MixPlate.move(-100);
                    MixPlate.setSpeed(300);
                    MixPlate.runSpeedToPosition();}
                else {
                    MixPlate.stop();
                    curmillis = 0;
                    prevmillis = 0;
                }
            }
        }

        MS3:

        if(MACHINE_CURRENT_STATE == 3){
            if(joint_roll_state == 1){
                JointRoll_M1.runSpeed();
                JointRoll_M2.runSpeed();}

            if(GlassTip_M1.currentPosition() >= 0){
                if(phase == 7 || phase == 8){
                    GlassTip_M1.move(-7874);
                    GlassTip_M2.move(7874);
                    GlassTip_M1.setSpeed(2000);
                    GlassTip_M2.setSpeed(2000);
                    GlassTip_M1.runSpeedToPosition();
                    GlassTip_M2.runSpeedToPosition();
                        if (GlassTip_M1.currentPosition() <= 1000){
                            phase = 8;
                        }
                    }
            }
            if (phase == 8 && Cointainer_M1.currentPosition() >=0){
                Cointainer_M1.move(-6666);
                Cointainer_M2.move(-6666);
                Cointainer_M1.setSpeed(2000);
                Cointainer_M2.setSpeed(2000);
                Cointainer_M1.runSpeedToPosition();
                Cointainer_M2.runSpeedToPosition();
                if (Cointainer_M1.currentPosition() <=4666 && UShape.currentPosition() <= 2000){
                    UShape.move(2000);
                    UShape.setSpeed(2000);
                    UShape.runSpeedToPosition();
                }
            }
            if(GlassTip_M1.currentPosition() < 0){
                if(phase == 7 || phase == 8){
                    GlassTip_M1.stop();
                    GlassTip_M2.stop();}
            }
            if(phase == 8 && Cointainer_M1.currentPosition() < 0){
                Cointainer_M1.stop();
                Cointainer_M2.stop();
                UShape.stop();
                init_UShape = 1;
                joint_roll_state = 0;
                JointRoll_M1.stop();
                JointRoll_M2.stop();
                phase = 9;
                MACHINE_CURRENT_STATE = 4;
            }   
        }
        if (MACHINE_CURRENT_STATE == 4){
            if(Platform_M1.currentPosition() >= 0){
                if (phase == 9 || phase == 10){
                    Platform_M1.move(-7760);
                    Platform_M2.move(-7760);
                    Platform_M1.setSpeed(4000);
                    Platform_M2.setSpeed(4000);
                    Platform_M1.runSpeedToPosition();
                    Platform_M2.runSpeedToPosition();}
                if(Platform_M1.currentPosition() <= 2640){
                    phase = 10;
                }
            }

            if(phase == 10 && Suction_M1.currentPosition() <= 6666){
                digitalWrite(Vaccum_valve, HIGH);
                Suction_M1.move(6666);
                Suction_M2.move(6666);
                Suction_M1.setSpeed(2000);
                Suction_M2.setSpeed(2000);
                Suction_M1.runSpeedToPosition();
                Suction_M2.runSpeedToPosition();
            }

            if(Platform_M1.currentPosition() < 0){
                if (phase == 9 || phase == 10){
                    Platform_M1.stop();
                    Platform_M2.stop();
                }
            }
            if(phase == 10 && Suction_M1.currentPosition() >= 6666){
                Suction_M1.stop();
                Suction_M2.stop();
                curmillis = millis();
                if(prevmillis == 0){
                    prevmillis = curmillis;
                }
            }
            if(phase == 10 && curmillis - prevmillis > 500){
                phase = 0;
                MACHINE_CURRENT_STATE = 1;
                weight_state = 0;
                curmillis = 0;
                prevmillis = 0;
            }
        }
    }
}

void MACHINE_STOP(){
    while(MACHINE_STATE == 2){
        vib_motors_off();
        normal_motors_off();
        other_motor_off();
        turn_towerlight_on_off(0,1,0,0);}
    digitalWrite(Vaccum_pump, HIGH);
}

void Machine_start_interrupt(){
    MACHINE_STATE = 1;
    joint_paper_finish = 0;
}

void Machine_stop_interrupt(){
    MACHINE_STATE = 2;
}

void Joint_paper_interrupt(){
    joint_paper_finish = 1;
}

void normal_motors_on(){
    //Making all motors with constant operation to run at a constant speed
    Joint_Cutter_M1.runSpeed();
    Joint_Cutter_M2.runSpeed();
    Grinding_M1.runSpeed();
    Grinding_M2.runSpeed();
    Cloth_M1.runSpeed();
    Cloth_M2.runSpeed();
}

void normal_motors_off(){
    //Making all motors with constant operation to stop running
    Joint_Cutter_M1.stop();
    Joint_Cutter_M2.stop();
    Grinding_M1.stop();
    Grinding_M2.stop();
    Cloth_M1.stop();
    Cloth_M2.stop();
    digitalWrite(Vaccum_pump, LOW);
}

void vib_motors_on(){
    //Turning all the vibrating motors on
    digitalWrite(Hopper_Vib_Motor, HIGH);
    digitalWrite(FastSlider_Vib_Motor1, HIGH);
    digitalWrite(FastSlider_Vib_Motor2, HIGH);
    digitalWrite(SlowSlider_Vib_Motor, HIGH);
}

void vib_motors_off(){
    //Turning all the vibrating motors off
    digitalWrite(Hopper_Vib_Motor, LOW);
    digitalWrite(FastSlider_Vib_Motor1, LOW);
    digitalWrite(FastSlider_Vib_Motor2, LOW);
    digitalWrite(SlowSlider_Vib_Motor, LOW);
}

//To control all the tower lights of the machine
void turn_towerlight_on_off(bool red, bool yellow, bool green, bool buzzer){
    if (red == 1){
        digitalWrite(Red_Towerlight, HIGH);}
    else{
        digitalWrite(Red_Towerlight, LOW);}

    if (yellow == 1){
        digitalWrite(Yellow_Towerlight, HIGH);}
    else{
        digitalWrite(Yellow_Towerlight, LOW);}

    if (green == 1){
        digitalWrite(Green_Towerlight, HIGH);}
    else{
        digitalWrite(Green_Towerlight, LOW);}

    if (buzzer == 1){
        digitalWrite(Buzzer_Towerlight, HIGH);}
    else{
        digitalWrite(Buzzer_Towerlight, LOW);}
}

void Machine_Home_Position(){

    bool Weight_Plate_Home_var = 1;
    bool Container_Home_var = 1;
    bool Mix_Plate_Home_var = 1;
    bool U_Shape_Home_var = 1;
    bool Suction_Home_var = 1;
    bool Glass_Tip_Home_var = 1;

    //Move every motor till the home limit switch is not active
    while (digitalRead(Weight_Plate_Home) && digitalRead(Container_Home) && digitalRead(Mix_Plate_Home) && 
        digitalRead(U_Shape_Home) && digitalRead(Suction_Home) && digitalRead(Glass_Tip_Home)){
            if(digitalRead(Weight_Plate_Home) == LOW){ //If the motor is already at home or has reached home then stop it
                Weight_Plate_Home_var = 0;
                WeightPlate_M1.stop(); // Stop moving the stepper
                WeightPlate_M2.stop();
            }
            if(digitalRead(Container_Home) == LOW){ //If the motor is already at home or has reached home then stop it
                Container_Home_var = 0;
                Cointainer_M1.stop(); // Stop moving the stepper
                Cointainer_M2.stop();
            }
            if(digitalRead(Mix_Plate_Home) == LOW){ //If the motor is already at home or has reached home then stop it
                Mix_Plate_Home_var = 0;
                MixPlate.stop(); // Stop moving the stepper
            }
            if(digitalRead(U_Shape_Home) == LOW){ //If the motor is already at home or has reached home then stop it
                U_Shape_Home_var = 0;
                UShape.stop(); // Stop moving the stepper
            }
            if(digitalRead(Suction_Home) == LOW){ //If the motor is already at home or has reached home then stop it
                Suction_Home_var = 0;
                Suction_M1.stop(); // Stop moving the stepper
                Suction_M2.stop();
            }
            if(digitalRead(Glass_Tip_Home) == LOW){ //If the motor is already at home or has reached home then stop it
                Glass_Tip_Home_var = 0;
                GlassTip_M1.stop(); // Stop moving the stepper
                GlassTip_M2.stop();
            }
            if(Weight_Plate_Home_var == 1){
                WeightPlate_M1.setSpeed(100); //Change 
                WeightPlate_M2.setSpeed(100);
                WeightPlate_M1.runSpeed(); // Start moving the stepper
                WeightPlate_M2.runSpeed();
            }
            if(Container_Home_var == 1){
                WeightPlate_M1.runSpeed(); // Start moving the stepper
                WeightPlate_M2.runSpeed();
            }
            if(Mix_Plate_Home_var == 1){
                MixPlate.runSpeed(); // Start moving the stepper
            }
            if(U_Shape_Home_var == 1){
                UShape.runSpeed(); // Start moving the stepper
            }
            if(Suction_Home_var == 1){
                Suction_M1.runSpeed(); // Start moving the stepper
                Suction_M2.runSpeed();
            }
            if(Glass_Tip_Home_var == 1){
                GlassTip_M1.runSpeed(); // Start moving the stepper
                GlassTip_M2.runSpeed();
            }
        }
    // Doing the same for platform
    if (digitalRead(Platform_Origin)){   
        while(digitalRead(Platform_Origin)){
            Platform_M1.runSpeed(); // Start moving the stepper
            Platform_M2.runSpeed();
        }
    }
    Platform_M1.stop(); //Stop moving the stepper
    Platform_M2.stop();
}

void Set_motor_init_pos(){
    //Setting the current position of motors to 0
    Platform_M1.setCurrentPosition(0);
    Platform_M2.setCurrentPosition(0);
    WeightPlate_M1.setCurrentPosition(0);
    WeightPlate_M2.setCurrentPosition(0);
    MixPlate.setCurrentPosition(0);
    Cointainer_M1.setCurrentPosition(0);
    Cointainer_M2.setCurrentPosition(0);
    UShape.setCurrentPosition(0);
    GlassTip_M1.setCurrentPosition(0);
    GlassTip_M2.setCurrentPosition(0);
    Suction_M1.setCurrentPosition(0);
    Suction_M2.setCurrentPosition(0);
}

void other_motor_off(){
    Platform_M1.stop(); //Stop moving the stepper
    Platform_M2.stop();
    WeightPlate_M1.stop();
    WeightPlate_M2.stop();
    Cointainer_M1.stop();
    Cointainer_M2.stop();
    MixPlate.stop();
    UShape.stop();
    Suction_M1.stop();
    Suction_M2.stop();
    GlassTip_M1.stop();
    GlassTip_M2.stop();
    JointRoll_M1.stop();
    JointRoll_M2.stop();
}