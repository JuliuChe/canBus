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
uint8_t numMillisecElapsed = 0;
uint8_t fiftyMillisecElapsed = 0;
uint8_t tenMillisecElapsed = 0;
uint8_t hour = 0, min = 0, sec = 0;
uint8_t numSecElapsed = 0, SecElapsed = 0;
uint8_t inc = 0;
uint8_t maxpwr = 0;

CAR_STATE myCar; //Values of the car 

//Communication variables used to exchange between the car and the controller
CAN_FILTEROBJ_ID fObj;
CAN_MASKOBJ_ID mObj;

uint8_t rxtab[8] = {0, 1, 2, 3, 4, 5, 6, 7};
CAN_RX_MSGOBJ rxObj;

uint8_t txd[8] = {0, 1, 2, 3, 4, 5, 6, 7};
CAN_TX_MSGOBJ txObj;

//Initialize Struct of myCar 

void carStateInit()//OK DONE
{
    myCar.tempomat = 0;
    myCar.gearSel = 'P';
    myCar.motorRpm = 0;
    myCar.carSpeed = 0;
    myCar.brakePedal = 0;
    myCar.accelPedal = 0;
    myCar.contactKey = 0;
    myCar.brokenCar = NO_ERROR;
    myCar.badMessage = NO_MSG_ERROR;
    frontSensorRR();
    steeringWheelRR();
    slopeValueRR();
    myCar.race = NOT_IN_RACE;

    myCar.newSensorValue = 0;
    //Value to be saved until next interrupt
    myCar.sensor.ext_sensor.frontLeftS = 63;
    myCar.sensor.ext_sensor.frontRightS = 63;
    myCar.lastFrontLightInt = 0;
    myCar.lastBackLightInt = 0;
    myCar.lastGearLevel = 0;
    myCar.lastVolume = 0;
    myCar.lastContactKey = 0;
    myCar.pwrBrake = 0;
    myCar.pwr = 0;
    myCar.powerOnStart = 10;
    myCar.odometer = 0;
    myCar.gearChanged = 1;
    myCar.statusChanged = 0;
    myCar.carStop = 0;
    myCar.drive = 0;
}
//Method that set the light intensity (0 to 100) of the front and back lights 

void setLight(uint8_t intensity, uint8_t Light_Type)//OK DONE
{
    uint8_t last = 0;

    if (Light_Type == LIGHT_FRONT)
    {
        last = myCar.lastFrontLightInt;
    }
    else
    {
        last = myCar.lastBackLightInt;
    }

    if (last != intensity)
    {
        txd[0] = intensity;
        txObj.bF.id.ID = (((uint16_t) Light_Type << 4) | myCar.carId); // standard identifier example
        txObj.bF.ctrl.DLC = CAN_DLC_1; // 1 bytes to send
        txObj.bF.ctrl.RTR = 0; // no remote frame
        txObj.bF.id.SID11 = 0; // only used in FD mode
        txObj.bF.ctrl.FDF = 0; // no CAN FD mode
        txObj.bF.ctrl.IDE = 0; // standard identifier format
        txObj.bF.ctrl.BRS = 0; // no data bitrate switch (FD mode)
        txObj.bF.ctrl.ESI = 0;
        CanSend(&txObj, txd);
    }

    if (Light_Type == LIGHT_FRONT)
    {
        myCar.lastFrontLightInt = intensity;
    }
    else
    {
        myCar.lastBackLightInt = intensity;
    }
}

void controlTime() //OK DONE
{

    sec++;
    if (sec == 60)
    {
        sec = 0;
        min++;
    }
    if (min == 60)
    {

        min = 0;
        hour++;
    }
    if (hour == 24)
    {

        // min=0;
        hour = 0;
    }

    if (sec % 2 == 0)
    {
        setTimeInCockpit(hour, min, true);
    }
    else
    {
        setTimeInCockpit(hour, min, false);
    }
}

