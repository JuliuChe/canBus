/* 
 * File:   carFunctions.h
 * Author: Julie
 *
 * Created on March 24, 2023, 4:04 PM
 */

#ifndef CARFUNCTIONS_H
#define	CARFUNCTIONS_H

#ifdef	__cplusplus
extern "C" {
#endif

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

    //Request driven messages
#define FRONT_SENS_REQ 0x3
#define STEERING_W_REQ 0x9
#define SLOPE_REQ 0xC
#define CAR_ID 0xFF //In case we have a problem, delete one F 

    //Event driven messages      
#define TEMPOMAT 0x1
#define GEAR_SEL 0x2
#define EXT_SENSORS 0x3  
#define MOTOR_STATUS 0x4
#define BRAKE_PEDAL 0x6
#define ACCEL_PEDAL 0x7
#define CONTACT_KEY 0x8
#define BROKEN_CAR 0xA
#define BAD_MESSAGE 0xB
#define RACE 0xD

    //Request to Car
#define LIGHT_FRONT 0x11
#define LIGHT_BACK 0x12
#define TIME 0x13
#define GEAR_LVL 0x14
#define AUDIO 0x15
#define PWR_MOTOR 0x16
#define PWR_BRAKE 0x17
#define TEMPO_OFF 0x18
#define KM_PULSE 0x19
#define AUTO_STEERING 0x1A
#define CAR_RST 0x1F
#define TOT_KM 0x20
#define SETUP_ANALOG 0x21
#define KP 1.2//0.8
#define KI 0.0//0.1//0.1 
#define KD 0.2//0.6//1.4


    extern uint8_t tenMillisecElapsed;
    extern uint8_t numMillisecElapsed;
    extern uint8_t fiftyMillisecElapsed;
    extern uint8_t hour, min, sec;
    extern uint8_t SecElapsed;
    extern uint8_t inc;
    extern uint8_t maxpwr;

    extern CAN_FILTEROBJ_ID fObj;
    extern CAN_MASKOBJ_ID mObj;
    extern uint8_t txd[8];
    extern uint8_t rxtab[8];
    extern CAN_RX_MSGOBJ rxObj1;
    extern CAN_TX_MSGOBJ txObj;

    typedef enum carError {
        STARTER = 1, RPM, SPEED, GEAR_CHG, POWER_MOTOR, WALL, RACE_END, NO_ERROR
    } carErrors;

    typedef enum msgError {
        WRONG_LGTH, WRONG_PARAM, UNKNWN_MSG_ID, NO_MSG_ERROR
    } msgErrors;

    typedef enum modes {
        NOT_IN_RACE, READY_RACE, RACE_START
    } mode;

    typedef struct extSensors {
        uint8_t frontLeftS; //Changes at event, in car race mode only
        uint8_t frontRightS; //Changes at event, in car race mode only
    } RACE_SENSORS;

    typedef struct Sensors {
        uint16_t frontSensor; //has to be requested by controller
        RACE_SENSORS ext_sensor;
    } SENSOR;

    typedef struct CARSTATE {
        bool tempomat;
        char gearSel;

        SENSOR sensor;
        uint16_t motorRpm;
        int16_t carSpeed; //Signed, a negative value means it's going backwards
        uint8_t brakePedal;
        uint8_t accelPedal;
        bool contactKey;
        int8_t steeringValue; // -100 to +100 has to be requested by controller
        carErrors brokenCar;
        msgErrors badMessage;
        int8_t slopeValue; // -100 to +100 has to be requested by controller
        mode race;
        uint8_t carId;
        uint16_t tempoSpeed;
        uint8_t newSensorValue;
        uint8_t newSensorValueNotRace;
        uint8_t carStop;

        //Value to be saved until next interrupt
        uint8_t lastFrontLightInt;
        uint8_t lastBackLightInt;
        uint8_t lastGearLevel;
        uint8_t lastVolume;
        bool lastContactKey;
        uint8_t pwr;
        uint8_t pwrBrake;
        uint8_t statusChanged;
        uint8_t powerOnStart;
        uint16_t odometer;
        uint8_t gearChanged;
        uint8_t drive;
        int8_t Anglecorrection;
        int16_t LastSpeed;
        uint8_t LastAccelPedal;

    } CAR_STATE;

    extern CAR_STATE myCar;
    
///////////////////////////////////FUNCTIONS PROTOTYPES/////////////////////////////////////////
    //main function executed in interrupt
    void carControlUpdate();


///////////////////////LOGIC OF THE CAR FUNCTIONS//////////////////////////////////////////// 
    void initCar();
    void controlTime();
    void lightsOnBrake();
    void setGas();
    //Set pwr factor applied to brakes
    void getBrake();
    void engineAtKeyEvt();
    void driveAtStart();
    void driveInDrive();
    void setSpeed(uint16_t speed);
    void tempoOn();
    void getDistance();
    void reverseMode();
    void regulationMethod();
    void startAndStop();
    void brakeAccelConciliation();
    void resetBrokenCar();
    void raceMode();
    
    
    //////////////////////Initalize status of each car component////////////////////////////////////
    void carStateInit();

    /////////////////GLOBAL TREATMENT OF MESSAGES SENT BY THE CAR//////////////////////////////
        void carStateUpdate();
        
    ////////////////////////RR REQUEST TO THE CAR/////////////////////////////////////////////////////
        //Used only at the start
    uint8_t getCarId();
    void steeringWheelRR();
    void slopeValueRR();
    void frontSensorRR();
    
    ///////////////////////CAN REQUEST TO THE CAR////////////////////////////////////////////////////
        //Set PWR of the motor
    // pwr is 0-100
    void setPwrMotor(uint8_t pwr, bool starter);
        //For the Light_Type use the following define : LIGHT_FRONT or LIGHT_BACK
    void setLight(uint8_t intensity, uint8_t Light_Type);
        //For sec, if you want the colon to blink put 1, if not put 0
    void setTimeInCockpit(uint8_t hours, uint8_t min, bool sec);
        //Auto_steering
    void setSteeringPos(int8_t pos, bool automatic);
        //Pulse to be sent each 100m
    void setKmPulse();
        //Set tempomat off
    void setTempoOff(); //to be impl. if brake are over x%, set it off
        //Sets the gear level 
    //0 is neutral
    //1-5 is gear level must be increased to increase car speed
    void setGearLvl(uint8_t g);
        void setPwrBrakes(uint8_t pwr);
        
        
    //UNUSED FUNCTION
    void torqueControl();
    //Reset Car (only for debug)
    void resetCarState();

    //Enables noise of motor
    void setAudio(uint8_t volume, bool motorDriven);

#ifdef	__cplusplus
}
#endif

#endif	/* CARFUNCTIONS_H */

