#ifndef LGK_QUEUE_GENERIC_H
#define LGK_QUEUE_GENERIC_H

#include <stdint.h>
#include <time.h>
#include <threads.h>
#include <lgk_tnt.h>
#include <lgk_timespec.h>

#define QUEUE_FLAG_UNTIMED  0x01

#define QUEUE_PROTOTYPES(type_data, type_size, name)\
    struct queue_##name;\
    int queue_##name##_init(struct queue_##name *q, type_data *buffer, type_size size, uint_fast8_t flags);\
    int queue_##name##_init_prefilled(struct queue_##name *q, type_data *buffer, type_size size, type_size used, uint_fast8_t flags);\
    void queue_##name##_close(struct queue_##name *q);\
    int queue_##name##_push(struct queue_##name *q, type_data item, unsigned timeout_ms);\
    int queue_##name##_pop(struct queue_##name *q, type_data *item, unsigned timeout_ms);\

#define QUEUE_STRUCT(type_data, type_size, name)\
    struct queue_##name\
    {\
        type_data *buffer;\
        type_size size;\
        type_size used;\
        type_size i_write;\
        type_size i_read;\
        mtx_t mutex;\
        cnd_t cnd_readable;\
        cnd_t cnd_writable;\
    };

#define QUEUE_FUNCTIONS_INTERNAL(sclass, type_data, type_size, name)\
    sclass int queue_##name##_init(struct queue_##name *q, type_data *buffer, type_size size, uint_fast8_t flags)\
    {\
        q->buffer = buffer;\
        q->size = size;\
        q->used = q->i_read = q->i_write = 0;\
        int status = mtx_init(&q->mutex, (flags & QUEUE_FLAG_UNTIMED) ? mtx_plain : mtx_timed);\
        TRAP(status!=thrd_success, mtx_init, "mtx_init(): %i", status);\
        status = cnd_init(&q->cnd_readable);\
        TRAP(status!=thrd_success, cnd_init_readable, "cnd_init(): %i", status);\
        status = cnd_init(&q->cnd_writable);\
        TRAP(status!=thrd_success, cnd_init_writable, "cnd_init(): %i", status);\
        return thrd_success;\
    trap_cnd_init_writable:\
        cnd_destroy(&q->cnd_readable);\
    trap_cnd_init_readable:\
        mtx_destroy(&q->mutex);\
    trap_mtx_init:\
        return status;\
    }\
    \
    sclass int queue_##name##_init_prefilled(struct queue_##name *q, type_data *buffer, type_size size, type_size used, uint_fast8_t flags)\
    {\
        TRAP(used>size, used, "used > size");\
        int status = queue_##name##_init(q, buffer, size, flags);\
        TRAP(status!=thrd_success, init, "queue_init(): %i", status);\
        q->used = used;\
        if(used<size) q->i_write = used;\
        return thrd_success;\
    trap_init:\
        return status;\
    trap_used:\
        return thrd_error;\
    }\
    \
    sclass void queue_##name##_close(struct queue_##name *q)\
    {\
        if(q->i_read != q->i_write) TRACE(TRACE_LEVEL_WARNING, "message queue not empty");\
        cnd_destroy(&q->cnd_writable);\
        cnd_destroy(&q->cnd_readable);\
        mtx_destroy(&q->mutex);\
    }\
    \
    static int queue_##name##_wait_write(struct queue_##name *q, type_size min_free)\
    {\
        int status = mtx_lock(&q->mutex);\
        TRAP(status==thrd_error, mtx_lock, "mtx_lock(): %i", status);\
        while((status == thrd_success) && ((q->size - q->used) < min_free)) status = cnd_wait(&q->cnd_writable, &q->mutex);\
        TRAP(status==thrd_error, cnd_wait, "cnd_wait(): %i", status);\
        return status;\
    trap_cnd_wait:\
    trap_mtx_lock:\
        return thrd_error;\
    }\
    \
    static int queue_##name##_timedwait_write(struct queue_##name *q, type_size min_free, unsigned timeout_ms)\
    {\
        struct timespec ts;\
        timespec_get_offset_ms(&ts, TIME_UTC, timeout_ms);\
        /*TODO error reporting */\
        int status = mtx_timedlock(&q->mutex, &ts);\
        TRAP(status==thrd_error, mtx_timedlock, "mtx_timedlock(): %i", status);\
        while((status == thrd_success) && ((q->size - q->used) < min_free)) status = cnd_timedwait(&q->cnd_writable, &q->mutex, &ts);\
        TRAP(status==thrd_error, cnd_timedwait, "cnd_timedwait(): %i", status);\
        return status;\
    trap_cnd_timedwait:\
    trap_mtx_timedlock:\
        return thrd_error;\
    }\
    \
    sclass int queue_##name##_push(struct queue_##name *q, type_data item, unsigned timeout_ms)\
    {\
        int status = timeout_ms ? queue_##name##_timedwait_write(q, 1, timeout_ms) : queue_##name##_wait_write(q, 1);\
        TRAP(status==thrd_error, queue_timedwait_write, "queue_"#name"_timedwait_write(): %i", status);\
        if(status == thrd_success)\
        {\
            q->buffer[q->i_write++] = item;\
            if(q->i_write > q->size) q->i_write = 0;\
            q->used++;\
            status = cnd_signal(&q->cnd_readable);\
            TRAP(status==thrd_error, cnd_signal, "cnd_signal(): %i", status);\
            status = mtx_unlock(&q->mutex);\
            TRAP(status==thrd_error, mtx_unlock, "mtx_unlock(): %i", status);\
        }\
        return status;\
    trap_mtx_unlock:\
    trap_cnd_signal:\
        mtx_unlock(&q->mutex);\
    trap_queue_timedwait_write:\
        return thrd_error;\
    }\
    \
    static int queue_##name##_wait_read(struct queue_##name *q)\
    {\
        int status = mtx_lock(&q->mutex);\
        TRAP(status==thrd_error, mtx_lock, "mtx_lock(): %i", status);\
        while((status == thrd_success) && (q->used == 0)) status = cnd_wait(&q->cnd_readable, &q->mutex);\
        TRAP(status==thrd_error, cnd_wait, "cnd_wait(): %i", status);\
        return status;\
    trap_cnd_wait:\
    trap_mtx_lock:\
        return thrd_error;\
    }\
    \
    static int queue_##name##_timedwait_read(struct queue_##name *q, unsigned timeout_ms)\
    {\
        struct timespec ts;\
        timespec_get_offset_ms(&ts, TIME_UTC, timeout_ms);\
        /*TODO error reporting */\
        int status = mtx_timedlock(&q->mutex, &ts);\
        TRAP(status==thrd_error, mtx_timedlock, "mtx_timedlock(): %i", status);\
        while((status == thrd_success) && (q->used == 0)) status = cnd_timedwait(&q->cnd_readable, &q->mutex, &ts);\
        TRAP(status==thrd_error, cnd_timedwait, "cnd_timedwait(): %i", status);\
        return status;\
    trap_cnd_timedwait:\
    trap_mtx_timedlock:\
        return thrd_error;\
    }\
    \
    sclass int queue_##name##_pop(struct queue_##name *q, type_data *item, unsigned timeout_ms)\
    {\
        int status = timeout_ms ? queue_##name##_timedwait_read(q, timeout_ms) : queue_##name##_wait_read(q);\
        TRAP(status==thrd_error, queue_timedwait_read, "queue_"#name"_timedwait_read(): %i", status);\
        if(status == thrd_success)\
        {\
            *item = q->buffer[q->i_read++];\
            if(q->i_read > q->size) q->i_read = 0;\
            q->used--;\
            status = cnd_signal(&q->cnd_writable);\
            TRAP(status==thrd_error, cnd_signal, "cnd_signal(): %i", status);\
            status = mtx_unlock(&q->mutex);\
            TRAP(status==thrd_error, mtx_unlock, "mtx_unlock(): %i", status);\
        }\
        return status;\
    trap_mtx_unlock:\
    trap_cnd_signal:\
        mtx_unlock(&q->mutex);\
    trap_queue_timedwait_read:\
        return thrd_error;\
    }

#define QUEUE_FUNCTIONS(type_data, type_size, name) QUEUE_FUNCTIONS_INTERNAL(, type_data, type_size, name)
#define QUEUE_FUNCTIONS_STATIC(type_data, type_size, name) QUEUE_FUNCTIONS_INTERNAL(static, type_data, type_size, name)

#endif
