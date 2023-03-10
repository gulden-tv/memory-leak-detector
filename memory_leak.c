#include <stdio.h>
#include <stdlib.h>
#include "memory_leak.h"

static list *memlist = NULL;
static size_t maxMemoryAllocated = 0;

#undef malloc 
#undef free
#undef realloc

static void insert_block(void *address, size_t size, char *comment) {
    list *tmp = malloc(sizeof(list));
    tmp->next = memlist;
    tmp->address = address;
    tmp->size = size;
    sprintf(tmp->comment,"%s",comment);
    memlist = tmp;
    size_t total = totalMemoryAllocate(memlist);
    if (maxMemoryAllocated < total) {
        maxMemoryAllocated = total;
    }
}
static _Bool delete_block(void *address) {
    if(memlist == NULL)
        return 0;
    list *del = NULL;
    if( memlist->address == address) {
        del = memlist;
        memlist = memlist->next;
        free(del);
        return  1;
    }
    list *tmp = memlist;
    while( tmp->next ) {
        if( tmp->next->address == address ) {
            del = tmp->next;
            tmp->next = del->next;
            free(del);
            return 1;
        }
        tmp=tmp->next;
    }
    return 0;
}
static _Bool update_block(void *oldptr, void *newptr, size_t newsize, char *comment) {
    list *tmp = memlist;
    while(tmp) {
        if(tmp->address == oldptr) {
            tmp->address = newptr;
            sprintf(tmp->comment,"%s", comment);
            tmp->size = newsize;
            return 1;
        }
        tmp = tmp->next;
    }
    return 0;
}
void printMemoryLeak() {
    list *head = memlist;
    if(head == NULL) {
        printf("No memory leak detect\n");
    }
    while(head) {
        printf("%s\n",head->comment);
        head = head->next;
    }
    printf("Max memory allocate is %zu bytes\n", maxMemoryAllocated);
}
 
void* my_malloc(size_t size, const char *file, int line, const char *func)
 {
    void *ptr = malloc(size);
    char coment[64] = {0};
    sprintf (coment,"Allocated = %s, %i, %s, %p[%li]", file, line, func, ptr, size);
    insert_block(ptr,size,coment);
    
    return ptr;
}
void my_free(void *ptr, const char *file, int line, const char *func)
 {
    if(!delete_block(ptr))
        free(ptr);
}

static size_t totalMemoryAllocate(list *head) {
    size_t max = 0;
    while(head) {
        max += head->size;
        head = head->next;
    }
    return max;
}

void* my_realloc(void *ptr, size_t newsize, const char *file, int line, const char *func) {
    void *oldptr = ptr;
    ptr = realloc(ptr, newsize);
    char coment[64] = {0};
    sprintf (coment,"Reallocated = %s, %i, %s, %p[%li]", file, line, func, ptr, newsize);
    update_block(oldptr,ptr,newsize,coment);
    
    return ptr;
}
