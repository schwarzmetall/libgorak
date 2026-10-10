#ifndef LGK_FIFO_H
#define LGK_FIFO_H

#include <string.h>
#include <stdint.h>
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

#define FIFO_INIT_HEADER(name) int_fast8_t name##_init(struct name *fifo, typeof(fifo->buffer) buffer, typeof(fifo->size) size)
#define FIFO_INIT(name)\
    FIFO_INIT_HEADER(name)\
    {\
        TRAPVNULL(fifo);\
        fifo->buffer = buffer;\
        fifo->size = size;\
        fifo->used = fifo->i_read = fifo->i_write = 0;\
        return 0;\
    trap_fifo_null:\
        return -1;\
    }

#define FIFO_INIT_PREFILLED_HEADER(name) int_fast8_t name##_init_prefilled(struct name *fifo, typeof(fifo->buffer) buffer, typeof(fifo->size) size, typeof(fifo->used) used)
#define FIFO_INIT_PREFILLED(name)\
    FIFO_INIT_PREFILLED_HEADER(name)\
    {\
        TRAPVNULL(fifo);\
        TRAP(used>size, used, "used > size");\
        int status = name##_init(fifo, buffer, size);\
        TRAPF(status, name##_init, status, "i");\
        fifo->used = used;\
        if(used<size) fifo->i_write = used;\
        return status;\
    trap_##name##_init:\
        return status;\
    trap_used:\
    trap_fifo_null:\
        return -1;\
    }

#define FIFO_PUSH_HEADER(name) int_fast8_t name##_push(struct name *fifo, const typeof(*fifo->buffer) *restrict item)
#define FIFO_PUSH(name)\
    FIFO_PUSH_HEADER(name)\
    {\
        TRAPVNULL(fifo);\
        if(fifo->used == fifo->size) return 1;\
        TRAPXXGTE(fifo->i_write, fifo->size, i_write, size, "u");\
        TRAPXNULL(fifo->buffer, buffer);\
        fifo->buffer[fifo->i_write++] = *item;\
        if(fifo->i_write == fifo->size) fifo->i_write = 0;\
        fifo->used++;\
        return 0;\
    trap_buffer_null:\
    trap_i_write_gte_size:\
    trap_fifo_null:\
        return -1;\
    }

#define FIFO_POP_HEADER(name) int_fast8_t name##_pop(struct name *fifo, typeof(*fifo->buffer) *item)
#define FIFO_POP(name)\
    FIFO_POP_HEADER(name)\
    {\
        TRAPVNULL(fifo);\
        TRAPVNULL(item);\
        if(!fifo->used) return 1;\
        TRAPXXGTE(fifo->i_read, fifo->size, i_read, size, "u");\
        TRAPXNULL(fifo->buffer, buffer);\
        *item = fifo->buffer[fifo->i_read++];\
        if(fifo->i_read == fifo->size) fifo->i_read = 0;\
        fifo->used--;\
        return 0;\
    trap_buffer_null:\
    trap_i_read_gte_size:\
    trap_item_null:\
    trap_fifo_null:\
        return -1;\
    }

/* TODO: return value is `(int)n` where `n` is a `type_size` count of items transferred;
 * if `type_size` is wider than `int` (or just large-valued) and `count`/`available` exceeds
 * INT_MAX, this cast silently truncates/wraps. Unlikely in practice (queue/ring buffer sizes
 * are not expected to approach INT_MAX items), but worth revisiting if `type_size` is ever
 * instantiated with a wide type or very large capacities. */
#define FIFO_WRITE_HEADER(name) int_fast8_t name##_write(struct name *fifo, const typeof(*fifo->buffer) *items, typeof(fifo->size) count)
#define FIFO_WRITE(name)\
    FIFO_WRITE_HEADER(name)\
    {\
        if(!count) return 0;\
        TRAPVNULL(fifo);\
        TRAPVNULL(items);\
        TRAPXNULL(fifo->buffer, buffer);\
        typeof(fifo->size) available = fifo->size - fifo->used;\
        typeof(fifo->size) n = (count < available) ? count : available;\
        typeof(fifo->size) to_end = fifo->size - fifo->i_write;\
        typeof(fifo->size) first = (n < to_end) ? n : to_end;\
        memcpy(&fifo->buffer[fifo->i_write], items, first * sizeof(*fifo->buffer));\
        typeof(fifo->size) second = n - first;\
        if(second) memcpy(fifo->buffer, items + first, second * sizeof(*fifo->buffer));\
        fifo->i_write += n;\
        if(fifo->i_write >= fifo->size) fifo->i_write -= fifo->size;\
        fifo->used += n;\
        return (int)n;\
    trap_buffer_null:\
    trap_items_null:\
    trap_fifo_null:\
        return -1;\
    }

/* TODO: same caveat as FIFO_WRITE - `(int)n` can silently truncate/wrap if `type_size`
 * is wide enough and `count`/`used` exceeds INT_MAX. Unlikely, but worth revisiting. */
#define FIFO_READ_HEADER(name) int_fast8_t name##_read(struct name *fifo, typeof(*fifo->buffer) *items, typeof(fifo->size) count)
#define FIFO_READ(name)\
    FIFO_READ_HEADER(name)\
    {\
        if(!count) return 0;\
        TRAPVNULL(fifo);\
        TRAPVNULL(items);\
        TRAPXNULL(fifo->buffer, buffer);\
        typeof(fifo->size) n = (count < fifo->used) ? count : fifo->used;\
        typeof(fifo->size) to_end = fifo->size - fifo->i_read;\
        typeof(fifo->size) first = (n < to_end) ? n : to_end;\
        memcpy(items, &fifo->buffer[fifo->i_read], first * sizeof(*fifo->buffer));\
        typeof(fifo->size) second = n - first;\
        if(second) memcpy(items + first, fifo->buffer, second * sizeof(*fifo->buffer));\
        fifo->i_read += n;\
        if(fifo->i_read >= fifo->size) fifo->i_read -= fifo->size;\
        fifo->used -= n;\
        return (int)n;\
    trap_buffer_null:\
    trap_items_null:\
    trap_fifo_null:\
        return -1;\
    }

#endif
