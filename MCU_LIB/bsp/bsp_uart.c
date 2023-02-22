/**
 *
 * @ 名称: bsp_uart.c
 * @ 描述: TODO
 * @ 作者: lxw
 * @ 日期: Jan 7, 2023
 * @ 版本: V1.0
 *
 * Copyright (c) 2022. All rights reserved.
 */
#include <stdlib.h>
#include <string.h>
#include "bsp/bsp_uart.h"
#include "bsp/bsp_led.h"
#include "util/utils.h"
#include "user/logcat.h"
#include "stm32f1xx_hal_conf.h"


#define LOG_TAG                             "bsp_uart"

static bsp_uart_t bsp_uart1;

void uart_debug1();

#if USE_HAL_UART_REGISTER_CALLBACKS
static void __hal_uart_mspinit(UART_HandleTypeDef* huart);
static void __hal_uart_mspdeinit(UART_HandleTypeDef* huart);
static void __hal_uart_rxcplt_cbk(UART_HandleTypeDef *huart);
#endif


static int __init(void* bsp_uart1);
static int __deinit(void* bsp_uart1);
static int __send_char(void* bsp_uart1, const char ch);
static int __send(void* bsp_uart1, const char* str);
static void __receive(void* bsp_uart1, const char** buffer);

bsp_uart_ops_t uart_ops = {
    .init = __init,
    .deinit = __deinit,
    .send = __send,
    .send_char = __send_char,
    .receive = __receive
};


static int __init(void* bsp_uart1) {
    if(!bsp_uart1) return -EINVPARAM;
    bsp_uart_t* bsp_uart = (bsp_uart_t*)bsp_uart1;
    int ret = HAL_UART_Init(bsp_uart->uart_handler_);
    if (bsp_uart->enable_it_ == ENABLE) {
        HAL_UART_RegisterCallback(bsp_uart->uart_handler_, HAL_UART_RX_COMPLETE_CB_ID, __hal_uart_rxcplt_cbk);
        HAL_UART_Receive_IT(bsp_uart->uart_handler_, bsp_uart->uart_buf, BUFFER_SIZE_IT);
    }
    return ret;
}

static int __deinit(void* bsp_uart1) {
    if(!bsp_uart1) return -EINVPARAM;
    bsp_uart_t* bsp_uart = (bsp_uart_t*)bsp_uart1;
    return HAL_UART_DeInit(bsp_uart->uart_handler_);
}

static int __send_char(void* bsp_uart1, const char ch) {
    if(!bsp_uart1) return -EINVPARAM;
    bsp_uart_t* bsp_uart = (bsp_uart_t*)bsp_uart1;
    while (HAL_OK != HAL_UART_Transmit(bsp_uart->uart_handler_, (uint8_t *)&ch, 1, 5000));
    return SUCCESS;
}

static int __send(void* bsp_uart1, const char* const str) {
    if(!bsp_uart1) return -EINVPARAM;
    bsp_uart_t* bsp_uart = (bsp_uart_t*)bsp_uart1;
    while (HAL_OK != HAL_UART_Transmit(bsp_uart->uart_handler_, (uint8_t *)(str), strlen(str), 5000));
    return SUCCESS;
}

static void __receive(void* bsp_uart1, const char** buffer) {
    if (!bsp_uart1) {
        *buffer = NULL;
        return;
    }
    bsp_uart_t* bsp_uart = (bsp_uart_t*)bsp_uart1;
    *buffer = (char *)bsp_uart->uart_buf;
}

static int bsp_uart_instantitation(bsp_uart_t * const uart) {
    if (!uart) return EINVPARAM;
    bsp_uart_gpio_t* rx = (bsp_uart_gpio_t*)malloc(sizeof(bsp_uart_gpio_t));
    if (!rx) goto error_rx;
    bsp_uart_gpio_t* tx = (bsp_uart_gpio_t*)malloc(sizeof(bsp_uart_gpio_t));
    if (!tx) goto error_tx;
    UART_HandleTypeDef* handle = (UART_HandleTypeDef*)malloc(sizeof(UART_HandleTypeDef));
    if (!handle) goto error_handle;

    uart->uart_rx_ = rx;
    uart->uart_tx_ = tx;
    uart->uart_handler_ = handle;

    return SUCCESS;
error_handle:
    free(tx);
error_tx:
    free(rx);
error_rx:
    return ENOMEM;
}

static int bsp_uart_destruction(bsp_uart_t * const uart) {
    if (!uart) return SUCCESS;
    bsp_gpio_structure_destruction(uart->uart_rx_);
    bsp_gpio_structure_destruction(uart->uart_tx_);
    free(uart->uart_rx_);
    free(uart->uart_tx_);
    free(uart->uart_handler_);
    return SUCCESS;
}

