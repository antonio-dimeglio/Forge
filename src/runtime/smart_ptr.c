#include "runtime.h"
#include <stdlib.h>
#include <stdio.h>

void* smart_ptr_malloc(int32_t size) {
    return malloc(size);
}

void unique_ptr_release(void* smart_ptr) {
    unique_ptr* uptr = (unique_ptr*)smart_ptr;
    if (uptr->data) {
        free(uptr->data);
        uptr->data = NULL;  // Mark as released
    }
    printf("Released unique pointer\n");
}

void shared_ptr_release(void* smart_ptr) {
    shared_ptr* sptr = (shared_ptr*)smart_ptr;
    sptr->ref_count--;
    if (sptr->ref_count == 0) {
        if (sptr->data) {
            free(sptr->data);
            sptr->data = NULL;  // Mark as released
        }
        printf("Released shared pointer (ref count reached 0)\n");
    }
}

void weak_ptr_release(void* smart_ptr) {
    weak_ptr* wptr = (weak_ptr*)smart_ptr;
    wptr->is_valid = 0;  // Just mark as invalid
    printf("Released weak pointer\n");
}

void shared_ptr_retain(void* smart_ptr) {
    shared_ptr* sptr = (shared_ptr*)smart_ptr;
    sptr->ref_count++;
    printf("Retained shared pointer (ref count: %d)\n", sptr->ref_count);
}

int32_t shared_ptr_use_count(void* smart_ptr) {
    shared_ptr* sptr = (shared_ptr*)smart_ptr;
    return sptr->ref_count;
}