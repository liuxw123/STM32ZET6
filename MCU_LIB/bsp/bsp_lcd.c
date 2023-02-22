/**
 *
 * @ 闁告艾绉惰ⅷ: bsp_lcd.c
 * @ 闁硅绻楅崼锟�: TODO
 * @ 濞达絾绮忛敓鏂ゆ嫹: lxw
 * @ 闁哄啨鍎插﹢锟�: Jan 8, 2023
 * @ 闁绘鐗婂﹢锟�: V1.0
 *
 * Copyright (c) 2022. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "bsp/bsp_lcd.h"
#include "bsp/ascii.h"
#include "user/logcat.h"
#include "fonts.h"
#include "user/fatfs_file.h"
#include "tjpgd.h"
#include "qrcode.h"

#define LOG_TAG                     "bsp_lcd"

#define WIDTH           240
#define HEIGHT          320

#define Bank1_LCD_C    ((uint32_t)0x6C000000)         //Disp Reg ADDR
#define Bank1_LCD_D    ((uint32_t)0x6C000800)         //Disp Data ADDR

//闂侇偄顦悾缍D闁圭娲ら悾鍓э拷闈涘閻°劑宕抽敓锟�
#define LCD_WR_REG(index)      ((*(__IO uint16_t *) (Bank1_LCD_C)) = ((uint16_t)index))
//鐎靛府鎷稬CD GRAM闁告劖鐟ラ崣鍡涘极閻楀牆绁�
#define LCD_WR_Data(val)       ((*(__IO uint16_t *) (Bank1_LCD_D)) = ((uint16_t)(val)))

#define LCD_ILI9341_CMD(index)       LCD_WR_REG(index)
#define LCD_ILI9341_Parameter(val)   LCD_WR_Data(val)

#define RGB24TORGB16(R,G,B) ((unsigned short int)((((R)>>3)<<11) | (((G)>>2)<<5)    | ((B)>>3)))


#define LCD_FONTS_SOURCE_RAM            0
#define LCD_FONTS_SOURCE_SD             1
#define LCD_FONT_HZ_WIDTH               16
#define LCD_FONT_HZ_HEIGHT              16

static sFONT* lcd_font;
static uint8_t lcd_font_source = LCD_FONTS_SOURCE_RAM; //

static char LCD_FONT_SD8[] = "/fonts/font8.bin";
static char LCD_FONT_SD12[] = "/fonts/font12.bin";
static char LCD_FONT_SD16[] = "/fonts/font16.bin";
static char LCD_FONT_SD20[] = "/fonts/font20.bin";
static char LCD_FONT_SD24[] = "/fonts/font24.bin";
static char LCD_FONT_SDHZ[] = "/fonts/hzlib.bin";
static char *lcd_font_sd;
static lcd_font_weights_t lcd_font_weight = LCD_FONT_16;
static uint8_t lcd_font_width;
static uint8_t lcd_font_height;

static uint32_t LCD_WIDTH = WIDTH;
static uint32_t LCD_HEIGHT = HEIGHT;
static uint8_t lcd_scan_show_reverse = 0;
static lcd_direction_t LCD_DIRECTION = LCD_SCAN_NORMAL;

static void __bsp_lcd_set_direction(lcd_direction_t direction, uint8_t updated);
static void __bsp_lcd_set_lcd_gram(uint8_t dir, uint16_t xs, uint16_t ys);

static void __msp_fsmc_init() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_FSMC_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_14
                          |GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4
                          |GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

static void __msp_fsmc_deinit() {
    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_0|GPIO_PIN_12);

     HAL_GPIO_DeInit(GPIOE, GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10
                             |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                             |GPIO_PIN_15);

     HAL_GPIO_DeInit(GPIOD, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_14
                             |GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4
                             |GPIO_PIN_5);

    __HAL_RCC_FSMC_CLK_DISABLE();
    __HAL_RCC_GPIOG_CLK_DISABLE();
    __HAL_RCC_GPIOE_CLK_DISABLE();
    __HAL_RCC_GPIOD_CLK_DISABLE();
}

static void __lcd_gpio_config() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOD_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

static void __lcd_fsmc_config() {
    FSMC_NORSRAM_TimingTypeDef Timing = {0};
    SRAM_HandleTypeDef hsram4 = {0};
    hsram4.Instance = FSMC_NORSRAM_DEVICE;
    hsram4.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
    /* hsram4.Init */
    hsram4.Init.NSBank = FSMC_NORSRAM_BANK4;
    hsram4.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
    hsram4.Init.MemoryType = FSMC_MEMORY_TYPE_NOR;
    hsram4.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
    hsram4.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
    hsram4.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
    hsram4.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
    hsram4.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
    hsram4.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
    hsram4.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
    hsram4.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
    hsram4.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
    hsram4.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;

    hsram4.MspInitCallback = __msp_fsmc_init;
    hsram4.MspDeInitCallback = __msp_fsmc_deinit;
    /* Timing */
    Timing.AddressSetupTime = 0x02;
    Timing.AddressHoldTime = 0x00;
    Timing.DataSetupTime = 0x05;
    Timing.BusTurnAroundDuration = 0x00;
    Timing.CLKDivision = 0x00;
    Timing.DataLatency = 0x00;
    Timing.AccessMode = FSMC_ACCESS_MODE_B;
    /* ExtTiming */

    if (HAL_SRAM_Init(&hsram4, &Timing, NULL) != HAL_OK) {
    }
    __HAL_AFIO_FSMCNADV_DISCONNECTED();
}

