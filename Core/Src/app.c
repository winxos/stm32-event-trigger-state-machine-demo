/*
 * app.c
 *
 *  Created on: Jul 29, 2022
 *      Author: wvv
 */
#include "common.h"
#include <FreeRTOS.h>
#include <portable.h>
#include <task.h>


int __io_putchar(int ch)
{
	ITM_SendChar(ch);
	return 0;
}
void show_mem()
{
	log_d("f1 mem rest:%d, head:%d",uxTaskGetStackHighWaterMark(NULL),xPortGetFreeHeapSize());
}
static void task_action(State * const me)
{
	log_d("task entry");
	show_mem();
}
static void task_tick(State * const me)
{
	log_d("task tick");
	show_mem();
}
void task_idle(State * const me)
{
    switch(SIG(me))
    {
        case SIG_ENTRY:
        	task_action(me);
            break;
        case SIG_TICK:
        	task_tick(me);
        	break;
    }
}
void recv(uint8_t *data, uint32_t len)
{
	log_d("rx %d",len);
}
void led_blink(void *p)
{
	HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);
	event_push(SIG_TICK);
}
osTimerId_t t_ledblink;
const osTimerAttr_t ledblink_attr = {
  .name = "ledblink"
};

void system_run()
{
	show_mem();
	drv_uart_init();
	fsm_init();
	show_mem();
	t_ledblink = osTimerNew(led_blink, osTimerPeriodic, NULL, &ledblink_attr);
	osTimerStart(t_ledblink, 1000);
	show_mem();
	TASK_TO(task_idle);
	drv_uart_set_callback(0, recv);
	while(1)
	{
		state_dispatcher();
		osDelay(1);
	}
}
