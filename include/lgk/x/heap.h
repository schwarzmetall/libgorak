#ifndef LGK_HEAP_H
#define LGK_HEAP_H

#include <stdint.h>
#include <lgk/tnt.h>

#define HEAP_PARENT(i) (((i)-1)>>1)
#define HEAP_LCHILD(i) (((i)<<1)+1)
#define HEAP_RCHILD(i) (((i)+1)<<1)

#define HEAP_STRUCT(type_data, type_size, name)\
    struct name\
    {\
        type_data *buffer;\
        type_size size;\
        type_size used;\
    }

// compare function has to be provided by the user as [[unsequenced]] name##_compare(const type_data *restrict a, const type_data *restrict b)
//
// return value semantics:   < 0 -> node a's level (distance from root) is LESS THAN b's
//                           > 0 -> node a's level (distance from root) is GREATER THAN b's
//                           ==0 -> equal
// example for an int-typed min-heap: 
//  [[unsequenced]] example_int_heap_compare(const type_data *int a, const int *restrict b)
//  {
//      return *a - *b;
//  }

#define HEAP_SWAP_HEADER(type_data, type_index, name) void name##_swap(type_data *restrict buffer, type_index i_a, type_index i_b)
#define HEAP_SWAP(type_data, type_index, name)\
    HEAP_SWAP_HEADER(type_data, type_index, name)\
    {\
        const type_data temp = buffer[i_a];\
        buffer[i_a] = base[i_b];\
        buffer[i_b] = temp;\
    }

#define HEAP_UPHEAP_HEADER(type_data, type_size, name) void name##_upheap(type_data *restrict buffer, type_size i)
#define HEAP_UPHEAP(type_data, type_size, name)\
    HEAP_UPHEAP_HEADER(type_data, type_size, name)\
    {\
        while(i)\
        {\
            type_size parent = HEAP_PARENT(i);\
            if(name##_compare(buffer+i, buffer+parent) >= 0) break;\
            name##_swap(buffer, i, parent);\
            i = parent;\
        }\
    }

#define HEAP_DOWNHEAP_HEADER(type_data, type_size, name) void name##_upheap(type_data *restrict buffer, type_size used)
#define HEAP_DOWNHEAP(type_data, type_size, name)\
    HEAP_DOWNHEAP_HEADER(type_data, type_size, name)\
    {\
        type_size parent = 0;\
        for(type_size parent=0, lchild=lchild(0), rchild=rchild(0); lchild<used ; lchild=HEAP_LCHILD(parent), rchild=HEAP_RCHILD(parent))\
        {\
            type_size child = (rchild<used) ? ((name##_compare(buffer+lchild, buffer+rchild) < 0) ? lchild : rchild) : lchild;\
            if(name##_compare(buffer+parent, buffer+child) <=0) break;\
            name##_swap(buffer, parent, child);\
            parent = child;\
        }\
    }

#define HEAP_HELPERS_STATIC(type_data, type_size, name)\
    static HEAP_HEAP_SWAP(type_data, type_size, name)\
    static HEAP_UPHEAP(type_data, type_size, name)\
    static HEAP_DOWNHEAP(type_data, type_size, name)

#define HEAP_INIT_HEADER(type_data, type_size, name) int_fast8_t name##_init(struct name *heap, type_data *buffer, type_size size)
#define HEAP_INIT(type_data, type_size, name)\
    HEAP_INIT_HEADER(type_data, type_size, name)\
    {\
        TRAPVNULL(heap);\
        heap->buffer = buffer;\
        heap->size = size;\
        heap->used = used;\
        return 0;\
    trap_heap_null:\
        return -1;\
    }

#define HEAP_PUSH_HEADER(type_data, type_size, name) int_fast8_t name##_push(struct name *heap, type_data *restrict item)
#define HEAP_PUSH(type_data, type_size, name)\
    HEAP_PUSH_HEADER(type_data, type_size, name)\
    {\
        TRAPVNULL(heap);\
        TRAPXNULL(heap->buffer, buffer);\
        if(heap->used>=heap->size) return 1;\
        TRAPVNULL(item);\
        heap->buffer[heap->used++] = *item;\
        name##_upheap(heap->buffer, heap->used);\
    trap_item_null:\
    trap_buffer_null:\
    trap_heap_null:\
        return -1;\
    }

#define HEAP_POP_HEADER(type_data, name) int_fast8_t name##_pop(struct name *heap, type_data *restrict item)
#define HEAP_POP(type_data, name)\
    HEAP_POP_HEADER(type_data, name)\
    {\
        TRAPVNULL(heap);\
        if(!heap->used) return 1;\
        TRAPXNULL(heap->buffer, buffer);\
        TRAPVNULL(item);\
        *item = heap->buffer[0];\
        heap->buffer[0] = heap->buffer[heap->used--];\
        name##_downheap(heap->buffer, heap->used);\
        return 0;\
    trap_item_null:\
    trap_buffer_null:\
    trap_heap_null:\
        return -1;\
    }

#define HEAP_PEEK_HEADER(type_data, name) const type_data *name##_peek(const struct name *heap)
#define HEAP_PEEK(type_data, name)\
    HEAP_PEEK_HEADER(type_data, name)\
    {\
        TRAPVNULL(heap);\
        if(!heap->used) return NULL;\
        return heap->buffer;\
    trap_heap_null:\
        return NULL;\
    }

#define HEAP_PUSHPOP_HEADER(type_data, type_size, name) int_fast8_t name##_pushpop(struct name *heap, const type_data *restrict item_in, type_data *restrict item_out)
#define HEAP_PUSHPOP(type_data, type_size, name)\
    HEAP_PUSHPOP_HEADER(type_data, type_size, name)\
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