static void __lcd_reset() {
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET);
    HAL_Delay(30);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);
    HAL_Delay(30);
}

static void __debug_delay(int num) {
    for (int i=0;i<num;i++)
        for (int j=0;j<num;j++);
}

static void __lcd_black_led(int on) {
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, on?GPIO_PIN_RESET:GPIO_PIN_SET);
}

static void __lcd_reg_config(void) {
    /*  Power control B (CFh)  */
    __debug_delay(8);
    LCD_ILI9341_CMD(0xCF);
    LCD_ILI9341_Parameter(0x00);
    LCD_ILI9341_Parameter(0x81);
    LCD_ILI9341_Parameter(0x30);

    /*  Power on sequence control (EDh) */
    __debug_delay(8);
    LCD_ILI9341_CMD(0xED);
    LCD_ILI9341_Parameter(0x64);
    LCD_ILI9341_Parameter(0x03);
    LCD_ILI9341_Parameter(0x12);
    LCD_ILI9341_Parameter(0x81);

    /*  Driver timing control A (E8h) */
    __debug_delay(8);
    LCD_ILI9341_CMD(0xE8);
    LCD_ILI9341_Parameter(0x85);
    LCD_ILI9341_Parameter(0x10);
    LCD_ILI9341_Parameter(0x78);

    /*  Power control A (CBh) */
    __debug_delay(8);
    LCD_ILI9341_CMD(0xCB);
    LCD_ILI9341_Parameter(0x39);
    LCD_ILI9341_Parameter(0x2C);
    LCD_ILI9341_Parameter(0x00);
    LCD_ILI9341_Parameter(0x34);
    LCD_ILI9341_Parameter(0x02);

    /* Pump ratio control (F7h) */
    __debug_delay(8);
    LCD_ILI9341_CMD(0xF7);
    LCD_ILI9341_Parameter(0x20);

    /* Driver timing control B */
    __debug_delay(8);
    LCD_ILI9341_CMD(0xEA);
    LCD_ILI9341_Parameter(0x00);
    LCD_ILI9341_Parameter(0x00);

    /* Frame Rate Control (In Normal Mode/Full Colors) (B1h) */
    __debug_delay(8);
    LCD_ILI9341_CMD(0xB1);
    LCD_ILI9341_Parameter(0x00);
    LCD_ILI9341_Parameter(0x1B);

    /*  Display Function Control (B6h) */
    __debug_delay(8);
    LCD_ILI9341_CMD(0xB6);
    LCD_ILI9341_Parameter(0x0A);
    LCD_ILI9341_Parameter(0xA2);

    /* Power Control 1 (C0h) */
    __debug_delay(8);
    LCD_ILI9341_CMD(0xC0);
    LCD_ILI9341_Parameter(0x35);

    /* Power Control 2 (C1h) */
    __debug_delay(8);
    LCD_ILI9341_CMD(0xC1);
    LCD_ILI9341_Parameter(0x11);

    /* VCOM Control 1(C5h) */
    LCD_ILI9341_CMD(0xC5);
    LCD_ILI9341_Parameter(0x45);
    LCD_ILI9341_Parameter(0x45);

    /*  VCOM Control 2(C7h)  */
    LCD_ILI9341_CMD(0xC7);
    LCD_ILI9341_Parameter(0xA2);

    /* Enable 3G (F2h) */
    LCD_ILI9341_CMD(0xF2);
    LCD_ILI9341_Parameter(0x00);

    /* Gamma Set (26h) */
    LCD_ILI9341_CMD(0x26);
    LCD_ILI9341_Parameter(0x01);
    __debug_delay(8);

    /* Positive Gamma Correction */
    LCD_ILI9341_CMD(0xE0); //Set Gamma
    LCD_ILI9341_Parameter(0x0F);
    LCD_ILI9341_Parameter(0x26);
    LCD_ILI9341_Parameter(0x24);
    LCD_ILI9341_Parameter(0x0B);
    LCD_ILI9341_Parameter(0x0E);
    LCD_ILI9341_Parameter(0x09);
    LCD_ILI9341_Parameter(0x54);
    LCD_ILI9341_Parameter(0xA8);
    LCD_ILI9341_Parameter(0x46);
    LCD_ILI9341_Parameter(0x0C);
    LCD_ILI9341_Parameter(0x17);
    LCD_ILI9341_Parameter(0x09);
    LCD_ILI9341_Parameter(0x0F);
    LCD_ILI9341_Parameter(0x07);
    LCD_ILI9341_Parameter(0x00);

    /* Negative Gamma Correction (E1h) */
    LCD_ILI9341_CMD(0XE1); //Set Gamma
    LCD_ILI9341_Parameter(0x00);
    LCD_ILI9341_Parameter(0x19);
    LCD_ILI9341_Parameter(0x1B);
    LCD_ILI9341_Parameter(0x04);
    LCD_ILI9341_Parameter(0x10);
    LCD_ILI9341_Parameter(0x07);
    LCD_ILI9341_Parameter(0x2A);
    LCD_ILI9341_Parameter(0x47);
    LCD_ILI9341_Parameter(0x39);
    LCD_ILI9341_Parameter(0x03);
    LCD_ILI9341_Parameter(0x06);
    LCD_ILI9341_Parameter(0x06);
    LCD_ILI9341_Parameter(0x30);
    LCD_ILI9341_Parameter(0x38);
    LCD_ILI9341_Parameter(0x0F);

    bsp_lcd_set_direction(LCD_SCAN_NORMAL);

    /*  Pixel Format Set (3Ah)  */
    __debug_delay(8);
    LCD_ILI9341_CMD(0x3a);
    LCD_ILI9341_Parameter(0x55);

    /* Sleep Out (11h)  */
    LCD_ILI9341_CMD(0x11);
    HAL_Delay(30);

    /* Display ON (29h) */
    LCD_ILI9341_CMD(0x29);
}

