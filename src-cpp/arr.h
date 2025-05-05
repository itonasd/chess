#ifndef _ARR_H_
#define _ARR_H_

#ifdef ARRAY_H
#error "INCOMPATIBLE"
#endif

#include "macros.h"
#include <stdlib.h>
#include <string.h>

typedef any (*_callback_1arg)(any);
typedef any (*_callback_2arg)(any, any);

// default macros

#define true (void*)1
#define false (void*)0

#define to(type, pointer) (*(type*)(pointer))   
#define boolean(condition) ((condition) ? true : false)
#define new(type, length) arr_init(sizeof(type), length)
#define get(type, src, index) (to(type, gather(src, index)))
#define get2D(type, src, i, j) get(int, deref(src, i), j)
#define get3D(type, src, i, j, k) get(int, deref(deref(src, i), j), k)

#define push(type, dest, value) do {                \
    type a = value;                                 \
    arr_write(dest, dest->length, dest->length, &a);    \
} while (0)

#define shift(type, dest, value) do {               \
    type a = value;                                 \
    insert(dest, 0, 0, &a);                         \
} while (0)

#define pop(dest) do {                              \
    arr_erase(dest, dest->length - 1, dest->length - 1);\
} while (0)

#define unshift(dest) do {                          \
    arr_erase(dest, 0, 0);                              \
} while (0)

#define start() 0
#define end(dest) do {                              \
    dest->length - 1;                               \
} while (0)

// ranges macros

