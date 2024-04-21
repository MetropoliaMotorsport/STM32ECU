
// Keep that file updated with the devices you want to use in the car

#include "ecumain.h"
#include "errors.h"
#include "canecu.h"
#include "bms.h"
#include "output.h"
#include "power.h"
#include "debug.h"
#include "node_device.h"


//TODO Keep it updated with the devices you want to use in the car
bool processBPPS(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle);
bool processAPPS1(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle);
bool processAPPS2(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle);
bool processSteeringAngle(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle);
bool processWaterLevel(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle);
bool processHeavesFront(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle);
bool processHeavesRear(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle);
bool processRolls1(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle);
bool processRolls2(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle);
bool processBrakeFront(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle);
bool processBrakeRear(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle);


    BPPS = { &DeviceState.BPPS, BPPS_ID, 8, processBPPS, NULL, 0 };
    APPS1 = { &DeviceState.APPS1, APPS1_ID, 8, processAPPS1, NULL, 0 };
    APPS2 = { &DeviceState.APPS2, APPS2_ID, 8, processAPPS2, NULL, 0 };
    SteeringAngle = { &DeviceState.SteeringAngle, STEERINGANGLE_ID, 8, processSteeringAngle, NULL, 0 };
    WaterLevel = { &DeviceState.WaterLevel, WATERLEVEL_ID, 8, processWaterLevel, NULL, 0 };
    HeavesFront = { &DeviceState.HeavesFront, HEAVESFRONT_ID, 8, processHeavesFront, NULL, 0 };
    Rolls1 = { &DeviceState.Rolls1, ROLLS1_ID, 8, processRolls1, NULL, 0 };
    Rolls2 = { &DeviceState.Rolls2, ROLLS2_ID, 8, processRolls2, NULL, 0 };
    BrakeFront = { &DeviceState.BrakeFront, BRAKEFRONT_ID, 8, processBrakeFront, NULL, 0 };
    BrakeRear = { &DeviceState.BrakeRear, BRAKEREAR_ID, 8, processBrakeRear, NULL, 0 };




int initNodeDevices( void ){

    RegisterCan2Message( &BPPS );
    RegisterCan2Message( &APPS1 );
    RegisterCan2Message( &APPS2 );
    RegisterCan2Message( &SteeringAngle );
    RegisterCan2Message( &WaterLevel );
    RegisterCan2Message( &HeavesFront );
    RegisterCan2Message( &HeavesRear );
    RegisterCan2Message( &Rolls1 );
    RegisterCan2Message( &Rolls2 );
    RegisterCan2Message( &BrakeFront );
    RegisterCan2Message( &BrakeRear );


    //TODO add more devices
    return 0;

}