void __set_default_fonts() {
    bsp_lcd_set_fonts(LCD_FONTS_SOURCE_RAM, LCD_FONT_16);
}

void bsp_lcd_init() {
    __lcd_gpio_config();
    __lcd_fsmc_config();
    __lcd_reset();
    __lcd_reg_config();
    __lcd_black_led(1);
    __set_default_fonts();
    bsp_lcd_clear(BACKGROUND);
}

void bsp_lcd_clear(uint16_t color) {
    uint32_t i = 0;

    /* column address control set */
    LCD_ILI9341_CMD(0X2A);
    LCD_ILI9341_Parameter(0 >> 8);     /* 闁稿繐鐗撻悵锟�8濞达絽绋勭槐婵嬫倿鐠虹儤鍊靛ù锝忔嫹8濞达綇鎷� */
    LCD_ILI9341_Parameter(0 & 0xff);           /* column start   */
    LCD_ILI9341_Parameter((LCD_WIDTH-1) >> 8);   /* column end   */
    LCD_ILI9341_Parameter((LCD_WIDTH-1) & 0xff);

    /* page address control set */
    LCD_ILI9341_CMD(0X2B);
    LCD_ILI9341_Parameter(0 >> 8);            /* page start   */
    LCD_ILI9341_Parameter(0 & 0xff );
    LCD_ILI9341_Parameter((LCD_HEIGHT-1) >> 8);  /* page end     */
    LCD_ILI9341_Parameter((LCD_HEIGHT-1) & 0xff);

    /* memory write */
    LCD_ILI9341_CMD(0x2c);

    for( i=0; i < LCD_WIDTH*LCD_HEIGHT; i++ ) {
        LCD_WR_Data( color );
    }
}

void bsp_lcd_open_window(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
    LCD_ILI9341_CMD(0X2A);               /* 閻犱礁澧介悿鍝柛褎鍔栭悥锟� */
    LCD_ILI9341_Parameter( x >> 8 );     /* 闁稿繐鐗撻悵锟�8濞达絽绋勭槐婵嬫倿鐠虹儤鍊靛ù锝忔嫹8濞达綇鎷� */
    LCD_ILI9341_Parameter( x & 0xff );   /* 閻犱礁澧介悿鍡欐導瀹勯偊娼楅柣鎰嚀閹锋壆绱掗幘瀛樺皢闁绘劧鎷�*/
    LCD_ILI9341_Parameter( (x+width-1) >> 8 );
    LCD_ILI9341_Parameter( (x+width-1) & 0xff );

    LCD_ILI9341_CMD(0X2B);               /* 閻犱礁澧介悿鍝柛褎鍔栭悥锟�*/
    LCD_ILI9341_Parameter( y >> 8 );
    LCD_ILI9341_Parameter( y & 0xff );
    LCD_ILI9341_Parameter( (y+height-1) >> 8);
    LCD_ILI9341_Parameter( (y+height-1) & 0xff);

    LCD_ILI9341_CMD(0X2C);
}

