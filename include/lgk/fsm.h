#ifndef LGK_FSM_H
#define LGK_FSM_H

#include <stdint.h>
#include <lgk/tnt.h>

#define FSM_TYPES(name, type_context, type_state, type_event, type_event_data)\
    typedef type_state name##_enter_handler(type_context context, type_state state);\
    typedef type_state name##_event_handler(type_context context, type_state state, type_event event, type_event_event_data);\
    struct name\
    {\
	    type_state current;\
        type_state  next;\
    	type_context context;\
    	name##_enter_handler **enter_handlers;\
    	name##_event_handler **event_handlers;\
    	type_state n_states;\
    };

#define FSM_PROTOTYPES(name, type_context, type_event, type_event_data)\
    int_fast8_t name##_init(struct name *fsm, void *context, name##_enter_handler **enter_handlers, name##_event_handler **event_handlers, name##_state_t n_states);\
    int_fast8_t name##_reset(struct name *fsm);\
    int_fast8_t name##_event(struct name *fsm, type_event event, type_event_data event_data);\
    int_fast8_t name##_step(struct name *fsm);

#define FSM_FUNCTION_INIT(name)\
    int_fast8_t name##_init(struct name *fsm, void *context, name##_enter_handler **enter_handlers, name##_event_handler **event_handlers, name##_state_t n_states)\
    {\
        TRAPNULL(enter_handlers);\
        TRAPNULL(event_handlers);\
        fsm->current = 0;\
        fsm->next = -1;\
        fsm->context = context;\
        fsm->enter_handlers = enter_handlers;\
        fsm->event_handlers = event_handlers;\
        fsm->n_states = n_states;\
        return 0;\
    trap_enter_handlers_null:\
    trap_event_handlers_null:\
        return -1;\
    }

#define FSM_FUNCTION_RESET(name)\
    int_fast8_t name##_reset(struct name *fsm)\
    {\
        TRAPNULL(fsm);\
        fsm->current = 0;\
        fsm->next = -1;\
        return 0;\
    trap_fsm_null:\
        return -1;\
    }

#define FSM_FUNCTION_EVENT(name, type_event, type_event_data)\
    int_fast8_t name##_event(struct name *fsm, type_event event, type_event_data event_data)\
    {\
        TRAPNULL(fsm);\
        TRAP((fsm->current<0)||(fsm->current>=fsm->n_states), current_range, "fsm->current==%i out of range", (int)fsm->current);\
        TRAP(fsm->next>=0, in_transition, "fsm->next==%i", (int)fsm->next);\
        name##_event_handler **event_handlers = fsm->event_handlers;\
        TRAPNULL(event_handlers);\
        name##_event_handler *event_handler = event_handlers[fsm->current];\
        TRAPNULL(event_handler);\
        fsm->next = event_handler(fsm->context, fsm->current, event, event_data);\
        return (fsm->next >= 0);\
    trap_event_handler_null:\
    trap_event_handlers_null:\
    trap_in_transition:\
    trap_current_range:\
    trap_fsm_null:\
        return -1;\
    }

#define FSM_FUNCTION_STEP(name)\
    int_fast8_t fsm_step(struct name *fsm)\
    {\
        TRAPNULL(fsm);\
        TRAP(fsm->next<0, no_transition, "fsm->next==%i", (int)fsm->next);\
        fsm->current = fsm->next;\
        fsm->next = -1;\
        if(fsm->enter_handlers)\
        {\
            if(fsm->enter_handlers[fsm->current])\
            {\
                fsm->next = fsm->enter_handlers[fsm->current](fsm->context, fsm->current);\
                TRAP(fsm->next>=fsm->n_states, fsm_next_range, "fsm->next==%i", (int)fsm->next);\
            }\
        }\
        return (fsm->next >= 0);\
    trap_fsm_next_range:\
    trap_no_transition:\
    trap_fsm_null:\
        return -1;\
    }

/*sm_state_t sm_current(const struct sm *sm);
sm_state_t sm_get_pending(const struct sm *sm);*/

#endif
