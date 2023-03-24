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
#define CAR_ID 0xF

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
    

uint8_t carId =0;

void initCarCommunication();
uint8_t getCarId();
void setFrontLight(uint8_t intensity);



#ifdef	__cplusplus
}
#endif

#endif	/* CARFUNCTIONS_H */

