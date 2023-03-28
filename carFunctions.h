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
  }CAR_STATE;
  
  extern CAR_STATE myCar;
  
void initCar();
uint8_t getCarId();
void setFrontLight(uint8_t intensity);
void carStateUpdate()();




#ifdef	__cplusplus
}
#endif

#endif	/* CARFUNCTIONS_H */

