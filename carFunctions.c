/* 
 * File:   carFunctions.c
 * Author: Julie
 *
 * Created on March 24, 2023, 3:59 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "mcc_generated_files/mcc.h"
#include "can.h"
#include "carFunctions.h"

/*
 * 
 */

CAR_STATE myCar; //Values of the car 

//Communication variables used to exchange between the car and the controller
 CAN_FILTEROBJ_ID fObj; 
 CAN_MASKOBJ_ID mObj;
 uint8_t txd[8] = {0,1,2,3,4,5,6,7};
 uint8_t rxtab[8]={0,1,2,3,4,5,6,7};
 CAN_RX_MSGOBJ rxObj1;
 CAN_TX_MSGOBJ txObj;


 //Method that set the light intensity of the front lights (0 to 100)
void setFrontLight(uint8_t intensity)
{
    static uint8_t last = 0;
    if(last!=intensity)
    {
    txd[0]=intensity;
    txObj.bF.id.ID = ((LIGHT_FRONT<<4)|myCar.carId);         // standard identifier example
    txObj.bF.ctrl.DLC = CAN_DLC_1;  // 1 bytes to send
    txObj.bF.ctrl.RTR = 0;          // no remote frame
    txObj.bF.id.SID11 = 0;          // only used in FD mode
    txObj.bF.ctrl.FDF = 0;          // no CAN FD mode
    txObj.bF.ctrl.IDE = 0;          // standard identifier format
    txObj.bF.ctrl.BRS = 0;          // no data bitrate switch (FD mode)
    txObj.bF.ctrl.ESI = 0;
    CanSend(&txObj,txd);
    last=intensity;
    }
}

void initCar()
{
    myCar.carId= getCarId();
}

uint8_t getCarId()
{
  

    // define mask for filter ------------------------------------------------
   
    mObj.MID = 0x7FF;             // check all the 11 bits in standard ID
    mObj.MSID11 = 0;              // 12 bits only used in FD mode
    mObj.MIDE = 1;                // match identifier size in filter
    
       // define filter to use --------------------------------------------------
   
    fObj.ID = 0xFF;              // standard filter 11 bits value
    fObj.SID11 = 0;               // 12 bits only used in FD mode
    fObj.EXIDE = 0;               // assign to standard identifiers
    
    CanSetFilter(CAN_FILTER0,&fObj,&mObj);
    
    //Request ID 
   
    txd[0]=0;
    txObj.bF.id.ID = CAR_ID;         // standard identifier example
    txObj.bF.ctrl.DLC = CAN_DLC_0;  // 0 bytes to send
    txObj.bF.ctrl.RTR = 1;          // no remote frame
    txObj.bF.id.SID11 = 0;          // only used in FD mode
    txObj.bF.ctrl.FDF = 0;          // no CAN FD mode
    txObj.bF.ctrl.IDE = 0;          // standard identifier format
    txObj.bF.ctrl.BRS = 0;          // no data bitrate switch (FD mode)
    txObj.bF.ctrl.ESI = 0;    
    
             CanSend(&txObj,txd);
     
     

  
    
  while(CanReceive(&rxObj1, rxtab)==1){}
      mObj.MID = 0xF;
      fObj.ID = rxtab[0];
      CanSetFilter(CAN_FILTER0,&fObj,&mObj);
      
       return rxtab[0];
      
}

void carStateUpdate()
{
    if (CanReceive(&rxObj1, rxtab) == 0)
    {
        volatile uint8_t dummy =25;

        switch (rxObj1.bF.id.ID >> 4)
        {
        case TEMPOMAT:
            myCar.tempomat = rxtab[0];
            break;
        case GEAR_SEL:
            myCar.gear=rxtab[0];
            break;
        case EXT_SENSORS:
            if (myCar.race==NOT_IN_RACE)
            {
                myCar.frontSensor=((((uint16_t)rxtab[0])<<8)|rxtab[1]); //Be careful with operators' precedence, parenthesis and typecast added 
            }
            else
            {
            myCar.frontLeftS=rxtab[0];
            myCar.frontRightS=rxtab[1];
            }
            break;

        case MOTOR_STATUS:
            myCar.motorRpm=((((uint16_t)rxtab[0])<<8)|rxtab[1]); //Be careful with operators' precedence, parenthesis and typecast added
            myCar.carSpeed=((((uint16_t)rxtab[2])<<8)|rxtab[3]); //Be careful with operators' precedence, parenthesis and typecast added
            break;

        case BRAKE_PEDAL:
            myCar.brakePedal=rxtab[0];
            break;

        case ACCEL_PEDAL:
            myCar.accelPedal=rxtab[0];
            break;

        case CONTACT_KEY:
            myCar.contactKey=rxtab[0];
            break;

        case STEERING_W_REQ:
            myCar.steeringValue=rxtab[0];
            break;

        case BROKEN_CAR:
            myCar.brokenCar=rxtab[0];
            break;
        case BAD_MESSAGE:
            myCar.badMessage=rxtab[0];
            break;

        case SLOPE_REQ:
            myCar.slopeValue=rxtab[0];
            break;

        case RACE:
            myCar.race=rxtab[0];
            break;

        case CAR_ID:
            myCar.carId=rxtab[0];
            break;

        default:
            setFrontLight(10);
            break;

        }
    
    }
    
}


void carControlUpdate()
{
    //Code to be implemented to control the car by the contoller

}
