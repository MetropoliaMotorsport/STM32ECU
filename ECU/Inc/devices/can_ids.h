
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

//Needs to be change due to new hardware

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
#define CRT_ID                  (0x0A3) //"Critical Error"
#define RDTS_ID                 (0x0A4) //"Ready to enable TS"
#define TSR_ID                  (0x0A5) //"TS Activation requested whilst ready."
#define TSSNR_ID                (0x0A6) //"TS Activation requested whilst not ready."
#define ESS_ID                  (0x0A7) //"Entering Startup State"
#define BMSF_ID                 (0x0A8) //"BMS Fail"
#define IVTF_ID                 (0x0A9) //"IVT Fail"
#define RPWF_ID                 (0x0AA) //"Readyness Power fail"
#define ERCS_ID                 (0x0AB) //"Entering Readyness check State"
#define ERRTL_ID                (0x0AC) //"Errorplace 0xBA Too many loops."
#define EPOS_ID                 (0x0AD) //"Entering PreOperational State"
#define PWRR_ID                 (0x0AE) //"Power requested"
#define PWRINV_ID               (0x0AF) //"Power off to inverters requested."
#define STT_ID                  (0x0B0) //"Start pressedr"  
#define TSP_ID                  (0x0B1) //"TS pressed"
#define RDTMP_ID                (0x0B2) //"RDTM pressed"
#define ERDTM_ID                (0x0B3) //"Entering RTDM State"
#define RDTMTA_ID               (0x0B4) //"RTDM:TA"
#define DISRN_ID                (0x0B5) //"Disabling regen"
#define ETSAS_ID                (0x0B6) //"Entering TS Active State"
#define TSLOV                   (0x0B7) //"TS:LoV"
#define TSON                    (0x0B8) //"TS:On"
#define RTDMAC_ID               (0x0B9) //"RTDM activation attempt with no braking"
#define WFP_ID                  (0x0BA) //"Waiting for precharge"
#define RTIS_ID                 (0x0BB) //"Returning to idle state at request."

////////////////////////////////////////
#endif
