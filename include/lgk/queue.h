#ifndef LGK_QUEUE_H
#define LGK_QUEUE_H

#include <stdint.h>
#include <time.h>
#include <threads.h>
#include <lgk/tnt.h>
#include <lgk/timespec.h>
#include <lgk/threads.h>

#define QUEUE_INIT_HEADER(type_data, type_size, name) int name##_init(struct name *q, type_data *buffer, type_size size, int_fast8_t timed)
#define QUEUE_INIT_PREFILLED_HEADER(type_data, type_size, name) int name##_init_prefilled(struct name *q, type_data *buffer, type_size size, type_size used, int_fast8_t timed)
#define QUEUE_CLOSE_HEADER(type_data, type_size, name) int name##_close(struct name *q)
#define QUEUE_PUSH_HEADER(type_data, type_size, name) int name##_push(struct name *q, type_data item, int timeout_ms)
#define QUEUE_POP_HEADER(type_data, type_size, name) int name##_pop(struct name *q, type_data *item, int timeout_ms)

#define QUEUE_STRUCT(type_data, type_size, name)\
    struct name\
    {\
        type_data *buffer;\
        type_size size;\
        type_size used;\
        type_size i_write;\
        type_size i_read;\
        mtx_t mutex;\
        cnd_t cnd_readable;\
        cnd_t cnd_writable;\
    }

#define QUEUE_INIT(type_data, type_size, name)\
    QUEUE_INIT_HEADER(type_data, type_size, name)\
    {\
        TRAPVNULL(q);\
        TRAPVNULL(buffer);\
        q->buffer = buffer;\
        q->size = size;\
        q->used = q->i_read = q->i_write = 0;\
        int status = mtx_init(&q->mutex, timed ? mtx_timed : mtx_plain);\
        TRAPF(status!=thrd_success, mtx_init, lgk_thrdstrerror(status), "s");\
        status = cnd_init(&q->cnd_readable);\
        TRAPFS(status!=thrd_success, cnd_init, readable, lgk_thrdstrerror(status), "s");\
        status = cnd_init(&q->cnd_writable);\
        TRAPFS(status!=thrd_success, cnd_init, writable, lgk_thrdstrerror(status), "s");\
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
    }

#define QUEUE_INIT_PREFILLED(type_data, type_size, name)\
    QUEUE_INIT_PREFILLED_HEADER(type_data, type_size, name)\
    {\
        TRAPVNULL(q);\
        TRAP(used>size, used, "used > size");\
        int status = name##_init(q, buffer, size, timed);\
        TRAPF(status!=thrd_success, name##_init, lgk_thrdstrerror(status), "s");\
        q->used = used;\
        if(used<size) q->i_write = used;\
        return status;\
    trap_##name##_init:\
        return status;\
    trap_used:\
    trap_q_null:\
        return thrd_error;\
    }
    
#define QUEUE_CLOSE(type_data, type_size, name)\
    QUEUE_CLOSE_HEADER(type_data, type_size, name)\
    {\
        TRAPVNULL(q);\
        if(q->i_read != q->i_write) WARN("queue not empty");\
        cnd_destroy(&q->cnd_writable);\
        cnd_destroy(&q->cnd_readable);\
        mtx_destroy(&q->mutex);\
        return thrd_success;\
    trap_q_null:\
        return thrd_error;\
    }

#define QUEUE_PUSH(type_data, type_size, name)\
    QUEUE_PUSH_HEADER(type_data, type_size, name)\
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
        TRAPF(status!=thrd_success, mtx_timedlock_ts, lgk_thrdstrerror(status), "s");\
        while((status==thrd_success) && (q->size==q->used)) status = cnd_timedwait_ts(&q->cnd_writable, &q->mutex, ts_ptr);\
        if(status == thrd_success)\
        {\
            if(q->i_write < q->size)\
            {\
                q->buffer[q->i_write++] = item;\
                if(q->i_write == q->size) q->i_write = 0;\
                q->used++;\
                status = cnd_signal(&q->cnd_readable);\
                if(status != thrd_success) CRITF(cnd_signal, lgk_thrdstrerror(status), "s");\
            }\
            else\
            {\
                CRIT("q->i_write==%u, q->size==%u", q->i_write, q->size);\
            }\
        }\
        else\
        {\
            if(status != thrd_timedout) CRITF(cnd_timedwait_ts, lgk_thrdstrerror(status), "s");\
        }\
        int status_unlock = mtx_unlock(&q->mutex);\
        if(status_unlock != thrd_success) CRITF(mtx_unlock, lgk_thrdstrerror(status_unlock), "s");\
        return (status == thrd_success) ? status_unlock : status;\
    trap_mtx_timedlock_ts:\
    trap_timespec_get_offset_ms:\
    trap_q_null:\
        return thrd_error;\
    }

#define QUEUE_POP(type_data, type_size, name)\
    QUEUE_POP_HEADER(type_data, type_size, name)\
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
        TRAPF(status!=thrd_success, mtx_timedlock_ts, lgk_thrdstrerror(status), "s");\
        while((status==thrd_success) && !q->used) status = cnd_timedwait_ts(&q->cnd_readable, &q->mutex, ts_ptr);\
        if(status == thrd_success)\
        {\
            if(q->i_read<q->size)\
            {\
                *item = q->buffer[q->i_read++];\
                if(q->i_read == q->size) q->i_read = 0;\
                q->used--;\
                status = cnd_signal(&q->cnd_writable);\
                if(status != thrd_success) CRITF(cnd_signal, lgk_thrdstrerror(status), "s");\
            }\
            else\
            {\
                CRIT("q->i_read==%u, q->size==%u", q->i_read, q->size);\
            }\
        }\
        else\
        {\
            if(status != thrd_timedout) CRITF(cnd_timedwait_ts, lgk_thrdstrerror(status), "s");\
        }\
        int status_unlock = mtx_unlock(&q->mutex);\
        if(status_unlock != thrd_success) CRITF(mtx_unlock, lgk_thrdstrerror(status), "s");\
        return (status == thrd_success) ? status_unlock : status;\
    trap_mtx_timedlock_ts:\
    trap_timespec_get_offset_ms:\
    trap_item_null:\
    trap_q_null:\
        return thrd_error;\
    }

#endif