static const uint8_t* __get_fonts_data_ascii(uint8_t ascii, int* total_byte, int* per_byte) {
    int w = lcd_font_width;
    int h = lcd_font_height;
    int w_b = w / 8 + ((w & 0x00000007) != 0);
    int total = h * w_b;
    int start = (ascii - ' ') * total;
    *total_byte = total;
    *per_byte = w_b;

    if (lcd_font_source == LCD_FONTS_SOURCE_RAM) return lcd_font->table+start;
    void* font_data = (void *)malloc(total);

    char path[128];
    f_full_path(lcd_font_sd, path, 128);
    FIL fil;
    FRESULT ret;
    ret = f_open(&fil, path, FA_READ);
    if (ret != FR_OK) goto error;
    ret = f_lseek(&fil, start);
    if (ret != FR_OK) goto close;
    unsigned int read;
    ret = f_read(&fil, font_data, total, &read);
    if (ret != FR_OK || read != total) goto close;
    f_close(&fil);
    return font_data;
close:
    f_close(&fil);
error:
    lcd_font_sd = NULL;
    __set_default_fonts();
    return __get_fonts_data_ascii(ascii, total_byte, per_byte);
}

static const uint8_t* __get_fonts_data_chinese(uint16_t ch, int* total_byte, int* per_byte) {
    int w = LCD_FONT_HZ_WIDTH;
    int h = LCD_FONT_HZ_HEIGHT;
    int w_b = w / 8 + ((w & 0x00000007) != 0);
    int total = h * w_b;
    uint8_t high = (uint8_t)((ch & 0xFF00) >> 8);
    uint8_t low = (uint8_t)ch;
    int start = ((high - 0xA0 -0x10) * 94 + low - 0xA1) * 32;
    *total_byte = total;
    *per_byte = w_b;

    void* font_data = (void *)malloc(total);
    if (!font_data) return NULL;
    char path[128];
    f_full_path(LCD_FONT_SDHZ, path, 128);
    FIL fil;
    FRESULT ret;
    ret = f_open(&fil, path, FA_READ);
    if (ret != FR_OK) goto error;
    ret = f_lseek(&fil, start);
    if (ret != FR_OK) goto close;
    unsigned int read;
    ret = f_read(&fil, font_data, total, &read);
    if (ret != FR_OK || read != total) goto close;
    f_close(&fil);
    return font_data;
close:
    f_close(&fil);
error:
    return NULL;
}


void __bsp_lcd_show(const uint8_t *font_data, int total_byte, int per_byte, uint8_t is_ch, uint16_t color) {
    for (int i=0;i<total_byte;i+=per_byte) {
        uint32_t val = 0x00000000;
        switch (per_byte) {
            case 4:
                val |= (font_data[i+3]);
                val |= (((uint32_t)font_data[i+2]) << 8) & 0x0000FF00;
                val |= (((uint32_t)font_data[i+1]) << 16) & 0x00FF0000;
                val |= (((uint32_t)font_data[i]) << 24) & 0xFF000000; break;
            case 3:
                val |= (font_data[i+2]);
                val |= (((uint32_t)font_data[i+1]) << 8) & 0x0000FF00;
                val |= (((uint32_t)font_data[i]) << 16) & 0x00FF0000; break;
            case 2:
                val |= (font_data[i+1]);
                val |= (((uint32_t)font_data[i]) << 8) & 0x0000FF00; break;
            case 1:
                val |= (font_data[i]); break;
            default: return;
        }

        int start_bits = per_byte * 8;
        int font_width = is_ch?LCD_FONT_HZ_WIDTH:lcd_font_width;
        if (lcd_scan_show_reverse) {
            for (int j=start_bits-font_width;j<start_bits;j++) {
                if(val & (0x00000001 << j)) LCD_WR_Data(color);
                else LCD_WR_Data(BACKGROUND);
            }
        } else {
            for (int j=start_bits;j>start_bits-font_width;j--) {
                if(val & (0x00000001 << (j-1))) LCD_WR_Data(color);
                else LCD_WR_Data(BACKGROUND);
            }
        }
    }
}

