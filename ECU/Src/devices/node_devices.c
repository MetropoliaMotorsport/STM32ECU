
// Keep that file updated with the devices you want to use in the car

#include "ecumain.h"
#include "errors.h"
#include "canecu.h"
#include "bms.h"
#include "output.h"
#include "power.h"
#include "debug.h"
#include "node_devices.h"

//TODO Keep it updated with the devices you want to use in the car
bool processBPPS(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle);
bool processAPPS1(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle);
bool processAPPS2(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle);
bool processSteeringAngle(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle);
bool processWaterLevel(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle)
bool processHeavesFront(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle);
bool processHeavesRear(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle);
bool processRolls1(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle);
bool processRolls2(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle);
bool processBrakeFront(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle);
bool processBrakeRear(const uint8_t CANRxData[8], const uint32_t DataLength, const CANData *datahandle);

CANData BPPS = {&DeviceState.AnalogNode1, BPPS_ID, 2, processFront_brake, ANodeCritTimeout, 2500};
CANData APPS1 = {&DeviceState.AnalogNode1, APPS1_ID, 2, processAPPS1, ANodeCritTimeout, 2500};
CANData APPS2 = {&DeviceState.AnalogNode1, APPS2_ID, 2, processAPPS2, ANodeCritTimeout, 2500};
CANData SteeringAngle = {&DeviceState.AnalogNode1, SteeringAngle_ID, 2, processSteeringAngle, ANodeCritTimeout, 2500};
CANData WaterLevel = {&DeviceState.AnalogNode1, WaterLevel_ID, 2, processWaterLevel, ANodeCritTimeout, 2500};
CANData HeavesFront = {&DeviceState.AnalogNode1, HeavesFront_ID, 2, processHeavesFront, ANodeCritTimeout, 2500};
CANData HeavesRear = {&DeviceState.AnalogNode2, HeavesRear_ID, 2, processHeavesRear, ANodeCritTimeout, 2500};
CANData Rolls1 = {&DeviceState.AnalogNode1, Rolls1_ID, 2, processRolls1, ANodeCritTimeout, 2500};
CANData Rolls2 = {&DeviceState.AnalogNode2, Rolls2_ID, 2, processRolls2, ANodeCritTimeout, 2500};
CANData BrakeFront = {&DeviceState.AnalogNode1, BrakeFront_ID, 2, processBrakeFront, ANodeCritTimeout, 2500};
CANData BrakeRear = {&DeviceState.AnalogNode2, BrakeRear_ID, 2, processBrakeRear, ANodeCritTimeout, 2500};   


int initNodeDevices( void ){

    RegisterCANData(&BPPS);
    RegisterCANData(&APPS1);
    RegisterCANData(&APPS2);
    RegisterCANData(&SteeringAngle);
    RegisterCANData(&WaterLevel);
    RegisterCANData(&HeavesFront);
    RegisterCANData(&HeavesRear);
    RegisterCANData(&Rolls1);
    RegisterCANData(&Rolls2);
    RegisterCANData(&BrakeFront);
    RegisterCANData(&BrakeRear);
    
    //TODO add more devices
    return 0;

}