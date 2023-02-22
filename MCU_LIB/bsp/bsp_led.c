/**
 *
 * @ 名称: bsp_led.c
 * @ 描述: TODO
 * @ 作者: lxw
 * @ 日期: Jan 6, 2023
 * @ 版本: V1.0
 *
 * Copyright (c) 2022. All rights reserved.
 */
#include <stdlib.h>

#include "bsp/bsp_led.h"
#include "util/utils.h"


#define NUMBER_LED_INSTANCE             (4)

static bsp_led_t bsp_led1;
static bsp_led_t bsp_led2;
static bsp_led_t bsp_led3;
static bsp_led_t bsp_led4;

static int __init(bsp_led_t *bsp_led);
static int __deinit(bsp_led_t *bsp_led);
static int __on(bsp_led_t *bsp_led);
static int __off(bsp_led_t *bsp_led);
static int __toggle(bsp_led_t *bsp_led);
static bsp_led_status_t __status(bsp_led_t *bsp_led);
static int __set_status(bsp_led_t *bsp_led, bsp_led_status_t status);

static int __init_dummy(bsp_led_t *bsp_led) {
    return SUCCESS;
}

static int __deinit_dummy(bsp_led_t *bsp_led){
    return SUCCESS;
}

static int __on_dummy(bsp_led_t *bsp_led){
    return SUCCESS;
}

static int __off_dummy(bsp_led_t *bsp_led){
    return SUCCESS;
}

static int __toggle_dummy(bsp_led_t *bsp_led){
    return SUCCESS;
}

static bsp_led_status_t __status_dummy(bsp_led_t *bsp_led){
    return LED_OFF;
}

static int __set_status_dummy(bsp_led_t *bsp_led, bsp_led_status_t status){
    return SUCCESS;
}


static bsp_led_ops_t ops = {
    .init = __init,
    .deinit = __deinit,
    .on = __on,
    .off = __off,
    .toggle = __toggle,
    .status = __status,
    .set_status = __set_status
};

static bsp_led_ops_t ops_dummy = {
    .init = __init_dummy,
    .deinit = __deinit_dummy,
    .on = __on_dummy,
    .off = __off_dummy,
    .toggle = __toggle_dummy,
    .status = __status_dummy,
    .set_status = __set_status_dummy
};

static int __init(bsp_led_t *bsp_led) {
    if (!bsp_led) return -EINVPARAM;
    gpio_bank_clk_enable(bsp_led->gpio_bank_);
    HAL_GPIO_Init(bsp_led->gpio_bank_, bsp_led->init_structure_);
    return SUCCESS;
}

static int __deinit(bsp_led_t *bsp_led) {
    if (!bsp_led) return -EINVPARAM;
    gpio_bank_clk_disable(bsp_led->gpio_bank_);
    HAL_GPIO_DeInit(bsp_led->gpio_bank_, bsp_led->init_structure_->Pin);
    return SUCCESS;
}

static int __on(bsp_led_t *bsp_led) {
    if (!bsp_led) return -EINVPARAM;
    HAL_GPIO_WritePin(bsp_led->gpio_bank_, bsp_led->init_structure_->Pin, LED_ON);
    return SUCCESS;
}
static int __off(bsp_led_t *bsp_led){
    if (!bsp_led) return -EINVPARAM;
    HAL_GPIO_WritePin(bsp_led->gpio_bank_, bsp_led->init_structure_->Pin, LED_OFF);
    return SUCCESS;
}

static int __toggle(bsp_led_t *bsp_led){
    if (!bsp_led) return -EINVPARAM;
    HAL_GPIO_TogglePin(bsp_led->gpio_bank_, bsp_led->init_structure_->Pin);
    return SUCCESS;
}

static bsp_led_status_t __status(bsp_led_t *bsp_led){
    if (!bsp_led) return LED_OFF;
    return HAL_GPIO_ReadPin(bsp_led->gpio_bank_, bsp_led->init_structure_->Pin);
}

static int __set_status(bsp_led_t *bsp_led, bsp_led_status_t status){
    if (!bsp_led) return -EINVPARAM;
    HAL_GPIO_WritePin(bsp_led->gpio_bank_, bsp_led->init_structure_->Pin, status);
    return SUCCESS;
}

int bsp_led_open() {
    ops.init(&bsp_led1);
    ops.init(&bsp_led2);
    ops.init(&bsp_led3);
    ops.init(&bsp_led4);
    ops.off(&bsp_led1);
    ops.off(&bsp_led2);
    ops.off(&bsp_led3);
    ops.off(&bsp_led4);

    return SUCCESS;
}

void debug_led_on(int index) {
    switch(index) {
    case 1: ops.on(&bsp_led1); break;
    case 2: ops.on(&bsp_led2); break;
    case 3: ops.on(&bsp_led3); break;
    case 4: ops.on(&bsp_led4); break;
    }
}

void debug_led_toggle(int index) {
    switch(index) {
    case 1: ops.toggle(&bsp_led1); break;
    case 2: ops.toggle(&bsp_led2); break;
    case 3: ops.toggle(&bsp_led3); break;
    case 4: ops.toggle(&bsp_led4); break;
    }
}

int bsp_led_init() {
    bsp_gpio_structure_instantitation(&bsp_led1, GPIOG, GPIO_PIN_6, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH);
    bsp_gpio_structure_instantitation(&bsp_led2, GPIOA, GPIO_PIN_8, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH);
    bsp_gpio_structure_instantitation(&bsp_led3, GPIOB, GPIO_PIN_8, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH);
    bsp_gpio_structure_instantitation(&bsp_led4, GPIOB, GPIO_PIN_9, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH);
    return SUCCESS;
}

int bsp_led_deinit() {
    bsp_gpio_structure_destruction(&bsp_led1);
    bsp_gpio_structure_destruction(&bsp_led2);
    bsp_gpio_structure_destruction(&bsp_led3);
    bsp_gpio_structure_destruction(&bsp_led4);
    return SUCCESS;
}

int bsp_led_instance_no() {
    return NUMBER_LED_INSTANCE;
}