void setTimeInCockpit(uint8_t hours, uint8_t min, bool sec)//OK DONE
{

    txd[0] = hours;
    txd[1] = min;
    txd[2] = sec;

    txObj.bF.id.ID = ((TIME << 4) | myCar.carId); // standard identifier example
    txObj.bF.ctrl.DLC = CAN_DLC_3; // 3 bytes to send
    txObj.bF.ctrl.RTR = 0; // no remote frame
    txObj.bF.id.SID11 = 0; // only used in FD mode
    txObj.bF.ctrl.FDF = 0; // no CAN FD mode
    txObj.bF.ctrl.IDE = 0; // standard identifier format
    txObj.bF.ctrl.BRS = 0; // no data bitrate switch (FD mode)
    txObj.bF.ctrl.ESI = 0;
    CanSend(&txObj, txd);
}

void initCar() //OK DONE
{
    myCar.carId = getCarId();
}

uint8_t getCarId() //OK DONE
{
    // define mask for filter ------------------------------------------------

    mObj.MID = 0x7FF; // check all the 11 bits in standard ID
    mObj.MSID11 = 0; // 12 bits only used in FD mode
    mObj.MIDE = 1; // match identifier size in filter

    // define filter to use --------------------------------------------------

    fObj.ID = 0xFF; // standard filter 11 bits value
    fObj.SID11 = 0; // 12 bits only used in FD mode
    fObj.EXIDE = 0; // assign to standard identifiers

    CanSetFilter(CAN_FILTER0, &fObj, &mObj);

    //Request ID 

    txd[0] = 0;
    txObj.bF.id.ID = CAR_ID; // standard identifier example
    txObj.bF.ctrl.DLC = CAN_DLC_0; // 0 bytes to send
    txObj.bF.ctrl.RTR = 1; // no remote frame
    txObj.bF.id.SID11 = 0; // only used in FD mode
    txObj.bF.ctrl.FDF = 0; // no CAN FD mode
    txObj.bF.ctrl.IDE = 0; // standard identifier format
    txObj.bF.ctrl.BRS = 0; // no data bitrate switch (FD mode)
    txObj.bF.ctrl.ESI = 0;

    CanSend(&txObj, txd);

    while (CanReceive(&rxObj, rxtab) != 0)
    {
    }
    mObj.MID = 0xF;
    fObj.ID = rxtab[0];
    CanSetFilter(CAN_FILTER0, &fObj, &mObj);

    return rxtab[0];

}

void carStateUpdate() //OK DONE
{
    int8_t temp = CanReceive(&rxObj, rxtab);
    if (temp == 0)
    {
        switch (rxObj.bF.id.ID >> 4)
        {
        case TEMPOMAT:
            myCar.tempomat = rxtab[0];
            myCar.tempoSpeed = rxtab[1];
            break;
        case GEAR_SEL:
            myCar.gearSel = rxtab[0];
            myCar.gearChanged = 1;
            break;
        case EXT_SENSORS:
            if (myCar.race == NOT_IN_RACE)
            {
                frontSensorRR();
                //myCar.newSensorValue
                myCar.sensor.frontSensor = ((((uint16_t) rxtab[0]) << 8) | rxtab[1]); //Be careful with operators' precedence, parenthesis and typecast added 
            }
            else
            {
                myCar.sensor.ext_sensor.frontLeftS = rxtab[0];
                myCar.sensor.ext_sensor.frontRightS = rxtab[1];
                myCar.newSensorValue = 1;
            }
            break;

        case MOTOR_STATUS:
            myCar.motorRpm = ((((uint16_t) rxtab[0]) << 8) | rxtab[1]); //Be careful with operators' precedence, parenthesis and typecast added
            myCar.carSpeed = ((((uint16_t) rxtab[2]) << 8) | rxtab[3]); //Be careful with operators' precedence, parenthesis and typecast added
            myCar.statusChanged = 1;
            break;

        case BRAKE_PEDAL:
            myCar.brakePedal = rxtab[0];
            break;

        case ACCEL_PEDAL:
            myCar.accelPedal = rxtab[0];
            break;

        case CONTACT_KEY:
            myCar.contactKey = rxtab[0];
            break;

        case STEERING_W_REQ:
            myCar.steeringValue = rxtab[0];
            break;

        case BROKEN_CAR:
            myCar.brokenCar = rxtab[0];
            break;
        case BAD_MESSAGE:
            myCar.badMessage = rxtab[0];
            break;

        case SLOPE_REQ:
            myCar.slopeValue = rxtab[0];
            break;

        case RACE:
            myCar.race = rxtab[0];
            break;

        case CAR_ID:
            myCar.carId = rxtab[0];
            break;

        default:
            break;

        }

    }

}