void bsp_lcd_show_char(uint16_t x, uint16_t y, uint8_t ascii, uint16_t color) {
    if (((int)ascii) - ' ' < 0) return;
    int total, w_b;
    const uint8_t *font_data = NULL;
    font_data = __get_fonts_data_ascii(ascii, &total, &w_b);
    bsp_lcd_open_window(x, y, lcd_font_width, lcd_font_height);
    __bsp_lcd_show(font_data, total, w_b, 0, color);
    if (lcd_font_source == LCD_FONTS_SOURCE_SD) free((void*)font_data);
}

void bsp_lcd_show_chinese(uint16_t x, uint16_t y, uint16_t ch, uint16_t color) {
    int total, w_b;
    const uint8_t *font_data = NULL;
    font_data = __get_fonts_data_chinese(ch, &total, &w_b);
    if (!font_data) return;
    bsp_lcd_open_window(x, y, LCD_FONT_HZ_WIDTH, LCD_FONT_HZ_HEIGHT);
    __bsp_lcd_show(font_data, total, w_b, 1, color);
    free((void*)font_data);
}

void bsp_lcd_show_str(uint16_t x, uint16_t y, uint16_t color, const char* fmt, ...) {
    int w = lcd_font_width;
    int h = lcd_font_height;
    char buf[256] = "";
    va_list args;
    va_start (args, fmt);
    vsprintf (buf, fmt, args);
    va_end (args);
    int length = strlen(buf);
    if (buf[0] == 196 && buf[1] == 250) debug_led_on(4);
    for (int i=0;i<length;i++) {
        if(x > (LCD_WIDTH-w)) {
            x = 0;
            y += h;
        }

        if(y > (LCD_HEIGHT-h)) {
            x = 0;
            y = 0;
        }
        bsp_lcd_show_char(x, y, buf[i], color);
        x += w;
    }
}

void bsp_lcd_show_num(uint16_t x, uint16_t y, int32_t num, uint16_t color) {
    int w = lcd_font_width;

    if (num < 0) {
        bsp_lcd_show_char(x, y, '-', color);
        bsp_lcd_show_num(x+w, y, -num, color);
        return;
    }
    uint32_t length = 0, temp = 0;
    temp = num;

    if(temp == 0) {
        bsp_lcd_show_char(x, y, '0', color);
        return;
    }

    while(temp) {
        temp /= 10;
        length ++;
    }

    while(num) {
        bsp_lcd_show_char((x+w*(length--)-w), y, (num%10)+'0', color);
        num /= 10;
    }
}

static void __bsp_lcd_set_cursor(uint16_t x, uint16_t y) {
    LCD_ILI9341_CMD(0X2A);               /* 閻犱礁澧介悿鍝柛褎鍔栭悥锟� */
    LCD_ILI9341_Parameter(x>>8);     /* 闁稿繐鐗撻悵锟�8濞达絽绋勭槐婵嬫倿鐠虹儤鍊靛ù锝忔嫹8濞达綇鎷� */
    LCD_ILI9341_Parameter(x&0xff);   /* 閻犱礁澧介悿鍡欐導瀹勯偊娼楅柣鎰嚀閹锋壆绱掗幘瀛樺皢闁绘劧鎷�*/
    LCD_ILI9341_Parameter(x>>8);
    LCD_ILI9341_Parameter(x&0xff);

    LCD_ILI9341_CMD(0X2B);               /* 閻犱礁澧介悿鍝柛褎鍔栭悥锟�*/
    LCD_ILI9341_Parameter(y>>8);
    LCD_ILI9341_Parameter(y&0xff);
    LCD_ILI9341_Parameter(y>>8);
    LCD_ILI9341_Parameter(y&0xff);
}

void bsp_lcd_set_point(uint16_t x , uint16_t y , uint16_t color) {
    __bsp_lcd_set_cursor(x, y);
    LCD_ILI9341_CMD(0x2c);           /* 闁告劖鐟﹂弳鐔煎箲閿燂拷 */
    LCD_WR_Data(color);
}


static uint16_t __bsp_lcd_read_data(void) {
    uint16_t R=0, G=0, B=0 ;

    R = *(__IO uint16_t *)Bank1_LCD_D;    /*FIRST READ OUT DUMMY DATA*/
    R = *(__IO uint16_t *)Bank1_LCD_D;      /*READ OUT RED DATA  */
    B = *(__IO uint16_t *)Bank1_LCD_D;      /*READ OUT BLACK DATA*/
    G = *(__IO uint16_t *)Bank1_LCD_D;      /*READ OUT GREEN DATA*/

    return (((R>>11)<<11) | ((G>>10)<<5) | (B>>11));
}

