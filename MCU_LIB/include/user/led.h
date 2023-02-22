/**
 *
 * @ 名称: led.h
 * @ 描述: TODO
 * @ 作者: lxw
 * @ 日期: Jan 7, 2023
 * @ 版本: V1.0
 *
 * Copyright (c) 2022. All rights reserved.
 */
#ifndef INCLUDE_USER_LED_H_
#define INCLUDE_USER_LED_H_

#include "bsp/bsp_led.h"

enum user_led_status {
    LED_STATUS_INIT,
    LED_STATUS_OPENNING,
    LED_STATUS_OPENED,
    LED_STATUS_ON,
    LED_STATUS_OFF,
    LED_STATUS_CLOSING,
    LED_STATUS_CLOSED,
    LED_STATUS_ERROR,
};

typedef enum user_led_status user_led_status_t;

struct user_led {
    user_led_status_t status_;
    bsp_led_t instance_;

};

typedef struct user_led user_led_t;



#endif /* INCLUDE_USER_LED_H_ */