//Set PWR of the motor
// pwr is 0-100
//NOT UNDERSTAND : starter could not understand what this is dong here 

void setPwrMotor(uint8_t pwr, bool starter)//OK DONE
{
    if (myCar.pwr != pwr)
    {
        txd[0] = pwr;
        txd[1] = starter;
        txObj.bF.id.ID = (((uint16_t) PWR_MOTOR) << 4 | myCar.carId); // standard identifier example
        txObj.bF.ctrl.DLC = CAN_DLC_2; // 1 bytes to send
        txObj.bF.ctrl.RTR = 0; // no remote frame
        txObj.bF.id.SID11 = 0; // only used in FD mode
        txObj.bF.ctrl.FDF = 0; // no CAN FD mode
        txObj.bF.ctrl.IDE = 0; // standard identifier format
        txObj.bF.ctrl.BRS = 0; // no data bitrate switch (FD mode)
        txObj.bF.ctrl.ESI = 0;
        CanSend(&txObj, txd);
        myCar.pwr = pwr;
    }


}

void lightsOnBrake()//OK DONE Could be improved with adaptive lights
{
    if (myCar.brakePedal >= 10 && myCar.contactKey == 1)
    {
        setLight(100, LIGHT_BACK);
    }

    else if (myCar.brakePedal < 10 && myCar.contactKey == 1) //<5 to detect that null position is not always exactly 0
    {
        setLight(50, LIGHT_BACK);
    }
    else
    {
        setLight(0, LIGHT_BACK);
    }
}

void setGas()//OK DONE
{
    static uint8_t pwr = 10;
    // uint8_t  inc = 5;
    //uint8_t set=myCar.accelPedal
    if (myCar.carStop == 0)
    {

        if (myCar.tempomat == 0 || myCar.accelPedal > 10)
        {
            if (myCar.motorRpm > 0)
            {
                if (myCar.accelPedal > (pwr))
                {
                    pwr += inc;
                }
                else if ((myCar.accelPedal < pwr) && (myCar.pwr > myCar.powerOnStart))
                {
                    pwr -= inc;
                }


                if (((myCar.motorRpm > 7400) || (myCar.carSpeed > 275))&& (myCar.pwr > myCar.powerOnStart))
                {
                    if (pwr > (inc + inc))
                    {
                        pwr -= (inc + inc);
                    }
                    else
                    {
                        pwr = myCar.powerOnStart;
                    }
                }
                //if (myCar.statusChanged == 1)
                //{
                setPwrMotor(MAX(MIN(maxpwr, pwr), myCar.powerOnStart), 0);
                //    myCar.statusChanged = 0;
                //}
            }
        }
    }
    /*if (!myCar.tempomat)
    {
        uint8_t pwr = MAX(12, MIN(80, myCar.accelPedal));
        if (myCar.carSpeed < 275)
        {
            if (myCar.contactKey == 1 && myCar.motorRpm > 0)
            {
                if (pwr != myCar.pwr)
                {
                    setPwrMotor(pwr, 0);
                }
            }
        }
        else
        {
            setPwrMotor(0, 0);
            //setPwrBrakes(20);
            //setPwrMotor(MIN(12,myCar.accelPedal),0);
        }

    }*/
}

void getBrake()//OK DONE
{
    uint8_t pwr = myCar.brakePedal;
    //if(myCar.carSpeed!=0 && pwr!=0){
    if ((myCar.lastGearLevel > 0) || (myCar.gearSel == 'N'))
    {
        if (myCar.carSpeed != 0) //can be a negative or positive speed
        {
            setPwrBrakes(pwr);
        }
    }
}
//}

//Set pwr factor applied to brakes

