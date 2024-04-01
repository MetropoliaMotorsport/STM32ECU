
#include "ecumain.h"
#include "errors.h"
#include "canecu.h"
#include "bms.h"
#include "output.h"
#include "power.h"
#include "debug.h"
#include "node_devices.h"

bool processFront_brake( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processRear_brake( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );
bool processBreak_pressure( const uint8_t CANRxData[8], const uint32_t DataLength, const CANData * datahandle );

CANData Front_brake = {&DeviceState.AnalogNode1, FrontBreak_ID, 2, processFront_brake, ANodeCritTimeout, 2500};
CANData Rear_brake = {&DeviceState.AnalogNode1, RearBreak_ID, 2, processRear_brake, ANodeCritTimeout, 2500};q
CANData Break_pressure = {&DeviceState.AnalogNode1, BreakPressure_ID, 2, processBreak_pressure, ANodeCritTimeout, 2500};

int initNodeDevices( void ){
    RegisterCan2Message( &Front_brake );
    RegisterCan2Message( &Rear_brake );
    RegisterCan2Message( &Break_pressure );
    //TODO add more devices

}