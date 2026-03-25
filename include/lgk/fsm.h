#ifndef LGK_FSM_H
#define LGK_FSM_H

#include <stdint.h>
#include <lgk/tnt.h>
#include <lgk/util.h>

/** TODO documentation
  * - type_state has to be a signed integer type (preferrably an enum based on a signed type)
  * - state "0" is init/reset state, fsm will always start in that state, handlers cannot initiate transition to that state.
  * - if using an enum as type_state (recommended), it shall not have "gaps", shall define only positive values and zero; negative values are reserved to signal errors
  * - transition to reset/init state shall be performed from "outside" by calling xyz_reset() (usually in error case)
  * 
  */

#define FSM_TYPES(name, type_context, type_state, type_event, type_event_data)\
    ASSERT_SIGNED(type_state);\
    typedef type_state name##_enter_handler(type_context context, type_state state);\
    typedef type_state name##_event_handler(type_context context, type_state state, type_event event, type_event_data event_data);\
    struct name\
    {\
	    type_state current;\
        type_state next;\
    	type_context context;\
        name##_enter_handler *const *enter_handlers;\
        name##_event_handler *const *event_handlers;\
    	type_state n_states;\
    };

#define FSM_PROTOTYPES(name, type_context, type_state, type_event, type_event_data)\
    type_state name##_event(struct name *fsm, type_event event, type_event_data event_data);\
    type_state name##_step(struct name *fsm);\
    type_state name##_process(struct name *fsm);\
    int_least8_t name##_event_process(struct name *fsm, type_event event, type_event_data event_data);\
    type_state name##_reset(struct name *fsm);\
    type_state name##_init(struct name *fsm, type_context context, name##_enter_handler *const *enter_handlers, name##_event_handler *const *event_handlers, type_state n_states);

#define FSM_FUNCTION_EVENT(name, type_state, type_event, type_event_data)\
    type_state name##_event(struct name *fsm, type_event event, type_event_data event_data)\
    {\
        TRAPNULL(fsm);\
        TRAP(fsm->next, next_not_zero, "fsm->next==%i", (int)fsm->next);\
        TRAP((fsm->current<0)||(fsm->current>=fsm->n_states), current_invalid, "fsm->current==%i", (int)fsm->current);\
        if(fsm->event_handlers) if(fsm->event_handlers[fsm->current]) return fsm->next = fsm->event_handlers[fsm->current](fsm->context, fsm->current, event, event_data);\
        WARN("no event handler for event %i", (int)event);\
        return 0;\
    trap_current_invalid:\
    trap_next_not_zero:\
    trap_fsm_null:\
        return -1;\
    }

#define FSM_FUNCTION_STEP(name, type_state)\
    type_state name##_step(struct name *fsm)\
    {\
        TRAPNULL(fsm);\
        TRAP((fsm->next<=0)||(fsm->next>=fsm->n_states), next_invalid, "fsm->next==%i", (int)fsm->next);\
        TRAPNULL_L(fsm->enter_handlers, enter_handlers);\
        TRAPNULL_L(fsm->enter_handlers[fsm->next], enter_handler);\
        fsm->current = fsm->next;\
        return fsm->next = fsm->enter_handlers[fsm->current](fsm->context, fsm->current);\
    trap_enter_handler_null:\
    trap_enter_handlers_null:\
    trap_next_invalid:\
    trap_fsm_null:\
        return -1;\
    }

#define FSM_FUNCTION_PROCESS(name, type_state)\
    type_state name##_process(struct name *fsm)\
    {\
        type_state next = 0;\
        do\
        {\
            next = name##_step(fsm);\
            TRAPF(next<0, name##_step, "%i", (int)next);\
        } while(next);\
        return 0;\
    trap_##name##_step:\
        return -1;\
    }

#define FSM_FUNCTION_EVENT_PROCESS(name, type_state, type_event, type_event_data)\
    int_least8_t name##_event_process(struct name *fsm, type_event event, type_event_data event_data)\
    {\
        type_state next = name##_event(fsm, event, event_data);\
        TRAPF(next<0, name##_event, "%i", (int)next);\
        if(next)\
        {\
            next = name##_process(fsm);\
            TRAPF(next, name##_process, "%i", (int)next);\
        }\
        return 0;\
    trap_##name##_process:\
    trap_##name##_event:\
        return -1;\
    }

#define FSM_FUNCTION_RESET(name, type_state)\
    type_state name##_reset(struct name *fsm)\
    {\
        TRAPNULL(fsm);\
        fsm->current = 0;\
        fsm->next = 0;\
        TRAPNULL_L(fsm->enter_handlers, enter_handlers);\
        TRAPNULL_L(fsm->enter_handlers[fsm->next], enter_handler);\
        fsm->current = 0;\
        fsm->next = 0;\
        return fsm->next = fsm->enter_handlers[0](fsm->context, 0);\
    trap_enter_handler_null:\
    trap_enter_handlers_null:\
    trap_fsm_null:\
        return -1;\
    }

#define FSM_FUNCTION_INIT(name, type_context, type_state)\
    type_state name##_init(struct name *fsm, type_context context, name##_enter_handler *const *enter_handlers, name##_event_handler *const *event_handlers, type_state n_states)\
    {\
        TRAPNULL(fsm);\
        TRAPNULL(enter_handlers);\
        fsm->context = context;\
        fsm->enter_handlers = enter_handlers;\
        fsm->event_handlers = event_handlers;\
        fsm->n_states = n_states;\
        return name##_reset(fsm);\
    trap_enter_handlers_null:\
    trap_fsm_null:\
        return -1;\
    }

#define FSM_FUNCTIONS_STATIC(name, type_context, type_state, type_event, type_event_data)\
    static FSM_FUNCTION_EVENT(name, type_state, type_event, type_event_data)\
    static FSM_FUNCTION_STEP(name, type_state)\
    static FSM_FUNCTION_PROCESS(name, type_state)\
    [[maybe_unused]] static FSM_FUNCTION_EVENT_PROCESS(name, type_state, type_event, type_event_data)\
    static FSM_FUNCTION_RESET(name, type_state)\
    static FSM_FUNCTION_INIT(name, type_context, type_state)

#define FSM_STATIC(name, type_context, type_state, type_event, type_event_data)\
    FSM_TYPES(name, type_context, type_state, type_event, type_event_data)\
    FSM_FUNCTIONS_STATIC(name, type_context, type_state, type_event, type_event_data)

#endif
