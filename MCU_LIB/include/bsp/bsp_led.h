/**
 *
 * @ 名称: bsp_led.h
 * @ 描述: TODO
 * @ 作者: lxw
 * @ 日期: Jan 6, 2023
 * @ 版本: V1.0
 *
 * Copyright (c) 2022. All rights reserved.
 */
#ifndef MCU_LIB_INCLUDE_BSP_BSP_LED_H_
#define MCU_LIB_INCLUDE_BSP_BSP_LED_H_

#include "core/common_defs.h"


typedef struct bsp_gpio bsp_led_t;

enum bsp_led_status {
    LED_ON = GPIO_PIN_RESET,
    LED_OFF = GPIO_PIN_SET
};

typedef enum bsp_led_status bsp_led_status_t;

struct bsp_led_operation {
    int (*init)(bsp_led_t *bsp_led);
    int (*deinit)(bsp_led_t *bsp_led);
    int (*on)(bsp_led_t *bsp_led);
    int (*off)(bsp_led_t *bsp_led);
    int (*toggle)(bsp_led_t *bsp_led);
    bsp_led_status_t (*status)(bsp_led_t *bsp_led);
    int (*set_status)(bsp_led_t *bsp_led, bsp_led_status_t status);
};

typedef struct bsp_led_operation bsp_led_ops_t;


int bsp_led_init();
int bsp_led_deinit();
int bsp_led_open();
int bsp_led_instance_no();
void debug_led_on(int index);

#endif /* MCU_LIB_INCLUDE_BSP_BSP_LED_H_ */