uint16_t bsp_lcd_get_point(uint16_t x , uint16_t y) {
    uint16_t temp = 0x0000;

    __bsp_lcd_set_cursor(x, y);
    LCD_ILI9341_CMD(0x2e);         /* 閻犲洨绮弳鐔煎箲閿燂拷 */
    temp=__bsp_lcd_read_data();
    return temp;
}

static void __bsp_lcd_set_lcd_gram(uint8_t dir, uint16_t xs, uint16_t ys) {
    __debug_delay(8);
    LCD_ILI9341_CMD(0x36);
    uint8_t data = (uint8_t)0x08 | (((uint8_t)dir) << 5);
    LCD_ILI9341_Parameter(data);
    __debug_delay(8);

    /* column address control set */
    LCD_ILI9341_CMD(0X2A);
    LCD_ILI9341_Parameter(0x00);
    LCD_ILI9341_Parameter(0x00);
    LCD_ILI9341_Parameter((uint8_t)((xs & 0xFF00) >> 8));
    LCD_ILI9341_Parameter((uint8_t)(xs & 0x00FF));

    /* page address control set */
    __debug_delay(8);
    LCD_ILI9341_CMD(0X2B);
    LCD_ILI9341_Parameter(0x00);
    LCD_ILI9341_Parameter(0x00);
    LCD_ILI9341_Parameter((uint8_t)((ys & 0xFF00) >> 8));
    LCD_ILI9341_Parameter((uint8_t)(ys & 0x00FF));
}

static void __bsp_lcd_set_direction(lcd_direction_t direction, uint8_t updated) {
    if (direction >= LCD_SCAN_RIGHT_BOTTOM_X_Y && direction <= LCD_SCAN_LEFT_UP_Y_X) {
        LCD_DIRECTION = direction;
        if (direction == LCD_SCAN_RIGHT_BOTTOM_Y_X ||
            direction == LCD_SCAN_LEFT_BOTTOM_Y_X ||
            direction == LCD_SCAN_RIGHT_UP_Y_X ||
            direction == LCD_SCAN_LEFT_UP_Y_X) {
            LCD_WIDTH = HEIGHT;
            LCD_HEIGHT = WIDTH;
        } else {
            LCD_WIDTH = WIDTH;
            LCD_HEIGHT = HEIGHT;
        }

        if (direction == LCD_SCAN_RIGHT_BOTTOM_Y_X ||
            direction == LCD_SCAN_LEFT_BOTTOM_X_Y ||
            direction == LCD_SCAN_RIGHT_UP_X_Y ||
            direction == LCD_SCAN_LEFT_UP_Y_X) {
            lcd_scan_show_reverse = 1;
        } else {
            lcd_scan_show_reverse = 0;
        }

        if (updated) __bsp_lcd_set_lcd_gram(LCD_DIRECTION, LCD_WIDTH, LCD_HEIGHT);
    }
}

void bsp_lcd_set_direction(lcd_direction_t direction) {
    __bsp_lcd_set_direction(direction, 1);
}

void bsp_lcd_set_fonts(int source, lcd_font_weights_t weight) {
    if (source == LCD_FONTS_SOURCE_RAM || source == LCD_FONTS_SOURCE_SD) {
        switch (weight) {
        case LCD_FONT_8: lcd_font_sd = LCD_FONT_SD8; lcd_font = &Font8; break;
        case LCD_FONT_12: lcd_font_sd = LCD_FONT_SD12; lcd_font = &Font12; break;
        case LCD_FONT_16: lcd_font_sd = LCD_FONT_SD16; lcd_font = &Font16; break;
        case LCD_FONT_20: lcd_font_sd = LCD_FONT_SD20; lcd_font = &Font20; break;
        case LCD_FONT_24: lcd_font_sd = LCD_FONT_SD24; lcd_font = &Font24; break;
        default:
            return;
        }
        lcd_font_width = lcd_font->Width;
        lcd_font_height = lcd_font->Height;

        if (source == LCD_FONTS_SOURCE_RAM) lcd_font_sd = NULL;
        else lcd_font = NULL;

        lcd_font_source = source;
        lcd_font_weight = weight;
    }
}

int __bsp_lcd_save_fonts(const char* path, const uint8_t* data, const int length) {
    FIL fil;
    FRESULT ret;
    ret = f_open(&fil, path, FA_WRITE | FA_CREATE_NEW);
    if (ret != FR_OK) return ret;
    unsigned int write = 0;
    ret = f_write(&fil, (const void *)data, length, &write);
    if (ret != FR_OK) {
        f_close(&fil);
        return ret;
    } else if (write != length) {
        f_close(&fil);
        return FR_DISK_ERR;
    } else return f_close(&fil);
}