void setPwrBrakes(uint8_t pwr)//OK DONE
{
    if (pwr != myCar.pwrBrake)
    {
        myCar.pwrBrake = pwr;
        txd[0] = pwr;
        txObj.bF.id.ID = (((uint16_t) PWR_BRAKE) << 4 | myCar.carId); // standard identifier example
        txObj.bF.ctrl.DLC = CAN_DLC_1; // 1 bytes to send
        txObj.bF.ctrl.RTR = 0; // no remote frame
        txObj.bF.id.SID11 = 0; // only used in FD mode
        txObj.bF.ctrl.FDF = 0; // no CAN FD mode
        txObj.bF.ctrl.IDE = 0; // standard identifier format
        txObj.bF.ctrl.BRS = 0; // no data bitrate switch (FD mode)
        txObj.bF.ctrl.ESI = 0;
        CanSend(&txObj, txd);
    }
}

//Code to be implemented to control the car by the contoller

void carControlUpdate()//OK DONE
{
    static uint8_t countSec = 0;

    countSec += 1;
    numMillisecElapsed += 1;

    tenMillisecElapsed = 1;

    if (numMillisecElapsed == 5)
    {
        fiftyMillisecElapsed = 1;
        numMillisecElapsed = 0;
    }
    if (countSec == 100)
    {
        countSec = 0;
        SecElapsed = 1;
    }



}

void engineAtKeyEvt() //OK DONE
{

    if (myCar.motorRpm == 0 && myCar.contactKey == 1 && myCar.lastContactKey == 0)
    {
        myCar.lastContactKey = 1;
        setPwrMotor(myCar.powerOnStart, 1);
        setLight(100, LIGHT_FRONT);
        setLight(50, LIGHT_BACK);
        setGearLvl(0);
    }
    if ((myCar.contactKey != myCar.lastContactKey) && (myCar.contactKey == 0))
    {
        setPwrMotor(0, 0);
        myCar.motorRpm = 0;
        myCar.carSpeed = 0;
        myCar.lastContactKey = 0;
        setLight(0, LIGHT_FRONT);
        setLight(0, LIGHT_BACK);
        setGearLvl(0);
        setPwrBrakes(100);
        myCar.gearSel = 'P';
        myCar.statusChanged = 0;
        myCar.brokenCar = NO_ERROR;
        myCar.badMessage = NO_MSG_ERROR;
        myCar.race = NOT_IN_RACE;
        myCar.odometer = 0;
        myCar.gearChanged = 1;
        myCar.carStop = 0;

    }
}

void driveAtStart() //OK
{
    //in the if statement && ((myCar.carSpeed<5)&&myCar.carSpeed>-5)
    if (((myCar.gearSel == 'D') || (myCar.gearSel == 'R')) && (myCar.lastGearLevel == 0) && (myCar.accelPedal < 10))
    {
        setPwrBrakes(100);
    }
    else if ((myCar.gearSel == 'N')&& (myCar.brakePedal < 5))
    {
        setPwrBrakes(0);
    }

}

void driveInDrive() //OK
{
    if (myCar.gearSel == 'D')
    {
        if (myCar.lastGearLevel == 0)
        {
            if (myCar.motorRpm > 1300 && myCar.drive == 0)
            {
                setGearLvl(myCar.lastGearLevel + 1);
                myCar.motorRpm = 2500;
                myCar.drive = 1;

            }
        }
        if (myCar.motorRpm > 5000)
        {
            if (myCar.lastGearLevel < 5)
            {
                setGearLvl(myCar.lastGearLevel + 1);
                myCar.motorRpm = 4500;
            }
        }
       /* if (myCar.motorRpm < 2200)
        {
            if (myCar.lastGearLevel > 0)
            {
                setGearLvl(myCar.lastGearLevel - 1);
                myCar.motorRpm = 2500;
                if (myCar.lastGearLevel == 0)
                {
                    myCar.drive = 0;
                }
            }
        }*/
                if (myCar.motorRpm < 2200)
        {
            if (myCar.lastGearLevel > 1)
            {
                setGearLvl(myCar.lastGearLevel - 1);
                myCar.motorRpm = 2500;
             
            }
            else
            {
                if(myCar.motorRpm<1200)
                {
                   
                    setGearLvl(myCar.lastGearLevel - 1);
                    myCar.motorRpm = 1250;
                    myCar.drive=0;
                    
                }
               
            }
        }
    }
}

