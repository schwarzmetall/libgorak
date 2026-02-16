#ifndef LGK_QUEUE_GENERIC_H
#define LGK_QUEUE_GENERIC_H

#include <stdint.h>
#include <time.h>
#include <threads.h>
#include <lgk_tnt.h>
#include <lgk_timespec.h>
#include <lgk_threads.h>

#define QUEUE_FLAG_UNTIMED  0x01

#define QUEUE_PROTOTYPES(type_data, type_size, name)\
    struct queue_##name;\
    int queue_##name##_init(struct queue_##name *q, type_data *buffer, type_size size, uint_fast8_t flags);\
    int queue_##name##_init_prefilled(struct queue_##name *q, type_data *buffer, type_size size, type_size used, uint_fast8_t flags);\
    void queue_##name##_close(struct queue_##name *q);\
    int queue_##name##_push(struct queue_##name *q, type_data item, int timeout_ms);\
    int queue_##name##_pop(struct queue_##name *q, type_data *item, int timeout_ms);\

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
        TRAPNULL(q);\
        TRAPNULL(buffer);\
        q->buffer = buffer;\
        q->size = size;\
        q->used = q->i_read = q->i_write = 0;\
        int status = mtx_init(&q->mutex, (flags & QUEUE_FLAG_UNTIMED) ? mtx_plain : mtx_timed);\
        TRAPF(status!=thrd_success, mtx_init, "%i", status);\
        status = cnd_init(&q->cnd_readable);\
        TRAPFS(status!=thrd_success, cnd_init, readable, "%i", status);\
        status = cnd_init(&q->cnd_writable);\
        TRAPFS(status!=thrd_success, cnd_init, writable, "%i", status);\
        return status;\
    trap_cnd_init_writable:\
        cnd_destroy(&q->cnd_readable);\
    trap_cnd_init_readable:\
        mtx_destroy(&q->mutex);\
    trap_mtx_init:\
        return status;\
    trap_buffer_null:\
    trap_q_null:\
        return thrd_error;\
    }\
    \
    sclass int queue_##name##_init_prefilled(struct queue_##name *q, type_data *buffer, type_size size, type_size used, uint_fast8_t flags)\
    {\
        int status = thrd_error;\
        TRAP(used>size, used, "used > size");\
        status = queue_##name##_init(q, buffer, size, flags);\
        TRAP(status!=thrd_success, init, "queue_init(): %i", status);\
        q->used = used;\
        if(used<size) q->i_write = used;\
        return thrd_success;\
    trap_init:\
    trap_used:\
        return status;\
    }\
    \
    sclass void queue_##name##_close(struct queue_##name *q)\
    {\
        /*TODO TRAPNULL(q)*/\
        if(q->i_read != q->i_write) TRACE(TRACE_LEVEL_WARNING, "message queue not empty");\
        cnd_destroy(&q->cnd_writable);\
        cnd_destroy(&q->cnd_readable);\
        mtx_destroy(&q->mutex);\
    }\
    \
    static int queue_##name##_wait_write(struct queue_##name *q, type_size min_free, int timeout_ms)\
    {\
        TRAPNULL(q);\
        int status = thrd_error;\
        struct timespec ts;\
        struct timespec *ts_ptr = NULL;\
        if(timeout_ms >= 0)\
        {\
            status = timespec_get_offset_ms((ts_ptr=&ts), TIME_UTC, timeout_ms);\
            TRAPF(status!=TIME_UTC, timespec_get_offset_ms, "%i", status);\
        }\
        status = mtx_timedlock_ts(&q->mutex, ts_ptr);\
        if(status == thrd_timedout) return status;\
        TRAPF(status!=thrd_success, mtx_timedlock_ts, "%i", status);\
        while((status==thrd_success) && ((q->size-q->used)<min_free)) status = cnd_timedwait_ts(&q->cnd_writable, &q->mutex, ts_ptr);\
        if(THRD_FAIL(status)) FATALF(cnd_timedwait_ts, "%i", status);\
        if(status != thrd_success)\
        {\
            int status_unlock = mtx_unlock(&q->mutex);\
            if(status_unlock != thrd_success) FATALF(mtx_unlock, "%i", status_unlock);\
        }\
        return status;\
    trap_mtx_timedlock_ts:\
        return status;\
    trap_timespec_get_offset_ms:\
    trap_q_null:\
        return thrd_error;\
    }\
    \
    sclass int queue_##name##_push(struct queue_##name *q, type_data item, int timeout_ms)\
    {\
        TRAPNULL(q);\
        int status = queue_##name##_wait_write(q, 1, timeout_ms);\
        if(status == thrd_timedout) return status;\
        TRAPF(status!=thrd_success, queue_##name##_wait_write, "%i", status);\
        TRAP(q->i_write>=q->size, out_of_bounds, "q->i_write==%u, q->size==%u", q->i_write, q->size);\
        q->buffer[q->i_write++] = item;\
        if(q->i_write == q->size) q->i_write = 0;\
        q->used++;\
        status = cnd_signal(&q->cnd_readable);\
        if(status != thrd_success) FATALF(cnd_signal, "%i", status);\
        int status_unlock = mtx_unlock(&q->mutex);\
        if(status_unlock != thrd_success) FATALF(mtx_unlock, "%i", status_unlock);\
        return (status == thrd_success) ? status_unlock : status;\
    trap_queue_##name##_wait_write:\
        return status;\
    trap_out_of_bounds:\
    trap_q_null:;\
        return thrd_error;\
    }\
    \
    static int queue_##name##_wait_read(struct queue_##name *q, int timeout_ms)\
    {\
        TRAPNULL(q);\
        int status = thrd_error;\
        struct timespec ts;\
        struct timespec *ts_ptr = NULL;\
        if(timeout_ms >= 0)\
        {\
            status = timespec_get_offset_ms((ts_ptr=&ts), TIME_UTC, timeout_ms);\
            TRAPF(status!=TIME_UTC, timespec_get_offset_ms, "%i", status);\
        }\
        status = mtx_timedlock_ts(&q->mutex, ts_ptr);\
        if(status == thrd_timedout) return status;\
        TRAPF(status!=thrd_success, mtx_timedlock_ts, "%i", status);\
        while((status==thrd_success) && !q->used) status = cnd_timedwait_ts(&q->cnd_readable, &q->mutex, ts_ptr);\
        if(THRD_FAIL(status)) FATALF(cnd_timedwait_ts, "%i", status);\
        if(status != thrd_success)\
        {\
            int status_unlock = mtx_unlock(&q->mutex);\
            if(status_unlock != thrd_success) FATALF(mtx_unlock, "%i", status_unlock);\
        }\
        return status;\
    trap_mtx_timedlock_ts:\
        return status;\
    trap_timespec_get_offset_ms:\
    trap_q_null:\
        return thrd_error;\
    }\
    \
    sclass int queue_##name##_pop(struct queue_##name *q, type_data *item, int timeout_ms)\
    {\
        TRAPNULL(q);\
        TRAPNULL(item);\
        int status = queue_##name##_wait_read(q, timeout_ms);\
        if(status == thrd_timedout) return status;\
        TRAPF(status!=thrd_success, queue_##name##_wait_read, "%i", status);\
        TRAP(q->i_read>=q->size, out_of_bounds, "q->i_read==%u, q->size==%u", q->i_read, q->size);\
        *item = q->buffer[q->i_read++];\
        if(q->i_read == q->size) q->i_read = 0;\
        q->used--;\
        status = cnd_signal(&q->cnd_writable);\
        if(status != thrd_success) FATALF(cnd_signal, "%i", status);\
        int status_unlock = mtx_unlock(&q->mutex);\
        if(status_unlock != thrd_success) FATALF(mtx_unlock, "%i", status);\
        return (status == thrd_success) ? status_unlock : status;\
    trap_queue_##name##_wait_read:\
        return status;\
    trap_out_of_bounds:\
    trap_item_null:\
    trap_q_null:\
        return thrd_error;\
    }

#define QUEUE_FUNCTIONS(type_data, type_size, name) QUEUE_FUNCTIONS_INTERNAL(, type_data, type_size, name)
#define QUEUE_FUNCTIONS_STATIC(type_data, type_size, name) QUEUE_FUNCTIONS_INTERNAL(static, type_data, type_size, name)

#endif
