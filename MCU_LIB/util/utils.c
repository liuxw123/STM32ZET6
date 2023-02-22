/**
 *
 * @ 名称: utils.c
 * @ 描述: TODO
 * @ 作者: lxw
 * @ 日期: Jan 7, 2023
 * @ 版本: V1.0
 *
 * Copyright (c) 2022. All rights reserved.
 */
#include <stdlib.h>
#include <stdio.h>
#include "core/common_defs.h"
#include "util/utils.h"


void gpio_bank_clk_disable(GPIO_TypeDef* gpio_bank) {
    unsigned int base = (unsigned int)(gpio_bank);
    switch(base) {
        case GPIOA_BASE: __HAL_RCC_GPIOA_CLK_DISABLE(); break;
        case GPIOB_BASE: __HAL_RCC_GPIOB_CLK_DISABLE(); break;
        case GPIOC_BASE: __HAL_RCC_GPIOC_CLK_DISABLE(); break;
        case GPIOD_BASE: __HAL_RCC_GPIOD_CLK_DISABLE(); break;
        case GPIOE_BASE: __HAL_RCC_GPIOE_CLK_DISABLE(); break;
        case GPIOF_BASE: __HAL_RCC_GPIOF_CLK_DISABLE(); break;
        case GPIOG_BASE: __HAL_RCC_GPIOG_CLK_DISABLE(); break;
    }
}

void gpio_bank_clk_enable(GPIO_TypeDef* gpio_bank) {
    unsigned int base = (unsigned int)(gpio_bank);
    switch(base) {
        case GPIOA_BASE: __HAL_RCC_GPIOA_CLK_ENABLE(); break;
        case GPIOB_BASE: __HAL_RCC_GPIOB_CLK_ENABLE(); break;
        case GPIOC_BASE: __HAL_RCC_GPIOC_CLK_ENABLE(); break;
        case GPIOD_BASE: __HAL_RCC_GPIOD_CLK_ENABLE(); break;
        case GPIOE_BASE: __HAL_RCC_GPIOE_CLK_ENABLE(); break;
        case GPIOF_BASE: __HAL_RCC_GPIOF_CLK_ENABLE(); break;
        case GPIOG_BASE: __HAL_RCC_GPIOG_CLK_ENABLE(); break;
    }
}

int bsp_gpio_structure_instantitation(bsp_gpio_t * const gpio, GPIO_TypeDef* bank, uint32_t pin, uint32_t mode, uint32_t pull, uint32_t speed) {
    if (!gpio) return -EINVPARAM;
    GPIO_InitTypeDef* init_structure = (GPIO_InitTypeDef*)malloc(sizeof(GPIO_InitTypeDef));
    if (!init_structure) return -ENOMEM;

    init_structure->Pin = pin;
    init_structure->Mode = mode;
    if (pull == GPIO_NOPULL || pull == GPIO_PULLUP || pull == GPIO_PULLDOWN) init_structure->Pull = pull;
    if (speed == GPIO_SPEED_FREQ_LOW || speed == GPIO_SPEED_FREQ_MEDIUM || speed == GPIO_SPEED_FREQ_HIGH) init_structure->Speed = speed;

    gpio->gpio_bank_ = bank;
    gpio->init_structure_ = init_structure;
    return SUCCESS;
}

int bsp_gpio_structure_destruction(bsp_gpio_t * const gpio) {
    if (!gpio) goto exit;
    gpio->gpio_bank_ = NULL;
    free(gpio->init_structure_);
exit:
    return SUCCESS;
}

int bsp_gpio_get_exti_no(int gpio_pin) {
    switch (gpio_pin) {
        case GPIO_PIN_0: return EXTI0_IRQn;
        case GPIO_PIN_1: return EXTI1_IRQn;
        case GPIO_PIN_2: return EXTI2_IRQn;
        case GPIO_PIN_3: return EXTI3_IRQn;
        case GPIO_PIN_4: return EXTI4_IRQn;
        case GPIO_PIN_5:
        case GPIO_PIN_6:
        case GPIO_PIN_7:
        case GPIO_PIN_8:
        case GPIO_PIN_9: return EXTI9_5_IRQn;
        case GPIO_PIN_10:
        case GPIO_PIN_11:
        case GPIO_PIN_12:
        case GPIO_PIN_13:
        case GPIO_PIN_14:
        case GPIO_PIN_15: return EXTI15_10_IRQn;
    }

    return 0;
}

void ms_to_time_string(char* buf, int ms) {
    if (!buf) return;

    int sec = ms / 1000;
    ms = ms % 1000;
    int min = sec / 60;
    sec = sec % 60;
    int hour = min / 60;
    min = min % 60;
    hour = hour % 24;

    sprintf(buf, "%02d:%02d:%02d.%03d", hour, min, sec, ms);
}

void clear_char_buffer(uint8_t* buffer, int size) {
    if (!buffer || size <= 0) return;
    int i = 0;
    while (i < size) {
        buffer[i] = 0x00;
        i++;
    }
}


