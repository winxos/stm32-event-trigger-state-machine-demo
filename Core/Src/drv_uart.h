/*
 * drv_uart.h
 *
 *  Created on: Dec 16, 2021
 *      Author: dev
 */

#ifndef SRC_APP_DRV_UART_H_
#define SRC_APP_DRV_UART_H_

#include "def.h"
void drv_uart_send(uint8_t id, uint8_t *data, uint16_t len);
void drv_uart_init();
void drv_uart_set_callback(uint8_t id, pFunUart f);
#endif /* SRC_APP_DRV_UART_H_ */
