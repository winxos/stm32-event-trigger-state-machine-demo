/*
 * fsm.c
 *
 *  Created on: 2022年7月30日
 *      Author: wvv
 */
#include "common.h"
osMessageQueueId_t q_t;
void fsm_init()
{
	q_t = osMessageQueueNew(EVENT_Q_SZ, sizeof(Event), NULL);
}
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

void state_dispatcher()
{
	if(osOK == osMessageQueueGet(q_t, &me->evt, NULL, 0))
	{
        if(me->state)
        {
            me->state(me);
        }
	}
}
