#ifndef FORGE_RUNTIME_H
#define FORGE_RUNTIME_H

#include <stdint.h>
typedef struct {
    void* data;
    void* metadata;
} unique_ptr;

typedef struct {
    int32_t ref_count;
    void* data;
    void* metadata;
} shared_ptr; 

typedef struct {
    void* shared_ptr_ref;
    void* metadata;
    char is_valid;
} weak_ptr;

void* smart_ptr_malloc(int32_t size);
void unique_ptr_release(void* unique_ptr);
void shared_ptr_release(void* shared_ptr);
void weak_ptr_release(void* weak_ptr);

void shared_ptr_retain(void* shared_ptr);  // Only shared ptrs need retain
int32_t shared_ptr_use_count(void* shared_ptr);

#endif