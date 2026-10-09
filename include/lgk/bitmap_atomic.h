#ifndef LGK_BITMAP_ATOMIC_H
#define LGK_BITMAP_ATOMIC_H

#include <stdint.h>
#include <stdatomic.h>
#include <lgk/util.h>
#include <lgk/tnt.h>
#include <lgk/bitmap.h>

#define BITMAP_ATOMIC_STRUCT(name, word_type)\
    struct name\
    {\
        static_assert(IS_UNSIGNED(word_type));\
        static_assert(IS_ATOMIC_TYPE(word_type));\
        word_type *map;\
        unsigned size;\
    }

#define BITMAP_ATOMIC_INIT_HEADER(name) int_fast8_t name##_init(struct name *bitmap, typeof(bitmap->map) map, unsigned size, int_fast8_t value)
#define BITMAP_ATOMIC_INIT(name)\
    BITMAP_ATOMIC_INIT_HEADER(name)\
    {\
        TRAPVNULL(bitmap);\
        if(size) TRAPVNULL(map);\
        bitmap->map = map;\
        bitmap->size = size;\
        if(!size) return 0;\
        const typeof_unqual(BITMAP_WORD_TYPE(name)) zero = 0;\
        if(value >= 0) for(unsigned i_word = 0; i_word < BITMAP_NWORDS(name, size); i_word++) atomic_init(map+i_word, value ? ~zero : zero);\
        if(size % BITMAP_WORD_BITWIDTH(name)) atomic_fetch_and(map+BITMAP_NWORDS(name, size)-1, BITMAP_TAIL_MASK(name, size));\
        return 0;\
    trap_map_null:\
    trap_bitmap_null:\
        return -1;\
    }

#define BITMAP_ATOMIC_FILL_HEADER(name) int_fast8_t name##_fill(struct name *bitmap, typeof(bitmap->map) map, unsigned size, uint_fast8_t value)
#define BITMAP_ATOMIC_FILL(name)\
    BITMAP_ATOMIC_FILL_HEADER(name)\
    {\
        TRAPVNULL(bitmap);\
        if(size) TRAPVNULL(map);\
        bitmap->map = map;\
        bitmap->size = size;\
        if(!size) return 0;\
        const typeof_unqual(BITMAP_WORD_TYPE(name)) zero = 0;\
        for(unsigned i_word = 0; i_word < BITMAP_NWORDS(name, size); i_word++) atomic_store(map+i_word, value ? ~zero : zero);\
        if(size % BITMAP_WORD_BITWIDTH(name)) atomic_fetch_and(map+BITMAP_NWORDS(name, size)-1, BITMAP_TAIL_MASK(name, size));\
        return 0;\
    trap_map_null:\
    trap_bitmap_null:\
        return -1;\
    }

#define BITMAP_ATOMIC_FILL_EXPLICIT_HEADER(name) int_fast8_t name##_fill_explicit(struct name *bitmap, typeof(bitmap->map) map, unsigned size, uint_fast8_t value, memory_order order)
#define BITMAP_ATOMIC_FILL_EXPLICIT(name)\
    BITMAP_ATOMIC_FILL_EXPLICIT_HEADER(name)\
    {\
        TRAPVNULL(bitmap);\
        if(size) TRAPVNULL(map);\
        bitmap->map = map;\
        bitmap->size = size;\
        if(!size) return 0;\
        const typeof_unqual(BITMAP_WORD_TYPE(name)) zero = 0;\
        for(unsigned i_word = 0; i_word < BITMAP_NWORDS(name, size); i_word++) atomic_store_explicit(map+i_word, value ? ~zero : zero, order);\
        if(size % BITWIDTH(BITMAP_WORD_TYPE(name))) atomic_fetch_and_explicit(map+BITMAP_N_WORDS(name, size)-1, BITMAP_TAIL_MASK(name, size), order);\
        return 0;\
    trap_map_null:\
    trap_bitmap_null:\
        return -1;\
    }

