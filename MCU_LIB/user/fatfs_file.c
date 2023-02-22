/**
 *
 * @ 名称: fatfs_file.c
 * @ 描述: TODO
 * @ 作者: lxw
 * @ 日期: 2023年1月15日
 * @ 版本: V1.0
 *
 * Copyright (c) 2022. All rights reserved.
 */


#include <string.h>
#include <stdio.h>
#include "bsp/bsp_lcd.h"
#include "user/fatfs_file.h"
#include "fonts.h"

#define FILE_DRIVE_PATH         "0"
static FATFS ff_disk;
static FIL file;


void f_full_path(const char* file_name, char * buf, const uint16_t buf_length) {
    snprintf(buf, buf_length, "%s:%s", FILE_DRIVE_PATH, file_name);
}

int f_exist(const char* file_name) {
    FIL ff_fil;
    FRESULT ret = f_open(&ff_fil, file_name, FA_OPEN_EXISTING);
    LCD_DEBUG(0, 100, "exist: %d", ret);
    if (ret == FR_OK) f_close(&ff_fil);
    return ret == FR_OK;
}

int d_exist(const char* dir) {
    char curr_dir[256];
    FRESULT ret = f_getcwd(curr_dir, 256);
    if (ret != FR_OK) {
        return ret;
    }
    ret = f_chdir(dir);
    if (ret == FR_OK) {
        ret = f_chdir(curr_dir);
    }
    return ret == FR_OK;
}

int bsp_fatfs_init() {
    const char init_file[16];
    f_full_path("hello.txt", (char *)init_file, 16);
    const char test_string[] = "Hello World!";
    uint32_t test_length = strlen(test_string);
    const char test_dir[16];
    f_full_path("/temp", (char *)test_dir, 16);
    FRESULT ret;

    ret = f_mount(&ff_disk, FILE_DRIVE_PATH, 1);
    if (ret != FR_OK) {
       LCD_DEBUG(0, 0, "f_mount failed. ret: %d", ret);
       return ret;
    }

    if (f_exist(init_file)) {
        f_unlink(init_file);
    }

    ret = f_open(&file, init_file, FA_WRITE | FA_CREATE_NEW);
    if (ret != FR_OK) {
        LCD_DEBUG(0, 0, "open failed. %d", ret);
        return -1;
    }

    unsigned int write;
    ret = f_write(&file, (void *)test_string, test_length, &write);
    if (ret != FR_OK) {
        LCD_DEBUG(0, 0, "write failed. %d", ret);
        return ret;
    }

    ret = f_close(&file);
    if (ret != FR_OK) {
        LCD_DEBUG(0, 0, "close failed. %d", ret);
        return ret;
    }

    ret = f_open(&file, init_file, FA_READ);
    if (ret != FR_OK) {
        LCD_DEBUG(0, 0, "open-2 failed.");
        return -1;
    }

    char buff[13];
    unsigned int read;
    ret = f_read(&file, buff, test_length, &read);
    if (ret != FR_OK) {
        LCD_DEBUG(0, 0, "read failed. %d", ret);
        return ret;
    }

    ret = f_close(&file);
    if (ret != FR_OK) {
        LCD_DEBUG(0, 0, "close-2 failed. %d", ret);
        return ret;
    }

    if (strncmp(buff, test_string, test_length) != 0) {
        LCD_DEBUG(0, 0, "wr data compare failed.");
    }

    ret = f_getcwd(buff, test_length+1);
    if (ret != FR_OK) {
        LCD_DEBUG(0, 0, "get_cwd failed.");
    }

    if (d_exist(test_dir)) {
        ret = f_unlink(test_dir);
    }
    ret = f_mkdir(test_dir);
    if (ret != FR_OK) {
        LCD_DEBUG(0, 0, "mkdir failed. %d", ret);
        return ret;
    }

    ret = f_unlink(test_dir);
    if (ret != FR_OK) {
        LCD_DEBUG(0, 0, "delete test dir. failed: %d", ret);
        return ret;
    }

    ret = f_unlink(init_file);
    if (ret != FR_OK) {
        LCD_DEBUG(0, 0, "delete init_file. failed: %d", ret);
        return ret;
    }

    LCD_DEBUG(0, 300, "fatfs_init success.");
    return 0;
}
