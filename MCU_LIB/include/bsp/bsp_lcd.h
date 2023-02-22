/**
 *
 * @ 名称: bsp_lcd.h
 * @ 描述: TODO
 * @ 作者: lxw
 * @ 日期: Jan 8, 2023
 * @ 版本: V1.0
 *
 * Copyright (c) 2022. All rights reserved.
 */
#ifndef INCLUDE_BSP_BSP_LCD_H_
#define INCLUDE_BSP_BSP_LCD_H_

#include "core/common_defs.h"

#define LCD_DEBUG(x, y, fmt, args...)      bsp_lcd_show_str(x, y, BLUE, fmt, ##args)

#define BACKGROUND      BLACK

#define WHITE                0xFFFF                    // 白色
#define BLACK        0x0000                    // 黑色
#define GREY         0xF7DE                    // 灰色
#define BLUE         0x001F                    // 蓝色
#define BLUE2        0x051F                    // 浅蓝色
#define RED          0xF800                    // 红色
#define MAGENTA      0xF81F                    // 红紫色，洋红色
#define GREEN        0x07E0                    // 绿色
#define CYAN         0x7FFF                    // 蓝绿色，青色
#define YELLOW       0xFFE0                    // 黄色

#define WIDTHBYTES(bits) (((bits)+31)/32*4)     //对于24位真彩色 每一行的像素宽度必须是4的倍数  否则补0补齐

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;

typedef enum {
    LCD_FONT_8 = 8,
    LCD_FONT_12 = 12,
    LCD_FONT_16 = 16,
    LCD_FONT_20 = 20,
    LCD_FONT_24 = 24,
} lcd_font_weights_t;

enum lcd_direction {
    LCD_SCAN_LEFT_UP_X_Y = 6,
    LCD_SCAN_LEFT_UP_Y_X = 7,
    LCD_SCAN_RIGHT_UP_X_Y = 4,
    LCD_SCAN_RIGHT_UP_Y_X = 5,
    LCD_SCAN_LEFT_BOTTOM_X_Y = 2,
    LCD_SCAN_LEFT_BOTTOM_Y_X = 3,
    LCD_SCAN_RIGHT_BOTTOM_X_Y = 0,
    LCD_SCAN_RIGHT_BOTTOM_Y_X = 1,
    LCD_SCAN_NORMAL = LCD_SCAN_LEFT_UP_X_Y,
    LCD_SCAN_PORTRAIT = LCD_SCAN_LEFT_UP_X_Y,
    LCD_SCAN_INPORTRAIT = LCD_SCAN_RIGHT_BOTTOM_X_Y,
    LCD_SCAN_LANDSCAPE = LCD_SCAN_LEFT_BOTTOM_Y_X,
    LCD_SCAN_INLANDSCAPE = LCD_SCAN_RIGHT_UP_Y_X,
};

typedef enum lcd_direction lcd_direction_t;

//位图文件头信息结构定义
//其中不包含文件类型信息（由于结构体的内存结构决定，要是加了的话将不能正确读取文件信息）
typedef struct tagBITMAPFILEHEADER
{
    DWORD bfSize;                                                           //文件大小                                                                              4
    WORD bfReserved1;                                                   //保留字，不考虑                                                                   2
    WORD bfReserved2;                                                   //保留字，同上                                                                        2
    DWORD bfOffBits;                                                        //实际位图数据的偏移字节数，即前三个部分长度之和   4
} BITMAPFILEHEADER,tagBITMAPFILEHEADER;


typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;                                                       //指定此结构体的长度，为40                                                     4
    LONG biWidth;                                                       //位图宽                                                                                           4
    LONG biHeight;                                                      //位图高                                                                                           4
    WORD biPlanes;                                                      //平面数，为1                                                                                    2
    WORD biBitCount;                                                    //采用颜色位数，可以是1，2，4，8，16，24新的可以是32    2
    DWORD biCompression;                                            //压缩方式，可以是0，1，2，其中0表示不压缩                        4
    DWORD biSizeImage;                                              //实际位图数据占用的字节数                                                      4
    LONG biXPelsPerMeter;                                       //X方向分辨率                                                                                    4
    LONG biYPelsPerMeter;                                       //Y方向分辨率                                                                                    4
    DWORD biClrUsed;                                                    //使用的颜色数，如果为0，则表示默认值(2^颜色位数)            4
    DWORD biClrImportant;                                       //重要颜色数，如果为0，则表示所有颜色都是重要的               4
} BITMAPINFOHEADER,tagBITMAPINFOHEADER;


typedef struct tagRGBQUAD {
    BYTE rgbBlue;                                                   //该颜色的蓝色分量
    BYTE rgbGreen;                                                  //该颜色的绿色分量
    BYTE rgbRed;                                                        //该颜色的红色分量
    BYTE rgbReserved;                                               //保留值
} RGBQUAD,tagRGBQUAD;


typedef struct RGB_PIXEL {      //像素的数据类型
    unsigned char   rgbBlue;
    unsigned char   rgbGreen;
    unsigned char   rgbRed;
}RGB_PIXEL;

void bsp_lcd_init();
void bsp_lcd_clear(uint16_t color);
void bsp_lcd_show_char(uint16_t x, uint16_t y, uint8_t ascii, uint16_t color);
void bsp_lcd_show_chinese(uint16_t x, uint16_t y, uint16_t ch, uint16_t color);
void bsp_lcd_show_str(uint16_t x, uint16_t y, uint16_t color, const char* fmt, ...);
void bsp_lcd_show_num(uint16_t x, uint16_t y, int32_t num, uint16_t color);
void bsp_lcd_set_point(uint16_t x , uint16_t y , uint16_t color);
uint16_t bsp_lcd_get_point(uint16_t x , uint16_t y);
void bsp_lcd_set_direction(lcd_direction_t direction);
void bsp_lcd_save_fonts();
void bsp_lcd_set_fonts(int source, lcd_font_weights_t weight);
void bsp_lcd_show_bmp_image(uint16_t x, uint16_t y, char *pic_name);
void bsp_lcd_show_jpg_image(uint16_t x, uint16_t y, char *pic_name);
void bsp_lcd_show_qrcode_image(uint16_t x, uint16_t y, char *content, uint8_t version);

#endif /* INCLUDE_BSP_BSP_LCD_H_ */
