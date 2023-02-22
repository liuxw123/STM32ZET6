/**
 *
 * @ 名称: bsp_key.c
 * @ 描述: TODO
 * @ 作者: lxw
 * @ 日期: Jan 8, 2023
 * @ 版本: V1.0
 *
 * Copyright (c) 2022. All rights reserved.
 */
#include <stdlib.h>
#include "bsp/bsp_key.h"
#include "util/utils.h"
#include "user/logcat.h"

#define LOG_TAG                         "bsp_key"

static bsp_key_t bsp_key1;
static bsp_key_t bsp_key2;
static bsp_key_t bsp_key3;
static bsp_key_t bsp_key4;

static int __init(bsp_key_t *bsp_key);
static int __deinit(bsp_key_t *bsp_key);
static bsp_key_status_t __status(bsp_key_t *bsp_key);


static bsp_key_ops_t ops = {
        .init = __init,
        .deinit = __deinit,
        .status = __status
};

extern uint8_t key_pressed;


static int __init(bsp_key_t *bsp_key) {
    if (!bsp_key) return -EINVPARAM;

    gpio_bank_clk_enable(bsp_key->key_gpio_->gpio_bank_);
    HAL_GPIO_Init(bsp_key->key_gpio_->gpio_bank_, bsp_key->key_gpio_->init_structure_);
    int no = bsp_gpio_get_exti_no(bsp_key->key_gpio_->init_structure_->Pin);
    if (no > 0) {
        HAL_NVIC_SetPriority(no, 0, 0);
        HAL_NVIC_EnableIRQ(no);
    }
    return SUCCESS;
}

static int __deinit(bsp_key_t *bsp_key) {
    if (!bsp_key) return -EINVPARAM;

    gpio_bank_clk_disable(bsp_key->key_gpio_->gpio_bank_);
    HAL_GPIO_DeInit(bsp_key->key_gpio_->gpio_bank_, bsp_key->key_gpio_->init_structure_->Pin);
    int no = bsp_gpio_get_exti_no(bsp_key->key_gpio_->init_structure_->Pin);
    if (no > 0) {
        HAL_NVIC_DisableIRQ(no);
    }
    return SUCCESS;
}

static bsp_key_status_t __status(bsp_key_t *bsp_key) {
    if (!bsp_key) return KEY_POPUP;
    return bsp_key->key_state_;
}


int bsp_key_init() {
    bsp_key1.key_gpio_ = (bsp_gpio_t*)malloc(sizeof(bsp_gpio_t));
    bsp_key2.key_gpio_ = (bsp_gpio_t*)malloc(sizeof(bsp_gpio_t));
    bsp_key3.key_gpio_ = (bsp_gpio_t*)malloc(sizeof(bsp_gpio_t));
    bsp_key4.key_gpio_ = (bsp_gpio_t*)malloc(sizeof(bsp_gpio_t));
    if (bsp_key1.key_gpio_) bsp_gpio_structure_instantitation(bsp_key1.key_gpio_, GPIOE, GPIO_PIN_2, GPIO_MODE_IT_FALLING, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
    if (bsp_key2.key_gpio_) bsp_gpio_structure_instantitation(bsp_key2.key_gpio_, GPIOE, GPIO_PIN_3, GPIO_MODE_IT_FALLING, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
    if (bsp_key3.key_gpio_) bsp_gpio_structure_instantitation(bsp_key3.key_gpio_, GPIOE, GPIO_PIN_4, GPIO_MODE_IT_FALLING, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
    if (bsp_key4.key_gpio_) bsp_gpio_structure_instantitation(bsp_key4.key_gpio_, GPIOE, GPIO_PIN_5, GPIO_MODE_IT_FALLING, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH);
    return SUCCESS;
}

int bsp_key_deinit() {
    bsp_gpio_structure_destruction(bsp_key1.key_gpio_);
    bsp_gpio_structure_destruction(bsp_key2.key_gpio_);
    bsp_gpio_structure_destruction(bsp_key3.key_gpio_);
    bsp_gpio_structure_destruction(bsp_key4.key_gpio_);

    free(bsp_key1.key_gpio_);
    free(bsp_key2.key_gpio_);
    free(bsp_key3.key_gpio_);
    free(bsp_key4.key_gpio_);

    bsp_key1.key_gpio_ = NULL;
    bsp_key2.key_gpio_ = NULL;
    bsp_key3.key_gpio_ = NULL;
    bsp_key4.key_gpio_ = NULL;
    return SUCCESS;
}

int bsp_key_open() {
    ops.init(&bsp_key1);
    ops.init(&bsp_key2);
    ops.init(&bsp_key3);
    ops.init(&bsp_key4);

    return SUCCESS;
}

void EXTI2_IRQHandler(void) {
    ALOGD("key1 pressed.");
    key_pressed = 1;
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}

void EXTI3_IRQHandler(void) {
    ALOGD("key2 pressed.");
    key_pressed = 1;
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}

void EXTI4_IRQHandler(void) {
    ALOGD("key3 pressed.");
    key_pressed = 1;
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
}

void EXTI9_5_IRQHandler(void) {
    ALOGD("key4 pressed.");
    key_pressed = 1;
    HAL_GPIO_EXTI_IRQHandler(bsp_key4.key_gpio_->init_structure_->Pin);
}
