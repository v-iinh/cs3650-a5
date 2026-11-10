#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#include <malloc.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

// Include any other headers we need here

// NOTE: You should NOT include <stdlib.h> in your final implementation.
// malloc.h turns every call to malloc/calloc/free in this file into a call
// to mymalloc/mycalloc/myfree, so calling them here would recurse forever.

#include <debug.h> // definition of debug_printf

typedef struct block {
  size_t size;
  struct block *next;
  int free;
} block_t;

static block_t *list_head = NULL;
static block_t *list_tail = NULL;

static block_t *find_first_fit(size_t size) {
  for (block_t *b = list_head; b != NULL; b = b->next) {
    if (b->free && b->size >= size) {
      return b;
    }
  }
  return NULL;
}

static block_t *request_block(size_t size) {
  if (size > (size_t)INTPTR_MAX - sizeof(block_t)) {
    return NULL;
  }
  void *mem = sbrk((intptr_t)(sizeof(block_t) + size));
  if (mem == (void *)-1) {
    return NULL;
  }
  block_t *b = (block_t *)mem;
  b->size = size;
  b->next = NULL;
  b->free = 0;

  if (list_tail == NULL) {
    list_head = b;
  } else {
    list_tail->next = b;
  }
  list_tail = b;
  return b;
}

static block_t *block_of(void *ptr) {
  assert(ptr != NULL);
  return (block_t *)ptr - 1;
}

void *mymalloc(size_t s) {
  if (s == 0) {
    return NULL;
  }
  debug_printf("Malloc %zu bytes\n", s);

  block_t *b = find_first_fit(s);
  if (b == NULL) {
    b = request_block(s);
    if (b == NULL) {
      return NULL;
    }
  }
  assert(b->size >= s);
  b->free = 0;
  return b + 1;
}

void *mycalloc(size_t nmemb, size_t s) {
  if (s != 0 && nmemb > SIZE_MAX / s) {
    return NULL;
  }
  size_t total = nmemb * s;

  void *p = mymalloc(total);
  if (p != NULL) {
    memset(p, 0, total);
  }
  debug_printf("Calloc %zu bytes\n", total);
  return p;
}

void myfree(void *ptr) {
  if (ptr == NULL) {
    return;
  }
  block_t *b = block_of(ptr);
  assert(!b->free);
  b->free = 1;
  debug_printf("Freed %zu bytes\n", b->size);
}
