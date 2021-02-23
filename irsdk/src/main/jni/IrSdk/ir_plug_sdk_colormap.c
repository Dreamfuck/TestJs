#include "ir_plug_sdk_colormap.h"



static const U16 * g_IrColormapListRGB565[IR_PLUG_SDK_COLORMAP_INVALID] =
{
    ColorMap_HotMetal_RGB565,
    ColorMap_Rainbow_RGB565,
    ColorMap_Gray_RGB565,
    ColorMap_HotSpot_RGB565,
    ColorMap_ColdSpot_RGB565
};


static const U32 * g_IrColormapListRGB888[IR_PLUG_SDK_COLORMAP_INVALID] =
{
    ColorMap_HotMetal_RGB888,
    ColorMap_Gray_RGB888,
    ColorMap_Rainbow_RGB888,
    ColorMap_HotSpot_RGB888,
    ColorMap_ColdSpot_RGB888
};


const U16 * IR_COLORMAP_RGB565 = ColorMap_HotMetal_RGB565;
const U32 * IR_COLORMAP_RGB888 = ColorMap_Human_RGB888;


static void SetColormapRGB565(IR_PLUG_SDK_ColormapTypeDef type);
static void SetColormapRGB888(IR_PLUG_SDK_ColormapTypeDef type);


void IR_COLORMAP_Set(IR_PLUG_SDK_ColormapTypeDef type)
{
    SetColormapRGB565(type);
    SetColormapRGB888(type);
}



void SetColormapRGB565(IR_PLUG_SDK_ColormapTypeDef type)
{
    if (IR_PLUG_SDK_COLORMAP_INVALID != type)
    {
        IR_COLORMAP_RGB565 = g_IrColormapListRGB565[type];
    }
    else
    {
        IR_COLORMAP_RGB565 = g_IrColormapListRGB565[0];
    }
}

void SetColormapRGB888(IR_PLUG_SDK_ColormapTypeDef type)
{
    if (IR_PLUG_SDK_COLORMAP_INVALID != type)
    {
        IR_COLORMAP_RGB888 = g_IrColormapListRGB888[type];
    }
    else
    {
        IR_COLORMAP_RGB888 = g_IrColormapListRGB888[0];
    }
}




