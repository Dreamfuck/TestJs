#ifndef __IR_PLUG_SDK_COLORMAP_H__
#define __IR_PLUG_SDK_COLORMAP_H__


#include "ir_platform.h"
#include "ir_plug_sdk.h"



#define R_OF_RGB565(rgb565)     ((((rgb565) & 0xF800) >> 11) << 3)
#define G_OF_RGB565(rgb565)     ((((rgb565) & 0x07E0) >>  5) << 2)
#define B_OF_RGB565(rgb565)     ((((rgb565) & 0x001F) >>  0) << 3)

#define R_OF_RGB888(rgb888)     (((rgb888) >> 16) & 0xFF)
#define G_OF_RGB888(rgb888)     (((rgb888) >>  8) & 0xFF)
#define B_OF_RGB888(rgb888)     (((rgb888) >>  0) & 0xFF)

#define RGB565_2_RGB888(rgb565) (((R_OF_RGB565(rgb565) << 16) | \
    (G_OF_RGB565(rgb565) << 8)) | \
    (B_OF_RGB565(rgb565) << 0))

#define RGB888_2_RGB565(rgb888) ((R_OF_RGB888(rgb888) >> 3) << 11) | \
    ((G_OF_RGB888(rgb888) >> 2) << 5) |\
    ((B_OF_RGB888(rgb888) >> 3) << 0)



#define ColorMap_Color_RGB565(x)   (IR_COLORMAP_RGB565[(x)])
#define ColorMap_Color_RGB888(x)   (IR_COLORMAP_RGB888[(x)])

extern const U16 * IR_COLORMAP_RGB565;
extern const U32 * IR_COLORMAP_RGB888;



/* RGB565 调色板声明                                                            */
extern const U16 ColorMap_HotSpot_RGB565[];
extern const U16 ColorMap_Rainbow_RGB565[];
extern const U16 ColorMap_Gray_RGB565[];
extern const U16 ColorMap_HotMetal_RGB565[];
extern const U16 ColorMap_ColdSpot_RGB565[];


/* RGB888 调色板声明                                                            */
extern const U32 ColorMap_HotSpot_RGB888[];
extern const U32 ColorMap_Rainbow_RGB888[];
extern const U32 ColorMap_Gray_RGB888[];
extern const U32 ColorMap_HotMetal_RGB888[];
extern const U32 ColorMap_ColdSpot_RGB888[];
extern const U32 ColorMap_Human_RGB888[];



void IR_COLORMAP_Set(IR_PLUG_SDK_ColormapTypeDef type);


#endif



