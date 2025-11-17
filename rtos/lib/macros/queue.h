#include "macro.h"
#include "util.h"
#include <stdbool.h>
#include <stddef.h>

#ifndef QUEUE_TYPENAME
#ifdef MACRO_DEBUG
#define QUEUE_TYPENAME debug_name
#else
#error This file should only be included if QUEUE_TYPENAME is set
#endif /* MACRO_DEBUG */
#endif /* ifndef QUEUE_TYPENAME */

#ifndef QUEUE_TYPE
#ifdef MACRO_DEBUG
#define QUEUE_TYPE int
#else
#error This file should only be included if QUEUE_TYPE is set
#endif /*MACRO_DEBUG*/
#endif /* ifndef QUEUE_TYPE */

#ifndef QUEUE_CAPACITY
#ifdef MACRO_DEBUG
#define QUEUE_CAPACITY 1
#else
#error This file should only be included if QUEUE_CAPACITY is set
#endif /* ifdef MACRO_DEBUG */
#endif /* ifndef QUEUE_CAPCITY */

#if QUEUE_CAPACITY == 0
#error Queue Capacity must be greater than 0
#endif

#define __Q_ADD(A, B) ((A + B) % QUEUE_CAPACITY)

#define Q_INIT_FN join2(QUEUE_TYPENAME, init)
#define Q_IS_FULL_FN join2(QUEUE_TYPENAME, is_full)
#define Q_IS_EMPTY_FN join2(QUEUE_TYPENAME, is_empty)
#define Q_ENQUEUE_FN join2(QUEUE_TYPENAME, enqueue)
#define Q_DEQUEUE_FN join2(QUEUE_TYPENAME, dequeue)
#define Q_SIZE_FN join2(QUEUE_TYPENAME, size)
#define Q_FREE_FN join2(QUEUE_TYPENAME, free)
#define Q_CONTAINS_FN join2(QUEUE_TYPENAME, contains)
#define Q_ENQUEUE_ALL_FN join2(QUEUE_TYPENAME, enqueue_all)
#define Q_DEQUEUE_ALL_FN join2(QUEUE_TYPENAME, dequeue_all)

typedef struct {
  QUEUE_TYPE data[QUEUE_CAPACITY];
  size_t head;
  size_t size;
} QUEUE_TYPENAME;

void Q_INIT_FN(QUEUE_TYPENAME* Q) {
  Q->head = 0;
  Q->size = 0;
}

bool Q_IS_FULL_FN(QUEUE_TYPENAME* Q) {
  return Q->size == QUEUE_CAPACITY;
}

bool Q_IS_EMPTY_FN(QUEUE_TYPENAME* Q) {
  return Q->size == 0;
}

bool Q_ENQUEUE_FN(QUEUE_TYPENAME* Q, QUEUE_TYPE item) {
  if (Q_IS_FULL_FN(Q)) {
    return false;
  }

  Q->data[__Q_ADD(Q->head, Q->size)] = item;
  Q->size++;
  return true;
}

bool Q_DEQUEUE_FN(QUEUE_TYPENAME* Q, QUEUE_TYPE* item) {
  NULL_GUARD(QUEUE_TYPE, item);
  if (Q_IS_EMPTY_FN(Q)) {
    return false;
  }
  *item = Q->data[Q->head];
  size_t new_head = __Q_ADD(Q->head, 1);
  Q->size--;
  Q->head = new_head;
  return true;
}

size_t Q_SIZE_FN(QUEUE_TYPENAME* Q) {
  return Q->size;
}

size_t Q_FREE_FN(QUEUE_TYPENAME* Q) {
  return QUEUE_CAPACITY - Q->size;
}

bool Q_CONTAINS_FN(QUEUE_TYPENAME* Q, QUEUE_TYPE item){
  for (int i = Q->head; i != __Q_ADD(Q->head, Q->size); i = __Q_ADD(i, 1)){
    if (Q->data[i] == item){
      return true;
    }
  }
  return false;
}

size_t Q_ENQUEUE_ALL_FN (QUEUE_TYPENAME* Q, size_t buffer_size, QUEUE_TYPE* const buffer){
  for (int i = 0; i < buffer_size; i++) {
    if (!Q_ENQUEUE_FN(Q, buffer[i])){
      return i;
    }
  }
  return buffer_size;
}

size_t Q_DEQUEUE_ALL_FN (QUEUE_TYPENAME* Q, size_t buffer_size, QUEUE_TYPE* buffer){
  for (int i = 0; i < buffer_size; i++){
    if (!Q_DEQUEUE_FN(Q, &buffer[i])){
      return i;
    }
  }
  return buffer_size;
}

#ifndef QUEUE_DONT_UNDEF
#include "queue_undef.h"
#endif /* QUEUE_DONT_UNDEF */
