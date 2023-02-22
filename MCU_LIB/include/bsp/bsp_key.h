/**
 *
 * @ 名称: bsp_key.h
 * @ 描述: TODO
 * @ 作者: lxw
 * @ 日期: Jan 8, 2023
 * @ 版本: V1.0
 *
 * Copyright (c) 2022. All rights reserved.
 */
#ifndef INCLUDE_BSP_BSP_KEY_H_
#define INCLUDE_BSP_BSP_KEY_H_
#include "core/common_defs.h"


enum bsp_key_status {
    KEY_PRESSED = GPIO_PIN_RESET,
    KEY_POPUP = GPIO_PIN_SET
};

typedef enum bsp_key_status bsp_key_status_t;

struct bsp_key {
    bsp_gpio_t* key_gpio_;
    bsp_key_status_t key_state_;

};

typedef struct bsp_key bsp_key_t;

struct bsp_key_operation {
    int (*init)(bsp_key_t *bsp_key);
    int (*deinit)(bsp_key_t *bsp_key);
    bsp_key_status_t (*status)(bsp_key_t *bsp_key);
};

typedef struct bsp_key_operation bsp_key_ops_t;



int bsp_key_init();
int bsp_key_deinit();
int bsp_key_open();



#endif /* INCLUDE_BSP_BSP_KEY_H_ */
