#ifndef LGK_FIFO_H
#define LGK_FIFO_H

#include <string.h>
#include <lgk/tnt.h>

#define FIFO_STRUCT(name, type_data, type_size)\
    struct name\
    {\
        type_data *buffer;\
        type_size size;\
        type_size used;\
        type_size i_write;\
        type_size i_read;\
    }

#define FIFO_INIT_HEADER(name) int name##_init(struct name *rb, typeof(rb->buffer) buffer, typeof(rb->size) size)
#define FIFO_INIT(name)\
    FIFO_INIT_HEADER(name)\
    {\
        TRAPVNULL(rb);\
        rb->buffer = buffer;\
        rb->size = size;\
        rb->used = rb->i_read = rb->i_write = 0;\
        return 0;\
    trap_rb_null:\
        return -1;\
    }

#define FIFO_INIT_PREFILLED_HEADER(name) int name##_init_prefilled(struct name *rb, typeof(rb->buffer) buffer, typeof(rb->size) size, typeof(rb->used) used)
#define FIFO_INIT_PREFILLED(name)\
    FIFO_INIT_PREFILLED_HEADER(name)\
    {\
        TRAPVNULL(rb);\
        TRAP(used>size, used, "used > size");\
        int status = name##_init(rb, buffer, size);\
        TRAPF(status, name##_init, status, "i");\
        rb->used = used;\
        if(used<size) rb->i_write = used;\
        return status;\
    trap_##name##_init:\
        return status;\
    trap_used:\
    trap_rb_null:\
        return -1;\
    }

#define FIFO_PUSH_HEADER(name) int name##_push(struct name *rb, const typeof(*rb->buffer) *restrict item)
#define FIFO_PUSH(name)\
    FIFO_PUSH_HEADER(name)\
    {\
        TRAPVNULL(rb);\
        if(rb->used == rb->size) return 1;\
        TRAPXXGTE(rb->i_write, rb->size, i_write, size, "u");\
        TRAPXNULL(rb->buffer, buffer);\
        rb->buffer[rb->i_write++] = *item;\
        if(rb->i_write == rb->size) rb->i_write = 0;\
        rb->used++;\
        return 0;\
    trap_buffer_null:\
    trap_i_write_gte_size:\
    trap_rb_null:\
        return -1;\
    }

#define FIFO_POP_HEADER(name) int name##_pop(struct name *rb, typeof(*rb->buffer) *item)
#define FIFO_POP(name)\
    FIFO_POP_HEADER(name)\
    {\
        TRAPVNULL(rb);\
        TRAPVNULL(item);\
        if(!rb->used) return 1;\
        TRAPXXGTE(rb->i_read, rb->size, i_read, size, "u");\
        TRAPXNULL(rb->buffer, buffer);\
        *item = rb->buffer[rb->i_read++];\
        if(rb->i_read == rb->size) rb->i_read = 0;\
        rb->used--;\
        return 0;\
    trap_buffer_null:\
    trap_i_read_gte_size:\
    trap_item_null:\
    trap_rb_null:\
        return -1;\
    }

/* TODO: return value is `(int)n` where `n` is a `type_size` count of items transferred;
 * if `type_size` is wider than `int` (or just large-valued) and `count`/`available` exceeds
 * INT_MAX, this cast silently truncates/wraps. Unlikely in practice (queue/ring buffer sizes
 * are not expected to approach INT_MAX items), but worth revisiting if `type_size` is ever
 * instantiated with a wide type or very large capacities. */
#define FIFO_WRITE_HEADER(name) int name##_write(struct name *rb, const typeof(*rb->buffer) *items, typeof(rb->size) count)
#define FIFO_WRITE(name)\
    FIFO_WRITE_HEADER(name)\
    {\
        if(!count) return 0;\
        TRAPVNULL(rb);\
        TRAPVNULL(items);\
        TRAPXNULL(rb->buffer, buffer);\
        typeof(rb->size) available = rb->size - rb->used;\
        typeof(rb->size) n = (count < available) ? count : available;\
        typeof(rb->size) to_end = rb->size - rb->i_write;\
        typeof(rb->size) first = (n < to_end) ? n : to_end;\
        memcpy(&rb->buffer[rb->i_write], items, first * sizeof(*rb->buffer));\
        typeof(rb->size) second = n - first;\
        if(second) memcpy(rb->buffer, items + first, second * sizeof(*rb->buffer));\
        rb->i_write += n;\
        if(rb->i_write >= rb->size) rb->i_write -= rb->size;\
        rb->used += n;\
        return (int)n;\
    trap_buffer_null:\
    trap_items_null:\
    trap_rb_null:\
        return -1;\
    }

/* TODO: same caveat as FIFO_WRITE - `(int)n` can silently truncate/wrap if `type_size`
 * is wide enough and `count`/`used` exceeds INT_MAX. Unlikely, but worth revisiting. */
#define FIFO_READ_HEADER(name) int name##_read(struct name *rb, typeof(*rb->buffer) *items, typeof(rb->size) count)
#define FIFO_READ(name)\
    FIFO_READ_HEADER(name)\
    {\
        if(!count) return 0;\
        TRAPVNULL(rb);\
        TRAPVNULL(items);\
        TRAPXNULL(rb->buffer, buffer);\
        typeof(rb->size) n = (count < rb->used) ? count : rb->used;\
        typeof(rb->size) to_end = rb->size - rb->i_read;\
        typeof(rb->size) first = (n < to_end) ? n : to_end;\
        memcpy(items, &rb->buffer[rb->i_read], first * sizeof(*rb->buffer));\
        typeof(rb->size) second = n - first;\
        if(second) memcpy(items + first, rb->buffer, second * sizeof(*rb->buffer));\
        rb->i_read += n;\
        if(rb->i_read >= rb->size) rb->i_read -= rb->size;\
        rb->used -= n;\
        return (int)n;\
    trap_buffer_null:\
    trap_items_null:\
    trap_rb_null:\
        return -1;\
    }

#endif