void setGearLvl(uint8_t g) //OK
{

    if (myCar.lastGearLevel != g)
    {
        txd[0] = g;
        txObj.bF.id.ID = ((GEAR_LVL << 4) | myCar.carId); // standard identifier example
        txObj.bF.ctrl.DLC = CAN_DLC_1; // 1 bytes to send
        txObj.bF.ctrl.RTR = 0; // no remote frame
        txObj.bF.id.SID11 = 0; // only used in FD mode
        txObj.bF.ctrl.FDF = 0; // no CAN FD mode
        txObj.bF.ctrl.IDE = 0; // standard identifier format
        txObj.bF.ctrl.BRS = 0; // no data bitrate switch (FD mode)
        txObj.bF.ctrl.ESI = 0;
        CanSend(&txObj, txd);
        myCar.lastGearLevel = g;
    }
}

void setSpeed(uint16_t speed)//OK DONE
{
    int8_t diff = (int8_t) (speed - myCar.carSpeed);
    static uint8_t set = 0;
    if (diff < 0)
    {
        diff = -diff;
    }
    if (diff > 15)
    {
        set = 15;
    }
    else if (diff > 10)
    {
        set = 10;
    }
    else if (diff > 5)
    {
        set = 5;
    }
    else
    {
        set = 1;
    }
    if (myCar.accelPedal < 50)
    {
        if (myCar.carSpeed <= 275)
        {
            if (myCar.carSpeed > speed)
            {
                setPwrMotor(MIN(MAX(0, myCar.pwr - set), 100), 0);
            }
            else if (myCar.carSpeed < speed)
            {
                setPwrMotor(MAX(MIN(100, myCar.pwr + set), 0), 0);
            }
        }
        else
        {
            setPwrBrakes(20);
        }
    }
    else
    {
        setGas();
    }
}

void tempoOn()//OK DONE
{
    if (myCar.tempomat == 1)
    {
        if (myCar.statusChanged == 1)
        {
            myCar.statusChanged = 0;
            setSpeed(myCar.tempoSpeed);
        }
        if (myCar.brakePedal > 10)
        {
            setTempoOff();
        }
    }

}


//Set tempomat off

void setTempoOff() //OK Done
{
    if (myCar.tempomat == 1)
    {
        txd[0] = 0;
        txObj.bF.id.ID = (((uint16_t) TEMPO_OFF) << 4 | myCar.carId); // standard identifier example
        txObj.bF.ctrl.DLC = CAN_DLC_0; // 0 bytes to send
        txObj.bF.ctrl.RTR = 0; // no remote frame
        txObj.bF.id.SID11 = 0; // only used in FD mode
        txObj.bF.ctrl.FDF = 0; // no CAN FD mode
        txObj.bF.ctrl.IDE = 0; // standard identifier format
        txObj.bF.ctrl.BRS = 0; // no data bitrate switch (FD mode)
        txObj.bF.ctrl.ESI = 0;
        CanSend(&txObj, 0);
        myCar.tempomat = 0;
    }
}

//Pulse to be sent each 100m
//NOT UNDERSTAND THIS MESSAGE

void setKmPulse() //OK DONE
{
    if (myCar.odometer > 3600)
    {
        txObj.bF.id.ID = (((uint16_t) KM_PULSE) << 4 | myCar.carId); // standard identifier example
        txObj.bF.ctrl.DLC = CAN_DLC_0; // 0 bytes to send
        txObj.bF.ctrl.RTR = 0; // no remote frame
        txObj.bF.id.SID11 = 0; // only used in FD mode
        txObj.bF.ctrl.FDF = 0; // no CAN FD mode
        txObj.bF.ctrl.IDE = 0; // standard identifier format
        txObj.bF.ctrl.BRS = 0; // no data bitrate switch (FD mode)
        txObj.bF.ctrl.ESI = 0;
        CanSend(&txObj, 0);
        myCar.odometer = 0;
    }
}

void getDistance() //OK DONE
{
    uint16_t time = 50;
    uint16_t dist = myCar.carSpeed*time;
    dist = dist / 100;
    myCar.odometer += dist;

    setKmPulse();
}

