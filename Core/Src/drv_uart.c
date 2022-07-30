/*
 * drv_uart.c
 *
 *  Created on: Dec 16, 2021
 *      Author: dev
 */
#include "common.h"

#define UART_MAX_COUNT 4
#define RX_BUF_SIZE 200
enum
{
	S_UART_IDLE, S_UART_BUSY, S_UART_READY,
};
struct UartInfo
{
	UART_HandleTypeDef *huart;
	pFunUart callback;
	uint32_t timeout;
	uint8_t buf;
	uint8_t state;
	uint8_t len;
	uint8_t rxbuf[RX_BUF_SIZE];
};
osSemaphoreId_t rx_sem;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart4;
static struct UartInfo _uarts[] =
{
{ .huart = &huart1 },
{ .huart = &huart2 },
{ .huart = &huart3 },
{ .huart = &huart4 } };
void drv_uart_send(uint8_t id, uint8_t *data, uint16_t len)
{
	HAL_UART_Transmit(_uarts[id].huart, data, len, 0xffff);
}
void drv_uart_set_callback(uint8_t id, pFunUart f)
{
	_uarts[id].callback = f;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	for (uint8_t id = 0; id < UART_MAX_COUNT; id++)
	{
		if (huart == _uarts[id].huart)
		{
			_uarts[id].rxbuf[_uarts[id].len++] = _uarts[id].buf;
			_uarts[id].timeout = 0;
			_uarts[id].state = S_UART_BUSY;
			HAL_UART_Receive_IT(huart, &_uarts[id].buf, 1);
		}
	}
}
void drv_uart_tick()
{
	for (uint8_t id = 0; id < UART_MAX_COUNT; id++)
	{
		if (_uarts[id].state == S_UART_BUSY)
		{
			_uarts[id].timeout++;
			if (_uarts[id].timeout >= 3)
			{
				_uarts[id].state = S_UART_READY;
				osSemaphoreRelease(rx_sem);
				_uarts[id].timeout = 0;
			}
		}
	}
}
void drv_uart_loop()
{
	while(TRUE)
	{
		osSemaphoreAcquire(rx_sem, osWaitForever);
		for (uint8_t id = 0; id < UART_MAX_COUNT; id++)
		{
			if (_uarts[id].state == S_UART_READY)
			{
				if (_uarts[id].callback)
				{
					_uarts[id].callback(_uarts[id].rxbuf, _uarts[id].len);
				}
				_uarts[id].len = 0;
				_uarts[id].state = S_UART_IDLE;
			}
		}
		osThreadYield();
	}
}
static osTimerId_t _tick;
static const osTimerAttr_t _tick_attr = {
  .name = "uart_tick"
};
static osThreadId_t _th;
static const osThreadAttr_t _th_attr = {
  .name = "uart",
  .stack_size = 1024,
  .priority = (osPriority_t) osPriorityHigh,
};
void drv_uart_init()
{
	for (uint8_t id = 0; id < UART_MAX_COUNT; id++)
	{
		HAL_UART_Receive_IT(_uarts[id].huart, &_uarts[id].buf, 1);
	}
	rx_sem = osSemaphoreNew(1, 0, NULL);
	_tick = osTimerNew(drv_uart_tick, osTimerPeriodic, NULL, &_tick_attr);
	osTimerStart(_tick, 1);
	_th = osThreadNew(drv_uart_loop, NULL, &_th_attr);
}
