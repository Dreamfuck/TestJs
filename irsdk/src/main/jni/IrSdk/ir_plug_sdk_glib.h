#ifndef __IR_PLUG_SDK_GLIB_H__
#define __IR_PLUG_SDK_GLIB_H__

#include "ir_platform.h"
#include "ir_plug_sdk_font.h"


void IR_PLUG_SDK_GLIB_SetDrawBuffer(U32 * pBuffer);
void IR_PLUG_SDK_GLIB_SetTextColor(U32 color);
void IR_PLUG_SDK_GLIB_DrawString(U16 row, U16 col, const U08 * str);
void IR_PLUG_SDK_GLIB_DrawStringAtLine(U08 line, const U08* str);
void IR_PLUG_SDK_GLIB_DrawChar(U16 row, U16 col, U08 ascii);
void IR_PLUG_SDK_GLIB_FillRect(U16 row, U16 col, U16 width, U16 height);
void IR_PLUG_SDK_GLIB_DrawCrosshair(void);



#endif
