/**
 *
 * @ 名称: common_defs.h
 * @ 描述: TODO
 * @ 作者: lxw
 * @ 日期: Jan 6, 2023
 * @ 版本: V1.0
 *
 * Copyright (c) 2022. All rights reserved.
 */
#ifndef MCU_LIB_INCLUDE_BSP_COMMON_DEFS_H_
#define MCU_LIB_INCLUDE_BSP_COMMON_DEFS_H_

#include "stm32f1xx_hal.h"

#define ARRAY_SIZE(x)           (unsigned int)(sizeof(x)/sizeof(x[0]))

struct bsp_gpio {
    GPIO_TypeDef* gpio_bank_;
    GPIO_InitTypeDef* init_structure_;
};

typedef struct bsp_gpio bsp_gpio_t;

enum errno {
    OK = SUCCESS,
    TRUE = OK,
    EINVPARAM,
    ENOMEM,
    EUNKWN,
};

typedef enum errno error_t;

#endif /* MCU_LIB_INCLUDE_BSP_COMMON_DEFS_H_ */