void reverseMode()//OK DONE
{

    if (myCar.gearSel == 'R')
    {
        if (myCar.accelPedal > 10 && myCar.carStop == 0)
        {
            setGearLvl(1);
        }
        else if (myCar.motorRpm < 2000 && myCar.carSpeed == 0)
        {
            setGearLvl(0);
        }
    }
}

void regulationMethod()//OK DONE
{

    if (myCar.gearSel == 'P' || myCar.gearSel == 'N')
    {
        inc = 10;
        maxpwr = 80;
    }
    else
    {
        inc = 5;
        maxpwr = 100;
    }
}

void startAndStop()//OK DONE
{
    if (myCar.race == NOT_IN_RACE)
    {
        if ((myCar.carSpeed == 0) &&(myCar.accelPedal < 5)&&(myCar.lastGearLevel == 0) &&(myCar.brakePedal > 10))
        {
            setPwrMotor(0, 0);
            myCar.carStop = 1;
        }

        if ((myCar.carStop == 1)&&(myCar.accelPedal > 10))
        {
            setPwrMotor(myCar.powerOnStart, 1);
            setGearLvl(0);
            myCar.carStop = 0;
        }
    }
}

//-----------------------------------------------------------------------------TO BE COMPLETED----------------------------------/

void resetBrokenCar() // OK DONE
{
    if (myCar.brokenCar != NO_ERROR)
    {
        carStateInit();
        resetCarState();
    }

}

void brakeAccelConciliation()
{
    if (myCar.gearSel == 'R' || myCar.gearSel == 'D')
    {
        if (myCar.lastGearLevel > 0)
        {
            if (myCar.brakePedal > 10)
            {
                if (myCar.accelPedal > 10)
                {
                    myCar.accelPedal = 0;
                }
            }
        }

    }

}


//FROM CAR TO CONTROLLER
//get steering Wheel REQ value

//FROM CONTROLLER TO CAR
//Auto_steering

void setSteeringPos(int8_t pos, bool automatic)
{
    if (pos != myCar.steeringValue)
    {
        txd[0] = pos;
        txd[1] = automatic;
        txObj.bF.id.ID = ((AUTO_STEERING << 4) | myCar.carId); // standard identifier example
        txObj.bF.ctrl.DLC = CAN_DLC_2; // 2 bytes to send
        txObj.bF.ctrl.RTR = 0; // no remote frame
        txObj.bF.id.SID11 = 0; // only used in FD mode
        txObj.bF.ctrl.FDF = 0; // no CAN FD mode
        txObj.bF.ctrl.IDE = 0; // standard identifier format
        txObj.bF.ctrl.BRS = 0; // no data bitrate switch (FD mode)
        txObj.bF.ctrl.ESI = 0;
        CanSend(&txObj, txd);
        myCar.steeringValue = pos;
    }
}

void raceMode()
{
    if (myCar.race == READY_RACE)
    {
        setSteeringPos(0, 1);
    }
    else
    {
        if (myCar.newSensorValue == 1)
        {
            myCar.newSensorValue = 0;
            if (myCar.lastGearLevel > 0)
            {

                if ((myCar.sensor.ext_sensor.frontLeftS < 25) || (myCar.sensor.ext_sensor.frontRightS < 25))
                {
                    if ((myCar.sensor.ext_sensor.frontLeftS < myCar.sensor.ext_sensor.frontRightS))
                    {
                        setSteeringPos(myCar.steeringValue + 5 , 1);
                    }
                    else
                    {
                        setSteeringPos(myCar.steeringValue - 5, 1);
                    }
                }
                else
                {
                    setSteeringPos(0, 1);
                }
            }
        }
        else
                {
                    setSteeringPos(0, 1);
                }
    }
}



//Enables noise of motor
//If motorDriven is set to 0, the noise will remain as slow speed
//If motorDriven is set to 1, the noise will increase with the speed

