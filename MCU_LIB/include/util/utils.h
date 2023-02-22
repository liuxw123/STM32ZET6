/**
 *
 * @ 名称: utils.h
 * @ 描述: TODO
 * @ 作者: lxw
 * @ 日期: Jan 7, 2023
 * @ 版本: V1.0
 *
 * Copyright (c) 2022. All rights reserved.
 */
#ifndef INCLUDE_UTIL_UTILS_H_
#define INCLUDE_UTIL_UTILS_H_



void gpio_bank_clk_enable(GPIO_TypeDef* gpio_bank);
void gpio_bank_clk_disable(GPIO_TypeDef* gpio_bank);

int bsp_gpio_structure_instantitation(bsp_gpio_t * const gpio, GPIO_TypeDef* bank, uint32_t pin, uint32_t mode, uint32_t pull, uint32_t speed);
int bsp_gpio_structure_destruction(bsp_gpio_t * const gpio);
int bsp_gpio_get_exti_no(int gpio_pin);
void ms_to_time_string(char* buf, int ms);
void clear_char_buffer(uint8_t* buffer, int size);

#endif /* INCLUDE_UTIL_UTILS_H_ */
