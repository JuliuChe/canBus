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
 
 uint8_t rxtab[8]={0,1,2,3,4,5,6,7};
 CAN_RX_MSGOBJ rxObj;
 
 uint8_t txd[8] = {0,1,2,3,4,5,6,7};
 CAN_TX_MSGOBJ txObj;


 //Method that set the light intensity (0 to 100) of the front and back lights 
void setLight(uint8_t intensity, uint8_t Light_Type)//TO BE TESTED
{
    uint8_t last=0;
    
    if(Light_Type=LIGHT_FRONT)
    {
    last=myCar.lastFrontLightInt;
    }
    else
    {
        last=myCar.lastBackLightInt;
    }
    if(myCar.contactKey==0)
    {
        intensity = 0;
    }
    if(last!=intensity)
    {
    txd[0]=intensity;
    txObj.bF.id.ID = ((Light_Type<<4)|myCar.carId);         // standard identifier example
    txObj.bF.ctrl.DLC = CAN_DLC_1;  // 1 bytes to send
    txObj.bF.ctrl.RTR = 0;          // no remote frame
    txObj.bF.id.SID11 = 0;          // only used in FD mode
    txObj.bF.ctrl.FDF = 0;          // no CAN FD mode
    txObj.bF.ctrl.IDE = 0;          // standard identifier format
    txObj.bF.ctrl.BRS = 0;          // no data bitrate switch (FD mode)
    txObj.bF.ctrl.ESI = 0;
    CanSend(&txObj,txd);
    //last=intensity;
    }
    
    if(Light_Type=LIGHT_FRONT)
    {
    myCar.lastFrontLightInt=intensity;
    }
    else
    {
    myCar.lastBackLightInt=intensity;
    }
}

void setTimeInCockpit(uint8_t hours, uint8_t min, char sec)//TO BE TESTED
{
    
    txd[0]=hours;
    txd[1]=min;
    txd[2]=sec;
    
    txObj.bF.id.ID = ((TIME<<4)|myCar.carId);         // standard identifier example
    txObj.bF.ctrl.DLC = CAN_DLC_3;  // 3 bytes to send
    txObj.bF.ctrl.RTR = 0;          // no remote frame
    txObj.bF.id.SID11 = 0;          // only used in FD mode
    txObj.bF.ctrl.FDF = 0;          // no CAN FD mode
    txObj.bF.ctrl.IDE = 0;          // standard identifier format
    txObj.bF.ctrl.BRS = 0;          // no data bitrate switch (FD mode)
    txObj.bF.ctrl.ESI = 0;
    CanSend(&txObj,txd);
}

void setGearLvl(uint8_t g) //TO BE TESTED
{
  uint8_t last = 0;
  last=myCar.lastGearLevel;
  
  if(last!=g)
  {
        txd[0]=g;
    txObj.bF.id.ID = ((GEAR_LVL<<4)|myCar.carId);         // standard identifier example
    txObj.bF.ctrl.DLC = CAN_DLC_1;  // 1 bytes to send
    txObj.bF.ctrl.RTR = 0;          // no remote frame
    txObj.bF.id.SID11 = 0;          // only used in FD mode
    txObj.bF.ctrl.FDF = 0;          // no CAN FD mode
    txObj.bF.ctrl.IDE = 0;          // standard identifier format
    txObj.bF.ctrl.BRS = 0;          // no data bitrate switch (FD mode)
    txObj.bF.ctrl.ESI = 0;
    CanSend(&txObj,txd);
    myCar.lastGearLevel=g;
  }
 
}


void initCar() //OK DONE
{
    myCar.carId= getCarId();
}

uint8_t getCarId() //OK DONE
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
     
     

  
    
  while(CanReceive(&rxObj, rxtab)==1){}
      mObj.MID = 0xF;
      fObj.ID = rxtab[0];
      CanSetFilter(CAN_FILTER0,&fObj,&mObj);
      
       return rxtab[0];
      
}

void carStateUpdate() //OK DONE
{
    if (CanReceive(&rxObj, rxtab) == 0)
    {
        volatile uint8_t dummy =25;

        switch (rxObj.bF.id.ID >> 4)
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
            break;

        }
    
    }
    
}


//-----------------------------------------------------------------------------TO BE COMPLETED----------------------------------/

//Enables noise of motor
//If motorDriven is set to 0, the noise will remain as slow speed
//If motorDriven is set to 1, the noise will increase with the speed
void setAudio(uint8_t volume, bool motorDriven)
{
}

//Set PWR of the motor
// pwr is 0-100
//NOT UNDERSTAND : starter could not understand what this is dong here 
void setPwrMotor(uint8_t pwr, bool starter)
{
}

//Set pwr factor applied to brakes
void setPwrBrakes(uint8_t pwr, bool starter)
{
}

//Set tempomat off
void setTempoOff() //to be impl. if brake are over x%, set it off
{
}

//Pulse to be sent each 100m
//NOT UNDERSTAND THIS MESSAGE
void setKmPulse()
{
}
        
//Auto_steering
void setSteeringPos(int8_t pos, bool auto)
{
}

//Reset Car (only for debug)
void resetCarState()
{
}

//get FRONT_SEN_REQ value
uint16_t getFrontSenValue()
{
}

//get steering Wheel REQ value
int8_t getSteeringValue()
{
}

//get Slope_REQ value
int8_t getSlopeValue()
{
}


   //Code to be implemented to control the car by the contoller
void carControlUpdate()
{
    if(myCar.contactKey==0)
    {
        
        
    
    }
 

}
