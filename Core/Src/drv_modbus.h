/*
 * drv_modbus.h
 *
 *  Created on: 2022年7月12日
 *      Author: dev
 */

#ifndef SRC_APP_DRV_MODBUS_H_
#define SRC_APP_DRV_MODBUS_H_
#include "def.h"
void drv_modbus_init();
void drv_modbus_read_regs(uint8_t address, uint16_t start, uint16_t count, pFunPtr success, pFunPtr fail);
void drv_modbus_write_reg(uint8_t address, uint16_t start, uint16_t value, pFunPtr success, pFunPtr fail);

#endif /* SRC_APP_DRV_MODBUS_H_ */