static int bsp_uart1_init() {
    int ret;
    ret = bsp_uart_instantitation(&bsp_uart1);
    if (ret != SUCCESS) return ret;
    ret = bsp_gpio_structure_instantitation(bsp_uart1.uart_rx_, GPIOA, GPIO_PIN_10, GPIO_MODE_INPUT, GPIO_NOPULL, 0);
    if (ret != SUCCESS) return ret;
    ret = bsp_gpio_structure_instantitation(bsp_uart1.uart_tx_, GPIOA, GPIO_PIN_9, GPIO_MODE_AF_PP, 4, GPIO_SPEED_FREQ_HIGH);
    if (ret != SUCCESS) return ret;


    bsp_uart1.uart_handler_->Instance = USART1;
    bsp_uart1.uart_handler_->gState = HAL_UART_STATE_RESET;
#if USE_HAL_UART_REGISTER_CALLBACKS
    bsp_uart1.uart_handler_->MspInitCallback = __hal_uart_mspinit;
    bsp_uart1.uart_handler_->MspDeInitCallback = __hal_uart_mspdeinit;
    bsp_uart1.uart_handler_->RxCpltCallback = __hal_uart_rxcplt_cbk;
#endif
    bsp_uart1.uart_handler_->Init.BaudRate = 115200;
    bsp_uart1.uart_handler_->Init.WordLength = UART_WORDLENGTH_8B;
    bsp_uart1.uart_handler_->Init.StopBits = UART_STOPBITS_1;
    bsp_uart1.uart_handler_->Init.Parity = UART_PARITY_NONE;
    bsp_uart1.uart_handler_->Init.Mode = UART_MODE_TX_RX;
    bsp_uart1.uart_handler_->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    bsp_uart1.uart_handler_->Init.OverSampling = UART_OVERSAMPLING_16;
    bsp_uart1.enable_it_ = DISABLE;
    if (bsp_uart1.enable_it_ == ENABLE) bsp_uart1.it_no_ = USART1_IRQn;
    else bsp_uart1.it_no_ = 0;
    return SUCCESS;
}

static int bsp_uart1_deinit() {
    bsp_uart_destruction(&bsp_uart1);
    bsp_uart1.uart_rx_ = NULL;
    bsp_uart1.uart_tx_ = NULL;
    bsp_uart1.uart_handler_ = NULL;
    bsp_uart1.it_no_ = -1;
    return SUCCESS;
}

int bsp_uart_init() {
    bsp_uart1_init();
    return SUCCESS;
}

int bsp_uart_deinit() {
    bsp_uart1_deinit();
    return SUCCESS;
}

int bsp_uart_get(bsp_uart_t** uart, bsp_uart_ops_t** ops) {
    if (!uart || !ops) return -EINVPARAM;

    *uart = &bsp_uart1;
    *ops = &uart_ops;

    return SUCCESS;
}

int bsp_uart_get_for_logcat(void** uart, bsp_logcat_ops_t* ops) {
    if (!uart || !ops) return -EINVPARAM;

    *uart = (void*)&bsp_uart1;
    ops->init = __init;
    ops->deinit = __deinit;
    ops->send = __send;
    ops->send_char = __send_char;

    return SUCCESS;
}

void USART1_IRQHandler(void) {
    HAL_UART_IRQHandler(bsp_uart1.uart_handler_);
}

static void __hal_uart_rxcplt_cbk(UART_HandleTypeDef *huart) {
    unsigned int base = (unsigned int)(huart->Instance);
    bsp_uart_t* uart = NULL;
    switch (base) {
        case USART1_BASE:uart = &bsp_uart1;break;
    }

    if (uart) {
        ALOGD((char *)uart->uart_buf);
        HAL_UART_Receive_IT(uart->uart_handler_, uart->uart_buf, BUFFER_SIZE_IT);
    }
}

static void __hal_uart_mspinit(UART_HandleTypeDef* huart) {
    unsigned int base = (unsigned int)(huart->Instance);
    bsp_uart_t* uart = NULL;
    switch (base) {
        case USART1_BASE:
            uart = &bsp_uart1;
            __HAL_RCC_USART1_CLK_ENABLE();
            break;
    }

    if (uart != NULL) {
        gpio_bank_clk_enable(uart->uart_rx_->gpio_bank_);
        gpio_bank_clk_enable(uart->uart_tx_->gpio_bank_);
        HAL_GPIO_Init(uart->uart_rx_->gpio_bank_, uart->uart_rx_->init_structure_);
        HAL_GPIO_Init(uart->uart_tx_->gpio_bank_, uart->uart_tx_->init_structure_);
        if (uart->enable_it_ == ENABLE) {
            HAL_NVIC_SetPriority(uart->it_no_, 0, 0);
            HAL_NVIC_EnableIRQ(uart->it_no_);
        }
    }
}

static void __hal_uart_mspdeinit(UART_HandleTypeDef* huart) {
    unsigned int base = (unsigned int)(huart->Instance);
    bsp_uart_t* uart = NULL;
    switch (base) {
        case USART1_BASE:
            uart = &bsp_uart1;
            __HAL_RCC_USART1_CLK_ENABLE();
            break;
    }

    if (uart != NULL) {
        gpio_bank_clk_disable(uart->uart_rx_->gpio_bank_);
        gpio_bank_clk_disable(uart->uart_tx_->gpio_bank_);
        HAL_GPIO_DeInit(uart->uart_rx_->gpio_bank_, uart->uart_rx_->init_structure_->Pin);
        HAL_GPIO_DeInit(uart->uart_tx_->gpio_bank_, uart->uart_tx_->init_structure_->Pin);
        if (uart->enable_it_ == ENABLE) {
            HAL_NVIC_DisableIRQ(uart->it_no_);
        }
    }
}
