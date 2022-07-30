/*
 * fsm.c
 *
 *  Created on: 2022年7月30日
 *      Author: wvv
 */
#include "common.h"
static osMessageQueueId_t q_t;

void event_push_arg(Event e)
{
	osMessageQueuePut(q_t, &e, 0U, 0U);
}
void event_push(uint8_t e)
{
	Event sig = {.sig = e};
	osMessageQueuePut(q_t, &sig, 0U, 0U);
}
static State s;
State *me = &s;

static void state_dispatcher()
{
	while(TRUE)
	{
		if(osOK == osMessageQueueGet(q_t, &me->evt, NULL, osWaitForever))
		{
	        if(me->state)
	        {
	            me->state(me);
	        }
		}
		osThreadYield();
	}
}
static osThreadId_t _th;
static const osThreadAttr_t _attr = {
  .name = "fsm",
  .stack_size = 2048,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
void fsm_init()
{
	q_t = osMessageQueueNew(EVENT_Q_SZ, sizeof(Event), NULL);
	_th = osThreadNew(state_dispatcher, NULL, &_attr);
}
