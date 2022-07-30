/*
 * fsm.h
 *
 *  Created on: 2022年7月30日
 *      Author: wvv
 */

#ifndef SRC_FSM_H_
#define SRC_FSM_H_
#include "def.h"
enum {
    SIG_ENTRY = 1,
	SIG_TICK,
	SIG_KEYPRESS,
	SIG_SCREEN_RST,
};
#define EVENT_Q_SZ		(10)
#define EVENT_MSG_SZ	(15)
typedef struct{
	uint8_t sig;
	uint8_t data[EVENT_MSG_SZ];
}Event;
void event_push(uint8_t e);
void event_push_arg(Event e);

typedef struct{
    pFunPtr state;
    Event evt;
}State;
extern State *me;
#define TASK_TO(_target) (me->state = (pFunPtr)_target,event_push(SIG_ENTRY))
#define SIG(me)	((uint8_t)(me->evt.sig))
void state_dispatcher();

void fsm_init();
#endif /* SRC_FSM_H_ */
