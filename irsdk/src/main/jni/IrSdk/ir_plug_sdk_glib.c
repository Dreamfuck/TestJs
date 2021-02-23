#include "ir_plug_sdk_glib.h"

#include <string.h>


#define LINE(x) ((x) * (((IR_PLUG_SDK_GLIB_PixelFont *)IR_PLUG_SDK_GLIB_GetFont())->Height))


/* 绘图缓冲区，将直接在该缓冲区上写数据，不检查合法性               */
static U32 * g_DrawBuffer = 0;

static IR_PLUG_SDK_GLIB_PixelFont * g_GlibFont = &Font12;
static U32 g_GlibTextColor      = 0xFFFFFFFF;
static U32 g_BackgroundColor    = 0xFF000000;


static void DrawChar(U16 col, U16 row, const U08* c);
static void DrawPixel(U16 row, U16 col, U32 color);

IR_PLUG_SDK_GLIB_PixelFont * IR_PLUG_SDK_GLIB_GetFont(void)
{
    return g_GlibFont;
}

void IR_PLUG_SDK_GLIB_SetDrawBuffer(U32 * pBuffer)
{
    g_DrawBuffer = pBuffer;
}

void IR_PLUG_SDK_GLIB_SetTextColor(U32 color)
{
    g_GlibTextColor = color;
}

void IR_PLUG_SDK_GLIB_DrawString(U16 row, U16 col, const U08* str)
{
    while (*str != 0)
    {
        IR_PLUG_SDK_GLIB_DrawChar(row, col, *str);
        col += g_GlibFont->Width;
        str++;
    }
}

void IR_PLUG_SDK_GLIB_DrawStringAtLine(U08 line, const U08* str)
{
    IR_PLUG_SDK_GLIB_DrawString(LINE(line), 0, str);
}

void IR_PLUG_SDK_GLIB_DrawChar(U16 row, U16 col, U08 ascii)
{
    DrawChar(col, row, &g_GlibFont->table[(ascii - ' ') *
        g_GlibFont->Height * ((g_GlibFont->Width + 7) / 8)]);
}

void IR_PLUG_SDK_GLIB_FillRect(U16 row, U16 col, U16 width, U16 height)
{
    U32 * pPos = 0;
    for (int i = 0; i < height; ++i)
    {
        pPos = g_DrawBuffer + (row + i) * 160 + col;
        for (int j = 0; j < width; ++j)
        {
            *pPos++ = 0xFFFF00FF;
        }
    }
}



// 十字光标颜色索引 127 ，对应颜色值为 FCFC54
#define CROSSHAIR_COLOR             (0xFFFFFF00)

void IR_PLUG_SDK_GLIB_DrawCrosshair(void)
{
    U16 row = 60;
    U16 col = 80;

    // left
    DrawPixel(row, col - 4, CROSSHAIR_COLOR);
    DrawPixel(row, col - 3, CROSSHAIR_COLOR);
    DrawPixel(row, col - 2, CROSSHAIR_COLOR);
    DrawPixel(row, col - 1, CROSSHAIR_COLOR);

    // top
    DrawPixel(row - 4, col, CROSSHAIR_COLOR);
    DrawPixel(row - 3, col, CROSSHAIR_COLOR);
    DrawPixel(row - 2, col, CROSSHAIR_COLOR);
    DrawPixel(row - 1, col, CROSSHAIR_COLOR);

    // right
    DrawPixel(row, col + 4, CROSSHAIR_COLOR);
    DrawPixel(row, col + 3, CROSSHAIR_COLOR);
    DrawPixel(row, col + 2, CROSSHAIR_COLOR);
    DrawPixel(row, col + 1, CROSSHAIR_COLOR);

    // bottom
    DrawPixel(row + 4, col, CROSSHAIR_COLOR);
    DrawPixel(row + 3, col, CROSSHAIR_COLOR);
    DrawPixel(row + 2, col, CROSSHAIR_COLOR);
    DrawPixel(row + 1, col, CROSSHAIR_COLOR);
}


void DrawChar(U16 col, U16 row, const U08* c)
{
    U32     i = 0, j = 0;
    U16     height, width;
    U08     offset;
    U08*    pchar;
    U32     line;

    height = g_GlibFont->Height;
    width  = g_GlibFont->Width;

    offset =  8 * ((width + 7) / 8) -  width ;

    for (i = 0; i < height; i++)
    {
        pchar = ((U08*)c + (width + 7) / 8 * i);

        switch (((width + 7) / 8))
        {
            case 1:
                line =  pchar[0];
            break;

            case 2:
                line = (pchar[0] << 8) | pchar[1];
            break;

            case 3:
            default:
                line = (pchar[0] << 16) | (pchar[1] << 8) | pchar[2];
            break;
        }

        for (j = 0; j < width; j++)
        {
            if (line & (1 << (width - j + offset - 1)))
            {
                DrawPixel((col + j), row, g_GlibTextColor);
            }
            else
            {
                //DrawPixel((col + j), row, 0x0);
            }
        }

        row++;
    }
}


static void DrawPixel(U16 row, U16 col, U32 color)
{
    if (0 == g_DrawBuffer) return;

    *(VU32*)(g_DrawBuffer + ((col * 120 + row))) = color;
//     *(VU32*)(g_DrawBuffer + ((row * 160 + col))) = color;
}


