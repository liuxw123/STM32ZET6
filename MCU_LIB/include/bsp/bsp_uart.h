/**
 *
 * @ 名称: bsp_uart.h
 * @ 描述: TODO
 * @ 作者: lxw
 * @ 日期: Jan 7, 2023
 * @ 版本: V1.0
 *
 * Copyright (c) 2022. All rights reserved.
 */
#ifndef INCLUDE_BSP_BSP_UART_H_
#define INCLUDE_BSP_BSP_UART_H_

#include "core/common_defs.h"
#include "user/logcat.h"

#define BUFFER_SIZE_IT                      (8)

typedef struct bsp_gpio bsp_uart_gpio_t;

struct bsp_uart {
    bsp_uart_gpio_t* uart_rx_;
    bsp_uart_gpio_t* uart_tx_;
    uint8_t it_no_;
    UART_HandleTypeDef* uart_handler_;
    FunctionalState enable_it_;
    uint8_t uart_buf[BUFFER_SIZE_IT+1];
};
typedef struct bsp_uart bsp_uart_t;

struct bsp_uart_operation {
    int (*init)(void* bsp_uart);
    int (*deinit)(void* bsp_uart);
    int (*send_char)(void* bsp_uart, const char ch);
    int (*send)(void* bsp_uart, const char* const str);
    void (*receive)(void* bsp_uart, const char** buffer);
};

typedef struct bsp_uart_operation bsp_uart_ops_t;

int bsp_uart_init();
int bsp_uart_deinit();
int bsp_uart_get(bsp_uart_t** uart, bsp_uart_ops_t** ops);
int bsp_uart_get_for_logcat(void** uart, bsp_logcat_ops_t* ops);


#endif /* INCLUDE_BSP_BSP_UART_H_ */
