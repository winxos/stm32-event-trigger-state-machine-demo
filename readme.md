# a framework for event trigger finite state machine

### user code

```c
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
```
using TASK_TO, switch state
```c
TASK_TO(task_idle);
```
a modbus visit demo
```c
void success()
{
	log_d("rx success");
}
void fail()
{
	log_d("rx failed");
}
static void task_tick(State * const me)
{
	drv_modbus_read_regs(1, 0, 1, success, fail);
}
```
### core code
fsm.c
```c
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
```
fsm.h
```c
#include "def.h"
enum {
    SIG_ENTRY = 1,
	SIG_TICK,
	SIG_KEYPRESS
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

void fsm_init();
```
