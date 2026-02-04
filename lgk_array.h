#ifndef LGK_ARRAY_H
#define LGK_ARRAY_H

#define ARRAY_FIND_FUNCTION_HEADER(data_type, index_type)\
    index_type arr_find_##data_type(data_type item, const data_type *array, index_type n)

#define ARRAY_FILL_FUNCTION_HEADER(data_type, index_type)\
    void arr_fill_##data_type(data_type value, data_type *array, index_type n)

#define ARRAY_FIND_FUNCTION(data_type, index_type)\
    ARRAY_FIND_FUNCTION_HEADER(data_type, index_type)\
    {\
        for(index_type i=0; i<n; i++) if(array[i] == item) return i;\
        return -1;\
    }

#define ARRAY_FILL_FUNCTION(data_type, index_type)\
    ARRAY_FILL_FUNCTION_HEADER(data_type, index_type)\
    {\
        for(index_type i=0; i<n; i++) array[i] = value;\
    }

ARRAY_FIND_FUNCTION_HEADER(int, int);
ARRAY_FILL_FUNCTION_HEADER(int, int);

#endif
