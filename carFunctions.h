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
    


extern CAN_FILTEROBJ_ID fObj;
 extern CAN_MASKOBJ_ID mObj;
  extern uint8_t txd[8];
  extern uint8_t rxtab[8];
  extern CAN_RX_MSGOBJ rxObj1;
  extern  CAN_TX_MSGOBJ txObj;
  typedef enum carError{STARTER=1, RPM,SPEED, GEAR_CHG, POWER_MOTOR, WALL, RACE_END}carErrors;
  typedef enum msgError{WRONG_LGTH, WRONG_PARAM, UNKNWN_MSG_ID}msgErrors;
  typedef enum modes{NOT_IN_RACE, READY_RACE, RACE_START}mode;
  
  typedef struct CARSTATE{
      bool tempomat;
      char gear;
      uint16_t frontSensor; //has to be requested by controller
      uint8_t frontLeftS; //Changes at event, in car race mode only
      uint8_t frontRightS;//Changes at event, in car race mode only
      uint16_t motorRpm;
      int16_t carSpeed;  //Signed, a negative value means it's going backwards
      uint8_t brakePedal;
      uint8_t accelPedal;
      bool contactKey;
      int8_t steeringValue; // -100 to +100 has to be requested by controller
      carErrors brokenCar;
      msgErrors badMessage;
      int8_t slopeValue; // -100 to +100 has to be requested by controller
      mode race;
      uint8_t carId;
      
      //Value to be saved until next interrupt
      uint8_t lastFrontLightInt;
      uint8_t lastBackLightInt;
      uint8_t lastGearLevel;
      
      
  }CAR_STATE;
  
  extern CAR_STATE myCar;
  
void initCar();
uint8_t getCarId();
void carStateUpdate();


//For the Light_Type use the following define : LIGHT_FRONT or LIGHT_BACK
void setLight(uint8_t intensity, uint8_t Light_Type); 

//For sec, if you want the colon to blink put 1, if not put 0
void setTimeInCockpit(uint8_t hours, uint8_t min, bool sec);

//Sets the gear level 
//0 is neutral
//1-5 is gear level must be increased to increase car speed
void setGearLvl(uint8_t g);

//-----------------------------------------------------------------------------TO BE COMPLETED IN carFunction.c----------------------------------/

//main function executed in interrupt
void carControlUpdate();


//Enables noise of motor
//If motorDriven is set to 0, the noise will remain as slow speed
//If motorDriven is set to 1, the noise will increase with the speed
void setAudio(uint8_t volume, bool motorDriven);

//Set PWR of the motor
// pwr is 0-100
//NOT UNDERSTAND : starter could not understand what this is dong here 
void setPwrMotor(uint8_t pwr, bool starter);

//Set pwr factor applied to brakes
void setPwrBrakes(uint8_t pwr, bool starter);

//Set tempomat off
void setTempoOff(); //to be impl. if brake are over x%, set it off

//Pulse to be sent each 100m
//NOT UNDERSTAND THIS MESSAGE
void setKmPulse();
        
//Auto_steering
void setSteeringPos(int8_t pos, bool auto);

//Reset Car (only for debug)
void resetCarState();

//get FRONT_SEN_REQ value
uint16_t getFrontSenValue();

//get steering Wheel REQ value
int8_t getSteeringValue();

//get Slope_REQ value
int8_t getSlopeValue();










#ifdef	__cplusplus
}
#endif

#endif	/* CARFUNCTIONS_H */

