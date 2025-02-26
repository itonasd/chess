#ifndef HASHMAP
#define HASHMAP

typedef struct {
    int key;
    int val;
} hashmap_t;

int strcmp_s (const char *a, const char *b) {
    while (*a && (*a == *b)) {
        a++; b++;
    }
    return (unsigned char) *a - (unsigned char) *b;
}

int hashmap_retrieve(hashmap_t pairs[], int size, int key) {
    for (int i = 0; i < size; i++) {
        if (pairs[i].key == key) return pairs[i].val;
    }
    return -1;
}

int hashmap_pairs(hashmap_t a[], hashmap_t b[], int size, int key) {
    for (int i = 0; i < size; i++) {
        if (a[i].val == 0) continue;
        if (a[i].key == key) return b[i].val;
    }
    return 0;
}

#endif
