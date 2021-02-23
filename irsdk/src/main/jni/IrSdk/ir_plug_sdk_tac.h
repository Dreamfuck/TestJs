#ifndef __IR_PLUG_SDK_TAC_H__
#define __IR_PLUG_SDK_TAC_H__

#include "ir_platform.h"

extern S32 AccuracyCalibrate();
extern U16 AcCalGrant(F32 GetTemp, F32 LowLimit, F32 HighLimit);
extern S32 AcCalCalc();

#endif