#define BITMAP_ATOMIC_GET_HEADER(name) int_fast8_t name##_get(const struct name *bitmap, unsigned i)
#define BITMAP_ATOMIC_GET(name)\
    BITMAP_ATOMIC_GET_HEADER(name)\
    {\
        TRAPVNULL(bitmap);\
        TRAPXNULL(bitmap->map, map);\
        TRAPVXGTE(i, bitmap->size, size, "u");\
        const unsigned bitwidth = BITMAP_WORD_BITWIDTH(name);\
        const typeof_unqual(BITMAP_WORD_TYPE(name)) mask = (typeof_unqual(BITMAP_WORD_TYPE(name)))1 << (i % bitwidth);\
        return ((atomic_load(bitmap->map + (i / bitwidth)) & mask) != 0);\
    trap_i_gte_size:\
    trap_map_null:\
    trap_bitmap_null:\
        return -1;\
    }

// Returns the bit at i (0 or 1), or -1 on invalid parameters
#define BITMAP_ATOMIC_GET_EXPLICIT_HEADER(name) int_fast8_t name##_get_explicit(const struct name *bitmap, unsigned i, memory_order order)
#define BITMAP_ATOMIC_GET_EXPLICIT(name)\
    BITMAP_ATOMIC_GET_EXPLICIT_HEADER(name)\
    {\
        TRAPVNULL(bitmap);\
        TRAPXNULL(bitmap->map, map);\
        TRAPVXGTE(i, bitmap->size, size, "u");\
        TRAPVXEQ(order, memory_order_release, release, "i");\
        TRAPVXEQ(order, memory_order_acq_rel, acq_rel, "i");\
        const unsigned bitwidth = BITMAP_WORD_BITWIDTH(name);\
        const typeof_unqual(BITMAP_WORD_TYPE(name)) mask = (typeof_unqual(BITMAP_WORD_TYPE(name)))1 << (i % bitwidth);\
        return ((atomic_load_explicit(bitmap->map + (i / bitwidth), order) & mask) != 0);\
    trap_order_eq_acq_rel:\
    trap_order_eq_release:\
    trap_i_gte_size:\
    trap_map_null:\
    trap_bitmap_null:\
        return -1;\
    }

#define BITMAP_ATOMIC_SET_HEADER(name) int_fast8_t name##_set(struct name *bitmap, unsigned i, int_fast8_t setval)
#define BITMAP_ATOMIC_SET(name)\
    BITMAP_ATOMIC_SET_HEADER(name)\
    {\
        TRAPVNULL(bitmap);\
        TRAPXNULL(bitmap->map, map);\
        TRAPVXGTE(i, bitmap->size, size, "u");\
        const unsigned bitwidth = BITMAP_WORD_BITWIDTH(name);\
        const typeof_unqual(BITMAP_WORD_TYPE(name)) mask = (typeof_unqual(BITMAP_WORD_TYPE(name)))1 << (i % bitwidth);\
        if(setval) atomic_fetch_or(bitmap->map + (i / bitwidth), mask);\
        else atomic_fetch_and(bitmap->map + (i / bitwidth), ~mask);\
        return (setval != 0);\
    trap_i_gte_size:\
    trap_map_null:\
    trap_bitmap_null:\
        return -1;\
    }

#define BITMAP_ATOMIC_SET_EXPLICIT_HEADER(name) int_fast8_t name##_set_explicit(struct name *bitmap, unsigned i, int_fast8_t setval, memory_order order)
#define BITMAP_ATOMIC_SET_EXPLICIT(name)\
    BITMAP_ATOMIC_SET_EXPLICIT_HEADER(name)\
    {\
        TRAPVNULL(bitmap);\
        TRAPXNULL(bitmap->map, map);\
        TRAPVXGTE(i, bitmap->size, size, "u");\
        const unsigned bitwidth = BITMAP_WORD_BITWIDTH(name);\
        const typeof_unqual(BITMAP_WORD_TYPE(name)) mask = (typeof_unqual(BITMAP_WORD_TYPE(name)))1 << (i % bitwidth);\
        if(setval) atomic_fetch_or_explicit(bitmap->map + (i / bitwidth), mask, order);\
        else atomic_fetch_and_explicit(bitmap->map + (i / bitwidth), ~mask, order);\
        return (setval != 0);\
    trap_i_gte_size:\
    trap_map_null:\
    trap_bitmap_null:\
        return -1;\
    }

#endif