void setAudio(uint8_t volume, bool motorDriven)//TO BE TESTED
{
    uint8_t lastVolume = 0;
    lastVolume = myCar.lastVolume;



    if (lastVolume != volume)
    {
        txd[0] = volume;
        txd[1] = motorDriven;
        txObj.bF.id.ID = ((AUDIO << 4) | myCar.carId); // standard identifier example
        txObj.bF.ctrl.DLC = CAN_DLC_2; // 1 bytes to send
        txObj.bF.ctrl.RTR = 0; // no remote frame
        txObj.bF.id.SID11 = 0; // only used in FD mode
        txObj.bF.ctrl.FDF = 0; // no CAN FD mode
        txObj.bF.ctrl.IDE = 0; // standard identifier format
        txObj.bF.ctrl.BRS = 0; // no data bitrate switch (FD mode)
        txObj.bF.ctrl.ESI = 0;
        CanSend(&txObj, txd);
        myCar.lastVolume = volume;
    }
}



//Request value of RR fields
//FRONT_SENS_REQ

void frontSensorRR()//TO BE TESTED
{

    //Request sensorValue 
    txObj.bF.id.ID = ((FRONT_SENS_REQ << 4) | myCar.carId);
    ; // standard identifier example
    txObj.bF.ctrl.DLC = CAN_DLC_0; // 0 bytes to send
    txObj.bF.ctrl.RTR = 1; // no remote frame
    txObj.bF.id.SID11 = 0; // only used in FD mode
    txObj.bF.ctrl.FDF = 0; // no CAN FD mode
    txObj.bF.ctrl.IDE = 0; // standard identifier format
    txObj.bF.ctrl.BRS = 0; // no data bitrate switch (FD mode)
    txObj.bF.ctrl.ESI = 0;

    CanSend(&txObj, txd);
    /*while (CanReceive(&rxObj, rxtab) == 1)
    {
    }
    myCar.sensor.frontSensor = (((uint16_t) rxtab[0] << 8) | rxtab[1]);
     */
}

void steeringWheelRR()//TO BE TESTED
{
    txObj.bF.id.ID = ((STEERING_W_REQ << 4) | myCar.carId); // standard identifier example
    txObj.bF.ctrl.DLC = CAN_DLC_1; // 1 bytes to send
    txObj.bF.ctrl.RTR = 1; // no remote frame
    txObj.bF.id.SID11 = 0; // only used in FD mode
    txObj.bF.ctrl.FDF = 0; // no CAN FD mode
    txObj.bF.ctrl.IDE = 0; // standard identifier format
    txObj.bF.ctrl.BRS = 0; // no data bitrate switch (FD mode)
    txObj.bF.ctrl.ESI = 0;

    CanSend(&txObj, txd);
    while (CanReceive(&rxObj, rxtab) == 1)
    {
    }
    myCar.steeringValue = (int8_t) rxtab[0];
}

void slopeValueRR()//TO BE Tested
{
    txObj.bF.id.ID = SLOPE_REQ; // standard identifier example
    txObj.bF.ctrl.DLC = CAN_DLC_0; // 0 bytes to send
    txObj.bF.ctrl.RTR = 1; // no remote frame
    txObj.bF.id.SID11 = 0; // only used in FD mode
    txObj.bF.ctrl.FDF = 0; // no CAN FD mode
    txObj.bF.ctrl.IDE = 0; // standard identifier format
    txObj.bF.ctrl.BRS = 0; // no data bitrate switch (FD mode)
    txObj.bF.ctrl.ESI = 0;

    CanSend(&txObj, txd);
    while (CanReceive(&rxObj, rxtab) == 1)
    {
    }
    myCar.slopeValue = (int8_t) rxtab[0];
}





//Reset Car (only for debug)

void resetCarState() //OK DONE
{
    txObj.bF.id.ID = (((uint16_t) CAR_RST) << 4 | myCar.carId); // standard identifier example
    txObj.bF.ctrl.DLC = CAN_DLC_0; // 0 bytes to send
    txObj.bF.ctrl.RTR = 0; // no remote frame
    txObj.bF.id.SID11 = 0; // only used in FD mode
    txObj.bF.ctrl.FDF = 0; // no CAN FD mode
    txObj.bF.ctrl.IDE = 0; // standard identifier format
    txObj.bF.ctrl.BRS = 0; // no data bitrate switch (FD mode)
    txObj.bF.ctrl.ESI = 0;
    CanSend(&txObj, 0);

}

//get FRONT_SEN_REQ value

uint16_t getFrontSenValue()
{
}



//get Slope_REQ value

int8_t getSlopeValue()
{

}

