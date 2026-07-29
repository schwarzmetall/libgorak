#ifndef LGK_RINGBUF_H
#define LGK_RINGBUF_H

#include <string.h>
#include <lgk/tnt.h>

#define RINGBUF_INIT_HEADER(type_data, type_size, name) int name##_init(struct name *rb, type_data *buffer, type_size size)
#define RINGBUF_INIT_PREFILLED_HEADER(type_data, type_size, name) int name##_init_prefilled(struct name *rb, type_data *buffer, type_size size, type_size used)
#define RINGBUF_PUSH_HEADER(type_data, type_size, name) int name##_push(struct name *rb, type_data item)
#define RINGBUF_POP_HEADER(type_data, type_size, name) int name##_pop(struct name *rb, type_data *item)
#define RINGBUF_WRITE_HEADER(type_data, type_size, name) int name##_write(struct name *rb, type_data *items, type_size count)
#define RINGBUF_READ_HEADER(type_data, type_size, name) int name##_read(struct name *rb, type_data *items, type_size count)

#define RINGBUF_STRUCT(type_data, type_size, name)\
    struct name\
    {\
        type_data *buffer;\
        type_size size;\
        type_size used;\
        type_size i_write;\
        type_size i_read;\
    }

#define RINGBUF_INIT(type_data, type_size, name)\
    RINGBUF_INIT_HEADER(type_data, type_size, name)\
    {\
        TRAPVNULL(rb);\
        TRAPVNULL(buffer);\
        rb->buffer = buffer;\
        rb->size = size;\
        rb->used = rb->i_read = rb->i_write = 0;\
        return 0;\
    trap_buffer_null:\
    trap_rb_null:\
        return -1;\
    }

#define RINGBUF_INIT_PREFILLED(type_data, type_size, name)\
    RINGBUF_INIT_PREFILLED_HEADER(type_data, type_size, name)\
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

#define RINGBUF_PUSH(type_data, type_size, name)\
    RINGBUF_PUSH_HEADER(type_data, type_size, name)\
    {\
        TRAPVNULL(rb);\
        if(rb->used == rb->size) return 1;\
        TRAPXXGTE(rb->i_write, rb->size, i_write, size, "u");\
        rb->buffer[rb->i_write++] = item;\
        if(rb->i_write == rb->size) rb->i_write = 0;\
        rb->used++;\
        return 0;\
    trap_i_write_gte_size:\
    trap_rb_null:\
        return -1;\
    }

#define RINGBUF_POP(type_data, type_size, name)\
    RINGBUF_POP_HEADER(type_data, type_size, name)\
    {\
        TRAPVNULL(rb);\
        TRAPVNULL(item);\
        if(!rb->used) return 1;\
        TRAPXXGTE(rb->i_read, rb->size, i_read, size, "u");\
        *item = rb->buffer[rb->i_read++];\
        if(rb->i_read == rb->size) rb->i_read = 0;\
        rb->used--;\
        return 0;\
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
#define RINGBUF_WRITE(type_data, type_size, name)\
    RINGBUF_WRITE_HEADER(type_data, type_size, name)\
    {\
        TRAPVNULL(rb);\
        TRAPVNULL(items);\
        type_size available = rb->size - rb->used;\
        type_size n = (count < available) ? count : available;\
        type_size to_end = rb->size - rb->i_write;\
        type_size first = (n < to_end) ? n : to_end;\
        memcpy(&rb->buffer[rb->i_write], items, first * sizeof(type_data));\
        type_size second = n - first;\
        if(second) memcpy(rb->buffer, items + first, second * sizeof(type_data));\
        rb->i_write += n;\
        if(rb->i_write >= rb->size) rb->i_write -= rb->size;\
        rb->used += n;\
        return (int)n;\
    trap_items_null:\
    trap_rb_null:\
        return -1;\
    }

/* TODO: same caveat as RINGBUF_WRITE - `(int)n` can silently truncate/wrap if `type_size`
 * is wide enough and `count`/`used` exceeds INT_MAX. Unlikely, but worth revisiting. */
#define RINGBUF_READ(type_data, type_size, name)\
    RINGBUF_READ_HEADER(type_data, type_size, name)\
    {\
        TRAPVNULL(rb);\
        TRAPVNULL(items);\
        type_size n = (count < rb->used) ? count : rb->used;\
        type_size to_end = rb->size - rb->i_read;\
        type_size first = (n < to_end) ? n : to_end;\
        memcpy(items, &rb->buffer[rb->i_read], first * sizeof(type_data));\
        type_size second = n - first;\
        if(second) memcpy(items + first, rb->buffer, second * sizeof(type_data));\
        rb->i_read += n;\
        if(rb->i_read >= rb->size) rb->i_read -= rb->size;\
        rb->used -= n;\
        return (int)n;\
    trap_items_null:\
    trap_rb_null:\
        return -1;\
    }

#endif
