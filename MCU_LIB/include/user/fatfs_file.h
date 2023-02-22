/**
 *
 * @ 名称: fatfs_file.h
 * @ 描述: TODO
 * @ 作者: lxw
 * @ 日期: 2023年1月15日
 * @ 版本: V1.0
 *
 * Copyright (c) 2022. All rights reserved.
 */
#ifndef INCLUDE_USER_FATFS_FILE_H_
#define INCLUDE_USER_FATFS_FILE_H_
#include "ff.h"

int bsp_fatfs_init();
int d_exist(const char* dir);
int f_exist(const char* file_name);
void f_full_path(const char* file_name, char * buf, const uint16_t buf_length);

#endif /* INCLUDE_USER_FATFS_FILE_H_ */
