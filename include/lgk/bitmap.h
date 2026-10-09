#ifndef LGK_BITMAP_H
#define LGK_BITMAP_H

#include <stdint.h>
#include <string.h>
#include <lgk/util.h>
#include <lgk/tnt.h>

#define BITMAP_STRUCT(name, word_type)\
    struct name\
    {\
        static_assert(IS_UNSIGNED(word_type));\
        word_type *map;\
        unsigned size;\
    }

// usable for plain and atomic bitmaps
#define BITMAP_WORD_TYPE(name) STYPEOF_DEREF(name, map)
#define BITMAP_WORD_BITWIDTH(name) BITWIDTH(BITMAP_WORD_TYPE(name))
#define BITMAP_NWORDS(name, nbits) INTDIVCEIL((nbits), BITMAP_WORD_BITWIDTH(name))
#define BITMAP_NBYTES(name, nbits) (BITMAP_NWORDS(name, nbits) * sizeof(BITMAP_WORD_TYPE(name)))
#define BITMAP_TAIL_MASK(name, nbits) ((typeof_unqual(BITMAP_WORD_TYPE(name)))~(((typeof_unqual(BITMAP_WORD_TYPE(name)))~((typeof_unqual(BITMAP_WORD_TYPE(name)))0)) << ((nbits) % BITWIDTH(BITMAP_WORD_TYPE(name)))))


// returns -1 on error
#define BITMAP_INIT_HEADER(name) int_fast8_t name##_init(struct name *bitmap, typeof(bitmap->map) map, unsigned size, int_fast8_t value)
#define BITMAP_INIT(name)\
    BITMAP_INIT_HEADER(name)\
    {\
        TRAPVNULL(bitmap);\
        if(size) TRAPVNULL(map);\
        bitmap->map = map;\
        bitmap->size = size;\
        if(!size) return 0;\
        if(value >= 0) memset(map, value ? ~0 : 0, BITMAP_NBYTES(name, size));\
        if(size % BITMAP_WORD_BITWIDTH(name)) map[BITMAP_NWORDS(name, size)-1] &= BITMAP_TAIL_MASK(name, size);\
        return 0;\
    trap_map_null:\
    trap_bitmap_null:\
        return -1;\
    }

// returns -1 on error
#define BITMAP_GET_HEADER(name) int_fast8_t name##_get(const struct name *bitmap, unsigned i)
#define BITMAP_GET(name)\
    BITMAP_GET_HEADER(name)\
    {\
        TRAPVNULL(bitmap);\
        TRAPXNULL(bitmap->map, map);\
        TRAPVXGTE(i, bitmap->size, size, "u");\
        const uint_fast8_t bitwidth = BITMAP_WORD_BITWIDTH(name);\
        const BITMAP_WORD_TYPE(name) mask = (BITMAP_WORD_TYPE(name))1 << (i % bitwidth);\
        return ((bitmap->map[i / bitwidth] & mask) != 0);\
    trap_i_gte_size:\
    trap_map_null:\
    trap_bitmap_null:\
        return -1;\
    }

// returns -1 on error
#define BITMAP_SET_HEADER(name) int_fast8_t name##_set(struct name *bitmap, unsigned i, int_fast8_t setval)
#define BITMAP_SET(name)\
    BITMAP_SET_HEADER(name)\
    {\
        TRAPVNULL(bitmap);\
        TRAPXNULL(bitmap->map, map);\
        TRAPVXGTE(i, bitmap->size, size, "u");\
        const uint_fast8_t bitwidth = BITMAP_WORD_BITWIDTH(name);\
        const BITMAP_WORD_TYPE(name) mask = (BITMAP_WORD_TYPE(name))1 << (i % bitwidth);\
        if(setval) bitmap->map[i / bitwidth] |= mask;\
        else bitmap->map[i / bitwidth] &= ~mask;\
        return (setval != 0);\
    trap_i_gte_size:\
    trap_map_null:\
    trap_bitmap_null:\
        return -1;\
    }

#endif