void bsp_lcd_save_fonts() {
    char path[128];
    f_full_path(LCD_FONT_SD8, path, 128);
    if (!f_exist(path)) {
        __bsp_lcd_save_fonts(path, Font8.table, Font8.size);
    }

    f_full_path(LCD_FONT_SD12, path, 128);
    if (!f_exist(path)) {
        __bsp_lcd_save_fonts(path, Font12.table, Font12.size);
    }

    f_full_path(LCD_FONT_SD16, path, 128);
    if (!f_exist(path)) {
        __bsp_lcd_save_fonts(path, Font16.table, Font16.size);
    }

    f_full_path(LCD_FONT_SD20, path, 128);
    if (!f_exist(path)) {
        __bsp_lcd_save_fonts(path, Font20.table, Font20.size);
    }
    f_full_path(LCD_FONT_SD24, path, 128);
    if (!f_exist(path)) {
        __bsp_lcd_save_fonts(path, Font24.table, Font24.size);
    }

    bsp_lcd_set_fonts(LCD_FONTS_SOURCE_SD, LCD_FONT_20);
    LCD_DEBUG(0, 260, "save all fonts");
    bsp_lcd_show_chinese(30, 40, 0xC4FA, RED);
    bsp_lcd_show_str(46, 40, RED, "123");
}

static void __bsp_lcd_draw_line(uint16_t x, uint16_t y, uint16_t length, uint16_t width, uint16_t color) {
    bsp_lcd_open_window(x,y,length,width);
    for (uint16_t i=0;i<length*width;i++) {
        LCD_WR_Data(color);
    }
}

static void __bsp_lcd_draw_rect(uint16_t x, uint16_t y, uint16_t length, uint16_t width, uint8_t line_width, uint16_t color) {
    __bsp_lcd_draw_line(x, y, length, line_width, color);
    __bsp_lcd_draw_line(x, y+width-line_width, length, line_width, color);
    __bsp_lcd_draw_line(x, y, line_width, length, color);
    __bsp_lcd_draw_line(x+length-line_width, y, line_width, length, color);
}

void bsp_lcd_show_bmp_image(uint16_t x, uint16_t y, char *pic_name) {
    char path[128];
    int width, height, l_width;
    int i, j, k;
    FIL fil;
    FRESULT bmpres;
    WORD fileType;
    BYTE red,green,blue;
    BYTE pColorData[960];
    unsigned int read_num;
    BITMAPFILEHEADER bitHead;
    BITMAPINFOHEADER bitInfoHead;
    f_full_path(pic_name, path, 128);
    LCD_DEBUG(0, 0, path);
    if (!f_exist(path)) {
        return;
    }
    LCD_DEBUG(0, 20, "debug-1");
    bmpres = f_open(&fil, path, FA_OPEN_EXISTING | FA_READ);
    if (bmpres != FR_OK) return;

    /* 读取文件头信息  两个字节*/
    LCD_DEBUG(0, 40, "debug-2");
    f_read(&fil, &fileType,sizeof(WORD), &read_num);

    /* 判断是不是bmp文件 "BM"*/
    if(fileType != 0x4d42) {
        return;
    }
    LCD_DEBUG(0, 60, "debug-3");

    /* 读取BMP文件头信息*/
    f_read(&fil, &bitHead, sizeof(tagBITMAPFILEHEADER), &read_num);

    /* 读取位图信息头信息 */
    f_read(&fil, &bitInfoHead, sizeof(BITMAPINFOHEADER), &read_num);

    width = bitInfoHead.biWidth;
    height = bitInfoHead.biHeight;

    /* 计算位图的实际宽度并确保它为32的倍数  */
    l_width = WIDTHBYTES(width* bitInfoHead.biBitCount);

    if(l_width > 960)
    {
        return;
    }
    LCD_DEBUG(0, 80, "debug-%d:%d:%d:%d", width, height, bitInfoHead.biBitCount,l_width);
    /* 设置Lcd Gram 扫描方向为: 右下角->左上角 */
    __bsp_lcd_set_lcd_gram(LCD_SCAN_RIGHT_BOTTOM_Y_X, width, height);

    /* 开一个图片大小的窗口*/
    bsp_lcd_open_window(x, y, width, height);
    /* 判断是否是24bit真彩色图 */
    if(bitInfoHead.biBitCount >= 24) {
        debug_led_on(1);
        for(i=0;i< height; i++) {
            /* 读取一行bmp的数据到数组pColorData里面 */
            f_read(&fil, pColorData, l_width/2, &read_num);
            f_read(&fil, pColorData+l_width/2, l_width/2, &read_num);

            for(j=0; j<width; j++) {                                            //一行有效信息
                k = j*3;                                                                     //一行中第K个像素的起点
                red = pColorData[k+2];
                green = pColorData[k+1];
                blue =  pColorData[k];
                LCD_WR_Data(RGB24TORGB16(red, green, blue)); //写入LCD-GRAM
            }
        }
    } else {
        debug_led_on(2);
        return ;
    }
    f_close(&fil);
}


