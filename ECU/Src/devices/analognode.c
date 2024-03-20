#include "analognode.h"
#include "ecumain.h"
#include "canecu.h"
#include "queue.h"
#include "can_ids.h"



bool processANode1Data(const uint8_t CANRxData[8], const uint32_t DataLength,
		const CANData *datahandle);

bool processANode2Data(const uint8_t CANRxData[8], const uint32_t DataLength,
		const CANData *datahandle);


CANData AnalogNode1 = { &DeviceState.AnalogNode1, AnalogNode1_ID, 2,
		processANode1Data, ANodeCritTimeout, NODECRITICALTIMEOUT };
CANData AnalogNode2 = { &DeviceState.AnalogNode2, AnalogNode2_ID, 2,
		processANodeData, NULL, NODETIMEOUT };