#define foreach(type, var, src)                     \
for (u64 __iterator = 0; __iterator < src->length; __iterator++) { \
    type var = to(type, src->buffer + (__iterator * src->size));

#define cycle(type, var, src)                       \
for (u64 __iterator = 0;; __iterator = (__iterator + 1) % src->length) { \
    type var = to(type, src->buffer + (__iterator * src->size));

#define endstate }


#define __TYPE_NAME__(type) _Generic((type){0}, \
        int: "int", \
        unsigned int: "unsigned int", \
        long: "long", \
        unsigned long: "unsigned long", \
        char: "char", \
        unsigned char: "unsigned char", \
        short: "short", \
        unsigned short: "unsigned short", \
        float: "float", \
        double: "double", \
        default: "unknown" \
)

#define clog_debug(type, src) do { \
    raw _byte = src->buffer; \
    type *_val = (type *)src->buffer; \
    const char *_formatting = _Generic((type){0}, \
        int: "] %d\n", \
        unsigned int: "] %u\n", \
        long: "] %ld\n", \
        unsigned long: "] %lu\n", \
        char: "] %c\n", \
        unsigned char: "] %hhu\n", \
        short: "] %hd\n", \
        unsigned short: "] %hu\n", \
        float: "] %.2f\n", \
        double: "] %.4lf\n", \
        default: "] unknown\n" \
    ); \
    \
    printf("\n    clog_debug %s %s\n", __TYPE_NAME__(type), #src); \
    printf("\n    initialized region\n"); \
    \
    u64 _iter; \
    for (_iter = 0; _iter < src->length; _iter++) { \
        printf("  %5llu: [ ", _iter); \
        \
        for (u64 _innerIter = 0; _innerIter < src->size; _innerIter++) \
            printf("0x%02x ", _byte[_iter * src->size + _innerIter]); \
        printf(_formatting, _val[_iter]); \
    } \
    \
    printf("    allocated region\n"); \
    if (src->length == src->alloc.length) printf("     none\n"); \
    for (_iter = src->length; _iter < src->alloc.length; _iter++) { \
        printf("  %5llu: [ ", _iter); \
        \
        for (u64 _innerIter = 0; _innerIter < src->size; _innerIter++) \
            printf("0x%02x ", _byte[_iter * src->size + _innerIter]); \
        printf(_formatting, _val[_iter]); \
    } \
    \
    printf("\n    statistic\n"); \
    printf("     size: %llu, length: %llu\n", src->size, src->length); \
    printf("     capacity: %llu, factor: %llu\n", src->alloc.length, src->alloc.factor); \
    printf("     struct addrs: %p\n", src); \
    printf("     buffst addrs: %p\n", src->buffer); \
    printf("     buffen addrs: %p\n", src->buffer + src->alloc.length); \
    printf("     buffer index: %d - %llu\n\n", 0, (src->size * src->alloc.length) - 1); \
} while (0)

#define clog_array(type, src) do { \
    type *_val = (type *)src->buffer; \
    const char *_formatting = _Generic((type){0}, \
        int: " %d", \
        unsigned int: " %u", \
        long: " %ld", \
        unsigned long: " %lu", \
        char: " %c", \
        unsigned char: " %hhu", \
        short: " %hd", \
        unsigned short: " %hu", \
        float: " %.2f", \
        double: " %.4lf", \
        default: " unknown" \
    ); \
    \
    printf("\n    clog_array %s %s", __TYPE_NAME__(type), #src); \
    printf("\n     ["); \
    for (u64 _iter = 0; _iter < src->length; _iter++) { \
        printf(_formatting, _val[_iter]); \
        if (_iter < src->length - 1)printf(","); \
    } \
    printf(" ]\n"); \
} while (0)

#define clog_array2D(type, src) do { \
    const char *_formatting = _Generic((type){0}, \
        int: " %d", \
        unsigned int: " %u", \
        long: " %ld", \
        unsigned long: " %lu", \
        char: " %c", \
        unsigned char: " %hhu", \
        short: " %hd", \
        unsigned short: " %hu", \
        float: " %.2f", \
        double: " %.4lf", \
        default: " unknown" \
    ); \
    \
    printf("\n    clog_array2D %s %s\n", __TYPE_NAME__(type), #src); \
    printf("     [\n"); \
    for (u64 _iter = 0; _iter < src->length; _iter++) { \
        type *_val = (type *) deref(src, _iter)->buffer; \
        printf("      ["); \
        for (u64 _innerIter = 0; _innerIter < deref(src, _iter)->length; _innerIter++) { \
            printf(_formatting, _val[_innerIter]);\
            if (_innerIter < deref(src, _iter)->length - 1) printf(","); \
        } \
        printf(" ]"); \
        if (_iter < src->length - 1)printf(","); \
        printf("\n"); \
    } \
    printf("     ]\n"); \
} while (0)

// 40ULL
typedef struct _arr {
    raw buffer;
    u64 length;
    u64 size;

    struct _mem {
        u64 length;
        u64 factor;
    } alloc;

} arr; 

// 24ULL
typedef struct _item {
    raw pointer;
    any value;
    u64 index;
} item;

typedef item* items;

// 16ULL
typedef struct _res {
    items item;
    u64 length;
} res;

typedef arr* array;
typedef res* result;

array arr_init(u64 size, u64 length);

void arr_config(array dest, u64 size, u64 length);

void arr_free(array src);

void item_free(items src);

void res_free(result src);


array arr_filtered(array src, _callback_1arg callback);
array arr_filter(array dest, _callback_1arg callback);

array mapped(array src, _callback_1arg callback);
array map(array dest, _callback_1arg callback);

array reduced(array src, _callback_2arg callback, any initial);
array reduce(array dest, _callback_2arg callback, any initial);

array merged(array src_a, array src_b);
array merge(array dest, array src);

array reversed(array src);
array reverse(array dest);

array distinct(array src);
array unique(array dest);

result findAll(array src, _callback_1arg callback);
items find(array src, _callback_1arg callback);

any gather(array src, u64 index);
array deref(array src, u64 index);


array extract(array src, u64 start, u64 end);

array arr_write(array dest, u64 start, u64 end, any src);

array insert(array dest, u64 start, u64 end, any src);

array fill(array dest, u64 start, u64 end, any val);

array arr_erase(array dest, u64 start, u64 end);

array flush(array dest, f64 percentage);






void arr_config(array dest, u64 size, u64 length) {
    if (length) {
        dest->buffer = (raw) malloc(size * length);
        memset(dest->buffer, 0xFF, size * length);
    } else dest->buffer = 0;

    dest->length = 0;
    dest->size = size;
    dest->alloc.length = length;
    dest->alloc.factor = 0;
}

array arr_init(u64 size, u64 length) {
    array dest = (array) malloc(sizeof(arr));
    arr_config(dest, size, length);
    return dest;
}

void arr_free(array src) {
    if (!src) return;

    free(src->buffer);
    free(src);
}

void item_free(items src) {
    if (!src) return;

    free(src->value);
    free(src);
}

void res_free(result src) {
    if (!src) return;

    for (u64 i = 0; i < src->length; i++)
        free(src->item[i].value);
    free(src->item);
    free(src);
}

array arr_filter(array dest, _callback_1arg callback) {
    if (!dest || !callback) return 0;

    raw extract = (raw) malloc(dest->size * dest->length);
    if (!extract) return 0;

    raw dsts = dest->buffer;
    u64 size = dest->size;
    u64 length = 0;

    for (u64 i = 0; i < dest->length; i++) {
        if ((raw) callback(dsts + (i * size))) {
            memcpy(
                extract + (length * size),
                dsts + (i * size),
                size
            );  
            length++;
        }
    }

    memcpy(
        dsts,
        extract,
        length * size
    );

    memset(
        dsts + (length * size),
        0xFF,
        (dest->length - length) * size
    );

    dest->length -= dest->length - length;
    dest->alloc.length += dest->length - length;

    free(extract);

    return dest;
}

array map(array dest, _callback_1arg callback) {
    if (!dest || !callback) return 0;

    raw dsts = dest->buffer;
    u64 size = dest->size;

    for (u64 i = 0; i < dest->length; i++)
        memcpy(
            dsts + (i * size), 
            callback(dsts + (i * size)), 
            size
        );

    return dest;
}

array reduce(array dest, _callback_2arg callback, any initial) {
    if (!dest || !callback) return 0;

    raw dsts = dest->buffer;
    u64 size = dest->size;

    u8 extract[size];
    memcpy(extract, initial, size);

    for (u64 i = 0; i < dest->length; i++)
        memcpy(
            extract, 
            callback(dsts + (i * size), extract), 
            size
        );

    memset(
        dsts + (1 * size), 
        0xFF, 
        (dest->length - 1) * size
    );

    memcpy(dsts, extract, size);
    dest->length = 1;

    return dest;
}

array merge(array dest, array src) {
    if (!dest || !src || dest->size != src->size) return 0;

    raw dsts = dest->buffer;
    raw srcs = src->buffer;
    u64 size = dest->size;

    if (dest->length + src->length > dest->alloc.length) {
        u64 new_alloc_length = dest->length + src->length + dest->alloc.factor;
        raw new_dsts = (raw) realloc(dsts, new_alloc_length * size);
        if (!new_dsts) return 0;

        dest->buffer = new_dsts;
        dsts = new_dsts;
        memset(
            dsts + (dest->alloc.length * size),
            0xFF,
            (new_alloc_length - dest->alloc.length) * size
        );
        dest->alloc.length = new_alloc_length;
    }

    memcpy(
        dsts + (dest->length * size),
        srcs,
        src->length * size
    );
    dest->length += src->length;

    return dest;
}

array reverse(array dest) {
    if (!dest) return 0;

    u64 size = dest->size;
    raw dsts = dest->buffer;

    for (u64 i = 0, j = dest->length - 1; i < j; i++, j--) {
        u8 temp[size];
        memcpy(temp, dsts + (i * size), size);
        memcpy(dsts + (i * size), dsts + (j * size), size);
        memcpy(dsts + (j * size), temp, size);
    }

    return dest;
}

array arr_write(
    array dest, 
    u64 start, 
    u64 end, 
    any src
) {
    if (!dest || !src) return 0;
    u8 swap = 0;
    if (start > end) {
        swap = 1;
        start ^= end;
        end ^= start;
        start ^= end;
    }

    u64 size = dest->size;
    u64 alloc_length = dest->alloc.length;
    u64 alloc_fac = dest->alloc.factor;
    raw dst = dest->buffer;

    if (dst == 0 || end >= alloc_length) {
        u64 new_alloc_length = end + 1 + alloc_fac;
        raw new_dst = (raw) realloc(dst, new_alloc_length * size);
        if (!new_dst) return 0;

        dest->buffer = new_dst;
        dst = new_dst;
        memset(dst + alloc_length * size, 0xFF, (new_alloc_length - alloc_length) * size);
        dest->alloc.length = new_alloc_length;
    }

    if (swap)
        for (u64 i = 0; i < (end - start + 1); i++)
            memcpy(
                dst + (start + i) * size,
                src + (end - start - i) * size,
                size
            );
    else
        memcpy(
            dst + start * size,
            src,
            (end - start + 1) * size
        );
    
    dest->length = (dest->length > end + 1) ? dest->length : end + 1;

    return dest;
}

array insert(
    array dest, 
    u64 start, 
    u64 end, 
    any src
) {
    if (!dest || !src) return 0;

    if (start >= dest->length) return arr_write(dest, start, end, src);

    u8 swap = 0;
    if (start > end) {
        swap = 1;
        start ^= end;
        end ^= start;
        start ^= end;
    }

    u64 size = dest->size;
    u64 length = dest->length;
    u64 alloc_length = dest->alloc.length;
    u64 alloc_fac = dest->alloc.factor;
    raw dst = dest->buffer;
    
    dest->length += ((end + 1) - start);
    if (dest->length > alloc_length) {
        u64 new_alloc_length = dest->length + alloc_fac;
        raw new_dst = (raw) realloc(dst, new_alloc_length * size);
        if (!new_dst) return 0;

        dest->buffer = new_dst;
        dst = new_dst;
        memset(dst + alloc_length * size, 0xFF, (new_alloc_length - alloc_length) * size);
        dest->alloc.length = new_alloc_length;
    }

    memmove(
        dst + ((end + 1) * size), 
        dst + (start * size), 
        (length - start) * size
    );

    if (swap)
        for (u64 i = 0; i < ((end + 1) - start); i++)
            memcpy(
                dst + (start + i) * size,
                src + (end - start - i) * size,
                size
            );
    else
        memcpy(
            dst + (start * size), 
            src, 
            ((end + 1) - start) * size
        );

    return dest;
}

array fill(array dest, u64 start, u64 end, any val) {
    if (!dest || !val) return 0;

    if (start > end) {
        start ^= end;
        end ^= start;
        start ^= end;
    }

    u64 size = dest->size;
    u64 alloc_length = dest->alloc.length;
    u64 alloc_fac = dest->alloc.factor;
    raw dst = dest->buffer;

    if (dst == 0 || end >= alloc_length) {
        u64 new_alloc_length = end + 1 + alloc_fac;
        raw new_dst = (raw) realloc(dst, new_alloc_length * size);
        if (!new_dst) return 0;

        dest->buffer = new_dst;
        dst = new_dst;
        memset(dst + alloc_length * size, 0xFF, (new_alloc_length - alloc_length) * size);
        dest->alloc.length = new_alloc_length;
    }

    for (u64 i = start; i < end + 1; i++)
        memcpy(
            dst + (i * size),
            val,
            size
        );

    dest->length = (dest->length > end + 1) ? dest->length : end + 1;
    return dest;
}

array arr_erase(array dest, u64 start, u64 end) {
    if (!dest || start >= dest->length || end >= dest->length) return 0;
    
    if (start > end) {
        start ^= end;
        end ^= start;
        start ^= end;
    }

    u64 size = dest->size, length = dest->length;
    raw dsts = dest->buffer;

    memmove(
        dsts + (start * size), 
        dsts + ((end + 1) * size), 
        (length - (end + 1)) * size
    );

    memset(
        dsts + ((length - ((end + 1) - start)) * size), 
        0xFF, 
        ((end + 1) - start) * size
    );

    dest->length -= ((end + 1) - start);
    return dest;
}

array flush(array dest, double percentage) {
    if (!dest) return 0;
    u64 old_length = dest->alloc.length;
    u64 size = dest->size;
    dest->alloc.length -= ((dest->alloc.length - dest->length) * (percentage / 100));

    if (dest->alloc.length != old_length) {
        raw new_buffer = (raw) realloc(dest->buffer, dest->alloc.length * size);
        if (!new_buffer) return 0;
        dest->buffer = new_buffer;

        if (dest->alloc.length > old_length)
            memset(
                dest->buffer + (old_length * size), 
                0xFF, 
                (dest->alloc.length - old_length) * size
            );

        if (dest->alloc.length < dest->length) {
            memset(
                dest->buffer + (dest->alloc.length * size),
                0xFF,
                (dest->length - dest->alloc.length) * size
            );

            dest->length -= (dest->length - dest->alloc.length);
        }
    }

    return dest;
}

array arr_filtered(array src, _callback_1arg callback) {
    if (!src || !callback) return 0;

    array dest = arr_init(src->size, 0);
    if (!dest) return 0;
    
    dest->buffer = (raw) malloc(src->size * src->length);
    if (!dest->buffer) {
        free(dest);
        return 0;
    }

    raw srcs = src->buffer;
    raw dsts = dest->buffer;
    u64 size = src->size;
    u64 length = 0;

    for (u64 i = 0; i < src->length; i++) {
        if ((raw) callback(srcs + (i * size))) {
            memcpy(dsts + (length * size), srcs + (i * size), size);
            length++;
        }
    }
    
    dest->length = length;
    dest->alloc.length = length;
    raw temp = (raw) realloc(dsts, length * size);
    if (!temp) {
        free(dsts);
        return 0;
    }

    dest->buffer = temp;

    return dest;
}

array mapped(array src, _callback_1arg callback) {
    if (!src || !callback) return 0;

    array dest = arr_init(src->size, 0);
    if (!dest) return 0;

    dest->length = src->length;
    dest->alloc.length = src->length;
    dest->buffer = (raw) malloc(src->size * src->length);
    if (!dest->buffer) {
        free(dest);
        return 0;
    }

    raw srcs = src->buffer;
    raw dsts = dest->buffer;
    u64 size = src->size;
    u64 length = src->length;

    for (u64 i = 0; i < length; i++)
        memcpy(dsts + (i * size), callback(srcs + (i * size)), size);

    return dest;
}

array reduced(array src, _callback_2arg callback, any initial) {
    if (!src || !callback) return 0;

    array dest = arr_init(src->size, 0);
    if (!dest) return 0;

    dest->length = 1;
    dest->alloc.length = 1;
    dest->buffer = (raw) malloc(src->size);
    if (!dest->buffer) {
        free(dest);
        return 0;
    }

    raw dsts = dest->buffer;
    raw srcs = src->buffer;
    u64 size = src->size;
    u64 length = src->length;
    
    memcpy(dsts, initial, size);

    for (u64 i = 0; i < length; i++)
        memcpy(dsts, callback(srcs + (i * size), dsts), size);
    
    return dest;
}

result findAll(array src, _callback_1arg callback) {
    if (!src || !callback) return 0;

    result ret = (result) malloc(sizeof(res));
    if (!ret) return 0;

    ret->length = src->length;
    ret->item = (items) malloc(sizeof(item) * src->length);
    if (!ret->item) {
        free(ret->item);
        free(ret);
        return 0;
    }

    u64 size = src->size;
    u64 retPointer = 0;
    raw srcs = src->buffer;

    for (u64 i = 0; i < src->length; i++) {
        if (callback(srcs + (i * size))) {
            ret->item[retPointer].pointer = srcs + (i * size);

            ret->item[retPointer].value = (raw) malloc(size);
            if (!ret->item[retPointer].value) {
                for (u64 i = 0; i < retPointer; i++)
                    free(ret->item[i].value);
                free(ret->item);
                free(ret);
                return 0;
            }

            memcpy(ret->item[retPointer].value, srcs + (i * size), size);

            ret->item[retPointer].index = i;
            retPointer++;
        }
    }

    if (!retPointer) {
        free(ret->item);
        ret->item = 0;
        ret->length = 0;
        return ret;
    }

    items new_item = (items) realloc(ret->item, sizeof(item) * retPointer);

    if (!new_item) {
        for (u64 i = 0; i < retPointer; i++)
            free(ret->item[i].value);
        free(ret->item);
        free(ret);
        return 0;
    }

    ret->item = new_item;
    ret->length = retPointer;

    return ret;
}

items find(array src, _callback_1arg callback) {
    if (!src || !callback) return 0;
    items ret = (items) malloc(sizeof(item));

    u64 size = src->size;
    raw srcs = src->buffer;

    for (u64 i = 0; i < src->length; i++) {
        if (callback(srcs + (i * size))) {
            ret->pointer = srcs + (i * size);

            ret->value = (raw) malloc(size);
            if (!ret->value) {
                free(ret);
                return 0;
            }
            memcpy(ret->value, srcs + (i * size), size);

            ret->index = i;
            break;
        }
    }

    return ret;
}

array merged(array src_a, array src_b) {
    if (!src_a || !src_b || src_a->size != src_b->size) return 0;

    array dest = arr_init(src_a->size, 0);
    if (!dest) return 0;

    dest->length = src_a->length + src_b->length;
    dest->alloc.length = src_a->length + src_b->length;
    dest->buffer = (raw) malloc(src_a->size * dest->length);

    if (!dest->buffer) {
        free(dest);
        return 0;
    }

    memcpy(
        dest->buffer, 
        src_a->buffer, 
        src_a->length * src_a->size
    );

    memcpy(
        dest->buffer + (src_a->length * src_a->size), 
        src_b->buffer, 
        src_b->length * src_b->size
    );

    return dest;
}

array reversed(array src) {
    if (!src) return 0;

    array dest = arr_init(src->size, 0);
    if (!dest) return 0;

    dest->length = src->length;
    dest->alloc.length = src->length;
    dest->buffer = (raw) malloc(src->size * src->length);
    if (!dest->buffer) {
        free(dest);
        return 0;
    }

    raw srcs = src->buffer;
    raw dsts = dest->buffer;
    u64 size = src->size;
    u64 length = src->length;

    for (u64 i = 0; i < length; i++)
        memcpy(dsts + (i * size), srcs + ((length - 1 - i) * size), size);

    return dest;
}

array extract(array src, u64 start, u64 end) {
    if (!src || start >= src->length || end >= src->length) return 0;
    u8 swap = 0;
    if (start > end) {
        swap = 1;
        start ^= end;
        end ^= start;
        start ^= end;
    }

    array dest = arr_init(src->size, 0);
    if (!dest) return 0;

    dest->length = end - start + 1;
    dest->alloc.length = end - start + 1;
    dest->buffer = (raw) malloc(src->size * src->length);
    if (!dest->buffer) {
        free(dest);
        return 0;
    }

    memcpy(
        dest->buffer,
        src->buffer + (start * src->size),
        (end - start + 1) * src->size
    );

    return dest;
}

any gather(array src, u64 index) {
    if (!src || index >= src->length) return 0;
    return src->buffer + (index * src->size);
}

array deref(array src, u64 index) {
    return (array)((u64*)src->buffer)[index];
}
#endif
