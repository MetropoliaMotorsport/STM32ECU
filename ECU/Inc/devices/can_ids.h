
#ifndef CAN_IDS_H
#define CAN_IDS_H

#define BMS_ID					0x008
#define BMSVOLT_ID				BMS_ID+3 // 0xB is voltage.
#define BMSSOC_ID				0x97
#define FLSpeed_COBID			0x71 // 112 // 0x70 orig
#define FRSpeed_COBID			0x70 // 113 // 0x71 orig

#define PDM_ID					0x520
#define MEMORATOR_ID			0x07B

#define NodeErr_ID        		0x600
#define NodeCmd_ID				0x602
#define NodeAck_ID				0x601

#define AdcSimInput_ID			0x608

#define APPS_ID					0x610
#define Brake_ID				0x620

#define AnalogNode1_ID			(0x680) // (1664)
#define AnalogNode2_ID			(0x690) // (1680)

////////////////////////////////////////


#define COBERR_ID				(0x080)
#define COBNMT_ID				(0x700)

#define COBTPDO1_ID				(0x180)
#define COBTPDO2_ID				(0x280)
#define COBTPDO3_ID				(0x380)
#define COBTPDO4_ID				(0x480)

#define COBRPDO1_ID				(0x200)
#define COBRPDO2_ID				(0x300)
#define COBRPDO3_ID				(0x400)
#define COBRPDO4_ID				(0x500)
#define COBRPDO5_ID				(0x540)

#define COBSDOS_ID				(0x600)

#define ECU_CAN_ID				(0x020) // send +1

//Debug Msgs
////////////////////////////////////////
#define EIS_ID					(0x0A0) //"Entering Idle State"
#define TSO_ID					(0x0A1) //"TS:Off"
#define TSB_ID                  (0x0A2) //"TS:BAD"
////////////////////////////////////////
#endif
