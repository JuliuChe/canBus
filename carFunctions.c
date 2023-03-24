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




void setFrontLight(uint8_t intensity)
{
    static uint8_t last = 0;
    if(last!=intensity)
    {
    CAN_TX_MSGOBJ txObj;
    uint8_t txd[8] = {0,1,2,3,4,5,6,7};  
    txd[0]=intensity;
    txObj.bF.id.ID = ((LIGHT_FRONT<<4)|carId);         // standard identifier example
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

void initCarCommunication()
{
    carId = getCarId();
}

uint8_t getCarId()
{

    CAN_TX_MSGOBJ txObj;
    uint8_t txd[8] = {0,1,2,3,4,5,6,7};  
    txd[0]=0;
    txObj.bF.id.ID = (CAR_ID<<4)|0x0F;         // standard identifier example
    txObj.bF.ctrl.DLC = CAN_DLC_1;  // 1 bytes to send
    txObj.bF.ctrl.RTR = 1;          // no remote frame
    txObj.bF.id.SID11 = 0;          // only used in FD mode
    txObj.bF.ctrl.FDF = 0;          // no CAN FD mode
    txObj.bF.ctrl.IDE = 0;          // standard identifier format
    txObj.bF.ctrl.BRS = 0;          // no data bitrate switch (FD mode)
    txObj.bF.ctrl.ESI = 0;
    
     CAN_RX_MSGOBJ rxObj1;
     uint8_t rxtab[1]={0};
    
    
     CanSend(&txObj,txd);
    
       // define filter to use --------------------------------------------------
    CAN_FILTEROBJ_ID fObj;
    fObj.ID = 0xF;              // standard filter 11 bits value
    fObj.SID11 = 0;               // 12 bits only used in FD mode
    fObj.EXIDE = 0;               // assign to standard identifiers
    // define mask for filter ------------------------------------------------
    CAN_MASKOBJ_ID mObj;
    mObj.MID = 0xF;             // check all the 11 bits in standard ID
    mObj.MSID11 = 0;              // 12 bits only used in FD mode
    mObj.MIDE = 1;                // match identifier size in filter
    CanSetFilter(CAN_FILTER0,&fObj,&mObj);
    
  if(CanReceive(&rxObj1, rxtab)==0){
      if((rxObj1.bF.id.ID >> 4)==CAR_ID)
      {
      return (*rxtab);
  }
  }

    
    return 0;
}