static unsigned int in_func(JDEC* jd, BYTE* buff, UINT nbyte) {
    unsigned int read_num = 0;
    FIL* fil = (FIL*)jd->device;
    if (buff) {
        f_read(fil, buff, nbyte, &read_num);
        return read_num;
    } else {
        return f_lseek(fil, f_tell(fil)+nbyte) == FR_OK ? nbyte : 0;
    }
}

static int out_func (JDEC* jd, void* bitmap, JRECT* rect) {
    uint16_t *src = (uint16_t*)bitmap;
    uint16_t x = rect->top;
    uint16_t y = rect->left;
    uint16_t w = rect->bottom-x+1;
    uint16_t h = rect->right-y+1;
    __bsp_lcd_set_lcd_gram(LCD_SCAN_LEFT_UP_X_Y, w, h);
    bsp_lcd_open_window(y,x,h,w);
    for (int i=0;i<w*h;i++) {
        uint16_t color = *(src+i);
        LCD_WR_Data(color);
    }
    return 1;
}

void bsp_lcd_show_jpg_image(uint16_t x, uint16_t y, char *pic_name) {
    uint8_t work[3100];
    JDEC jdec;
    FIL fil;
    char path[128];
    FRESULT jpgres;
    JRESULT jres;

    f_full_path(pic_name, path, 128);
    LCD_DEBUG(0, 0, path);
    if (!f_exist(path)) {
        return;
    }
    LCD_DEBUG(0, 20, "debug-1");
    jpgres = f_open(&fil, path, FA_OPEN_EXISTING | FA_READ);
    if (jpgres != FR_OK) return;
    LCD_DEBUG(0, 40, "debug-2");

    jres = jd_prepare(&jdec, in_func, work, 3100, &fil);
    LCD_DEBUG(0, 60, "debug-3:%d", jres);
    if (jres != JDR_OK) return;
    LCD_DEBUG(0, 60, "%ux%u(%u)", jdec.width, jdec.height, 3100 - jdec.sz_pool);

    int scale = 0;
    int wb = 0, hb = 0;
    if (jdec.width > jdec.height) {
        wb = jdec.width / HEIGHT;
        if ((jdec.width % HEIGHT) != 0) wb++;
        hb = jdec.height / WIDTH;
        if ((jdec.height % WIDTH) != 0) hb++;
    } else {
        wb = jdec.width / WIDTH;
        if ((jdec.width % WIDTH) != 0) wb++;
        hb = jdec.height / HEIGHT;
        if ((jdec.height % HEIGHT) != 0) hb++;
    }

    scale = (wb > hb)?wb:hb;
    if (scale > 8) {
        LCD_DEBUG(0, 200, "image too large. %d", scale);
        return;
    }

    if (scale > 4) scale = 3;
    else if (scale > 2) scale = 2;
    else if (scale > 1) scale = 1;
    else scale = 0;
    LCD_DEBUG(0, 200, "scale: %d", scale);
    jres = jd_decomp(&jdec, out_func, scale);
}

void bsp_lcd_show_qrcode_image(uint16_t x, uint16_t y, char *content, uint8_t version) {
    QRCode qrcode = {0};
    uint8_t line_width = 4;
    uint8_t* qrcodeBytes = (uint8_t *)malloc(QRCODE_BUFF_SIZE(version));
    if (!qrcodeBytes) {
        LCD_DEBUG(0, 200, "no-mem");
        return;
    }

    qrcode_initText(&qrcode, qrcodeBytes, version, ECC_HIGH, content);
    __bsp_lcd_draw_rect(x, y, qrcode.size * line_width + 12, qrcode.size * line_width + 12, 2, WHITE);
    __bsp_lcd_draw_rect(x+4, y+4, qrcode.size * line_width + 4, qrcode.size * line_width + 4, 2, WHITE);
    for (uint8_t i = 0; i < qrcode.size; i++) {
        for (uint8_t j = 0; j < qrcode.size; j++) {
            bsp_lcd_open_window(x+line_width*j+6,y+line_width*i+6,line_width,line_width);
            uint8_t times = line_width * line_width;
            uint16_t color = qrcode_getModule(&qrcode, j, i)?WHITE:BLACK;
            while(times--) LCD_WR_Data(color);
        }
    }

    free(qrcodeBytes);
    LCD_DEBUG(0, 280, "QRCode Success.");
}

