#ifndef LGK_QUEUE_H
#define LGK_QUEUE_H

#include <stdint.h>
#include <time.h>
#include <threads.h>
#include <lgk/tnt.h>
#include <lgk/time_ms.h>
#include <lgk/threads.h>

// TODO: don't return threads.h enum error codes - switch to common contract (0:OK, <0: critical failure, >0: partial failure) and use int_fast8_t return type
// not changing now because bikebox will need to be adapted

#define QUEUE_STRUCT(name, container)\
    struct name\
    {\
        struct container container;\
        mtx_t mutex;\
        cnd_t cnd_readable;\
        cnd_t cnd_writable;\
    }

#define QUEUE_INIT_HEADER(name, container) int name##_init(struct name *q, typeof(q->container.buffer) buffer, typeof(q->container.size) size, int_fast8_t timed)
#define QUEUE_INIT(name, container)\
    QUEUE_INIT_HEADER(name, container)\
    {\
        TRAPVNULL(q);\
        int status = container##_init(&q->container, buffer, size);\
        TRAPF(status, container##_init, status, "i");\
        status = mtx_init(&q->mutex, timed ? mtx_timed : mtx_plain);\
        TRAPFT(status!=thrd_success, mtx_init, status);\
        status = cnd_init(&q->cnd_readable);\
        TRAPFTS(status!=thrd_success, cnd_init, readable, status);\
        status = cnd_init(&q->cnd_writable);\
        TRAPFTS(status!=thrd_success, cnd_init, writable, status);\
        return status;\
    trap_cnd_init_writable:\
        cnd_destroy(&q->cnd_readable);\
    trap_cnd_init_readable:\
        mtx_destroy(&q->mutex);\
    trap_mtx_init:\
    trap_##container##_init:\
    trap_q_null:\
        return thrd_error;\
    }

#define QUEUE_INIT_PREFILLED_HEADER(name, container) int name##_init_prefilled(struct name *q, typeof(q->container.buffer) buffer, typeof(q->container.size) size, typeof(q->container.used) used, int_fast8_t timed)
#define QUEUE_INIT_PREFILLED(name, container)\
    QUEUE_INIT_PREFILLED_HEADER(name, container)\
    {\
        TRAPVNULL(q);\
        int status = container##_init_prefilled(&q->container, buffer, size, used);\
        TRAPF(status, container##_init_prefilled, status, "i");\
        status = mtx_init(&q->mutex, timed ? mtx_timed : mtx_plain);\
        TRAPFT(status!=thrd_success, mtx_init, status);\
        status = cnd_init(&q->cnd_readable);\
        TRAPFTS(status!=thrd_success, cnd_init, readable, status);\
        status = cnd_init(&q->cnd_writable);\
        TRAPFTS(status!=thrd_success, cnd_init, writable, status);\
        return status;\
    trap_cnd_init_writable:\
        cnd_destroy(&q->cnd_readable);\
    trap_cnd_init_readable:\
        mtx_destroy(&q->mutex);\
    trap_mtx_init:\
    trap_##container##_init_prefilled:\
    trap_q_null:\
        return thrd_error;\
    }

#define QUEUE_CLOSE_HEADER(name, container) int name##_close(struct name *q)
#define QUEUE_CLOSE(name, container)\
    QUEUE_CLOSE_HEADER(name, container)\
    {\
        TRAPVNULL(q);\
        if(q->container.used) WARN("queue not empty");\
        cnd_destroy(&q->cnd_writable);\
        cnd_destroy(&q->cnd_readable);\
        mtx_destroy(&q->mutex);\
        return thrd_success;\
    trap_q_null:\
        return thrd_error;\
    }

#define QUEUE_PUSH_HEADER(name, container) int name##_push(struct name *q, const typeof(*q->container.buffer) *item, int timeout_ms)
#define QUEUE_PUSH(name, container)\
    QUEUE_PUSH_HEADER(name, container)\
    {\
        TRAPVNULL(q);\
        int status = thrd_error;\
        struct timespec ts;\
        struct timespec *ts_ptr = NULL;\
        if(timeout_ms >= 0)\
        {\
            status = timespec_get_offset_ms((ts_ptr=&ts), TIME_UTC, timeout_ms);\
            TRAPF(status, timespec_get_offset_ms, status, "i");\
        }\
        status = mtx_timedlock_ts(&q->mutex, ts_ptr);\
        if(status == thrd_timedout) return status;\
        TRAPFT(status!=thrd_success, mtx_timedlock_ts, status);\
        while((status==thrd_success) && (q->container.used==q->container.size)) status = cnd_timedwait_ts(&q->cnd_writable, &q->mutex, ts_ptr);\
        if(status == thrd_success)\
        {\
            int status_rb = container##_push(&q->container, item);\
            if(!status_rb)\
            {\
                status = cnd_signal(&q->cnd_readable);\
                if(status != thrd_success) CRITFT(cnd_signal, status);\
            }\
            else\
            {\
                status = thrd_error;\
                CRITF(container##_push, status_rb, "i");\
            }\
        }\
        else\
        {\
            if(status != thrd_timedout) CRITFT(cnd_timedwait_ts, status);\
        }\
        int status_unlock = mtx_unlock(&q->mutex);\
        if(status_unlock != thrd_success) CRITFT(mtx_unlock, status_unlock);\
        return (status == thrd_success) ? status_unlock : status;\
    trap_mtx_timedlock_ts:\
    trap_timespec_get_offset_ms:\
    trap_q_null:\
        return thrd_error;\
    }

#define QUEUE_TRYPUSH_HEADER(name, container) int name##_trypush(struct name *q, const typeof(*q->container.buffer) *item, int mutex_timeout_ms)
#define QUEUE_TRYPUSH(name, container)\
    QUEUE_TRYPUSH_HEADER(name, container)\
    {\
        TRAPVNULL(q);\
        int status = thrd_error;\
        struct timespec ts;\
        struct timespec *ts_ptr = NULL;\
        if(mutex_timeout_ms >= 0)\
        {\
            status = timespec_get_offset_ms((ts_ptr=&ts), TIME_UTC, mutex_timeout_ms);\
            TRAPF(status, timespec_get_offset_ms, status, "i");\
        }\
        status = mtx_timedlock_ts(&q->mutex, ts_ptr);\
        if(status == thrd_timedout) return status;\
        TRAPFT(status!=thrd_success, mtx_timedlock_ts, status);\
        int status_rb = container##_push(&q->container, item);\
        if(!status_rb)\
        {\
            status = cnd_signal(&q->cnd_readable);\
            if(status != thrd_success) CRITFT(cnd_signal, status);\
        }\
        else\
        {\
            if(status_rb > 0)\
            {\
                status = thrd_busy;\
            }\
            else\
            {\
                status = thrd_error;\
                CRITF(container##_push, status_rb, "i");\
            }\
        }\
        int status_unlock = mtx_unlock(&q->mutex);\
        if(status_unlock != thrd_success) CRITFT(mtx_unlock, status_unlock);\
        return (status == thrd_success) ? status_unlock : status;\
    trap_mtx_timedlock_ts:\
    trap_timespec_get_offset_ms:\
    trap_q_null:\
        return thrd_error;\
    }

#define QUEUE_POP_HEADER(name, container) int name##_pop(struct name *q, typeof(*q->container.buffer) *item, int timeout_ms)
#define QUEUE_POP(name, container)\
    QUEUE_POP_HEADER(name, container)\
    {\
        TRAPVNULL(q);\
        TRAPVNULL(item);\
        int status = thrd_error;\
        struct timespec ts;\
        struct timespec *ts_ptr = NULL;\
        if(timeout_ms >= 0)\
        {\
            status = timespec_get_offset_ms((ts_ptr=&ts), TIME_UTC, timeout_ms);\
            TRAPF(status, timespec_get_offset_ms, status, "i");\
        }\
        status = mtx_timedlock_ts(&q->mutex, ts_ptr);\
        if(status == thrd_timedout) return status;\
        TRAPFT(status!=thrd_success, mtx_timedlock_ts, status);\
        while((status==thrd_success) && !q->container.used) status = cnd_timedwait_ts(&q->cnd_readable, &q->mutex, ts_ptr);\
        if(status == thrd_success)\
        {\
            int status_rb = container##_pop(&q->container, item);\
            if(!status_rb)\
            {\
                status = cnd_signal(&q->cnd_writable);\
                if(status != thrd_success) CRITFT(cnd_signal, status);\
            }\
            else\
            {\
                status = thrd_error;\
                CRITF(container##_pop, status_rb, "i");\
            }\
        }\
        else\
        {\
            if(status != thrd_timedout) CRITFT(cnd_timedwait_ts, status);\
        }\
        int status_unlock = mtx_unlock(&q->mutex);\
        if(status_unlock != thrd_success) CRITFT(mtx_unlock, status_unlock);\
        return (status == thrd_success) ? status_unlock : status;\
    trap_mtx_timedlock_ts:\
    trap_timespec_get_offset_ms:\
    trap_item_null:\
    trap_q_null:\
        return thrd_error;\
    }

#define QUEUE_TRYPOP_HEADER(name, container) int name##_trypop(struct name *q, typeof(*q->container.buffer) *item, int mutex_timeout_ms)
#define QUEUE_TRYPOP(name, container)\
    QUEUE_TRYPOP_HEADER(name, container)\
    {\
        TRAPVNULL(q);\
        TRAPVNULL(item);\
        int status = thrd_error;\
        struct timespec ts;\
        struct timespec *ts_ptr = NULL;\
        if(mutex_timeout_ms >= 0)\
        {\
            status = timespec_get_offset_ms((ts_ptr=&ts), TIME_UTC, mutex_timeout_ms);\
            TRAPF(status, timespec_get_offset_ms, status, "i");\
        }\
        status = mtx_timedlock_ts(&q->mutex, ts_ptr);\
        if(status == thrd_timedout) return status;\
        TRAPFT(status!=thrd_success, mtx_timedlock_ts, status);\
        int status_rb = container##_pop(&q->container, item);\
        if(!status_rb)\
        {\
            status = cnd_signal(&q->cnd_writable);\
            if(status != thrd_success) CRITFT(cnd_signal, status);\
        }\
        else\
        {\
            if(status_rb > 0)\
            {\
                status = thrd_busy;\
            }\
            else\
            {\
                status = thrd_error;\
                CRITF(container##_pop, status_rb, "i");\
            }\
        }\
        int status_unlock = mtx_unlock(&q->mutex);\
        if(status_unlock != thrd_success) CRITFT(mtx_unlock, status_unlock);\
        return (status == thrd_success) ? status_unlock : status;\
    trap_mtx_timedlock_ts:\
    trap_timespec_get_offset_ms:\
    trap_item_null:\
    trap_q_null:\
        return thrd_error;\
    }

//TODO: QUEUE_PEEK: 

#endif
