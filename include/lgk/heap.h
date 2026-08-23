#ifndef LGK_HEAP_H
#define LGK_HEAP_H

#include <stdint.h>
#include <lgk/util.h>
#include <lgk/tnt.h>

#define HEAP_PARENT(i) (((i)-1)>>1)
#define HEAP_LCHILD(i) (((i)<<1)+1)
#define HEAP_RCHILD(i) (((i)+1)<<1)

#define HEAP_STRUCT(name, type_data, type_size)\
    struct name\
    {\
        type_data *buffer;\
        type_size size;\
        type_size used;\
    }

// compare function has to be provided by the user as <signed int type> name##_compare(const type_data *restrict a, const type_data *restrict b)
//
// return value semantics:   < 0 -> node a's level (distance from root) is LESS THAN b's
//                           > 0 -> node a's level (distance from root) is GREATER THAN b's
//                           ==0 -> equal
// example for an int-typed min-heap:
//  static int_fast8_t example_int_heap_compare(const int *restrict a, const int *restrict b) [[unsequenced]]
//  {
//      return (*a >= *b) - (*a <= *b);
//  }

#define HEAP_SWAP_HEADER(name) void name##_swap(STYPEOF_DEREF(name, buffer) *restrict buffer, STYPEOF(name, size) i_a, STYPEOF(name, size) i_b)
#define HEAP_SWAP(name)\
    HEAP_SWAP_HEADER(name)\
    {\
        const typeof(*buffer) temp = buffer[i_a];\
        buffer[i_a] = buffer[i_b];\
        buffer[i_b] = temp;\
    }

#define HEAP_UPHEAP_HEADER(name) void name##_upheap(STYPEOF_DEREF(name, buffer) *restrict buffer, STYPEOF(name, size) i)
#define HEAP_UPHEAP(name)\
    HEAP_UPHEAP_HEADER(name)\
    {\
        while(i)\
        {\
            STYPEOF(name, size) parent = HEAP_PARENT(i);\
            if(name##_compare(buffer+i, buffer+parent) >= 0) break;\
            name##_swap(buffer, i, parent);\
            i = parent;\
        }\
    }

#define HEAP_DOWNHEAP_HEADER(name) void name##_downheap(STYPEOF_DEREF(name, buffer) *restrict buffer, STYPEOF(name, size) used)
#define HEAP_DOWNHEAP(name)\
    HEAP_DOWNHEAP_HEADER(name)\
    {\
        for(STYPEOF(name, size) parent=0, lchild=HEAP_LCHILD(0), rchild=HEAP_RCHILD(0); lchild<used ; lchild=HEAP_LCHILD(parent), rchild=HEAP_RCHILD(parent))\
        {\
            STYPEOF(name, size) child = (rchild<used) ? ((name##_compare(buffer+lchild, buffer+rchild) < 0) ? lchild : rchild) : lchild;\
            if(name##_compare(buffer+parent, buffer+child) <= 0) break;\
            name##_swap(buffer, parent, child);\
            parent = child;\
        }\
    }

#define HEAP_HELPERS_STATIC(name)\
    static HEAP_SWAP(name)\
    static HEAP_UPHEAP(name)\
    static HEAP_DOWNHEAP(name)

#define HEAP_INIT_HEADER(name) int_fast8_t name##_init(struct name *heap, typeof(heap->buffer) buffer, typeof(heap->size) size)
#define HEAP_INIT(name)\
    HEAP_INIT_HEADER(name)\
    {\
        TRAPVNULL(heap);\
        heap->buffer = buffer;\
        heap->size = size;\
        heap->used = 0;\
        return 0;\
    trap_heap_null:\
        return -1;\
    }

#define HEAP_PUSH_HEADER(name) int_fast8_t name##_push(struct name *heap, const typeof(*heap->buffer) *restrict item)
#define HEAP_PUSH(name)\
    HEAP_PUSH_HEADER(name)\
    {\
        TRAPVNULL(heap);\
        TRAPXNULL(heap->buffer, buffer);\
        if(heap->used >= heap->size) return 1;\
        TRAPVNULL(item);\
        heap->buffer[heap->used] = *item;\
        name##_upheap(heap->buffer, heap->used++);\
        return 0;\
    trap_item_null:\
    trap_buffer_null:\
    trap_heap_null:\
        return -1;\
    }

#define HEAP_POP_HEADER(name) int_fast8_t name##_pop(struct name *heap, typeof(*heap->buffer) *restrict item)
#define HEAP_POP(name)\
    HEAP_POP_HEADER(name)\
    {\
        TRAPVNULL(heap);\
        if(!heap->used) return 1;\
        TRAPXNULL(heap->buffer, buffer);\
        TRAPVNULL(item);\
        *item = heap->buffer[0];\
        heap->buffer[0] = heap->buffer[--heap->used];\
        name##_downheap(heap->buffer, heap->used);\
        return 0;\
    trap_item_null:\
    trap_buffer_null:\
    trap_heap_null:\
        return -1;\
    }

#define HEAP_PEEK_HEADER(name) const STYPEOF_DEREF(name, buffer) *name##_peek(const struct name *heap)
#define HEAP_PEEK(name)\
    HEAP_PEEK_HEADER(name)\
    {\
        TRAPVNULL(heap);\
        if(!heap->used) return NULL;\
        return heap->buffer;\
    trap_heap_null:\
        return NULL;\
    }

#define HEAP_PUSHPOP_HEADER(name) int_fast8_t name##_pushpop(struct name *heap, const typeof(*heap->buffer) *restrict item_in, typeof(*heap->buffer) *restrict item_out)
#define HEAP_PUSHPOP(name)\
    HEAP_PUSHPOP_HEADER(name)\
    {\
        TRAPVNULL(heap);\
        TRAPVNULL(item_in);\
        TRAPVNULL(item_out);\
        if(heap->used)\
        {\
            TRAPXNULL(heap->buffer, buffer);\
            if(name##_compare(item_in, heap->buffer) > 0)\
            {\
                *item_out = heap->buffer[0];\
                *heap->buffer[0] = *item_in;\
                name##_downheap(heap->buffer, heap->used);\
                return 0;\
            }\
        }\
        *item_out = *item_in;\
        return 0;\
    trap_buffer_null:\
    trap_item_out_null:\
    trap_item_in_null:\
    trap_heap_null:\
        return -1;\
    }

#endif
