
// TODO: remove
#include <stdbool.h>
#include <stddef.h>
// #define QUEUE_TYPENAME print_queue
// #define QUEUE_TYPE char
// #define QUEUE_CAPACITY 256
#ifndef QUEUE_TYPENAME
#error This file should only be included if QUEUE_TYPENAME is set
#endif /* ifndef QUEUE_TYPENAME */
#ifndef QUEUE_TYPE
#error This file should only be included if QUEUE_TYPE is set
#endif /* ifndef QUEUE_TYPE */
#ifndef QUEUE_CAPACITY
#error This file should only be included if QUEUE_CAPACITY is set
#endif /* ifndef QUEUE_CAPCITY */

#define _cat2(A,B) A##B
#define cat2(A,B) _cat2(A,B)
#define cat3(A,B,C) cat2(A, cat2(B,C))
#define join2(A, B) cat3(A, _, B)

#define __Q_ADD(A, B, cap) ((A + B) % cap)
#define queue_name QUEUE_TYPENAME
#define queue_capacity QUEUE_CAPACITY
#define queue_type QUEUE_TYPE

#define init_fn join2(queue_name, init)
#define is_full_fn join2(queue_name, is_full)
#define is_empty_fn join2(queue_name, is_empty)
#define enqueue_fn join2(queue_name, enqueue)
#define dequeue_fn join2(queue_name, dequeue)
#define size_fn join2(queue_name, size)

typedef struct {
  queue_type data[queue_capacity];
  size_t head;
  size_t size;
} queue_name;

void init_fn(queue_name* Q) {
  Q->head = 0;
  Q->size = 0;
}

bool is_full_fn(queue_name* Q) {
  return Q->size == queue_capacity;
}

bool is_empty_fn(queue_name* Q) {
  return Q->size == 0;
}

bool enqueue_fn(queue_name* Q, queue_type item) {
  if (is_full_fn(Q)) {
    return false;
  }

  Q->data[__Q_ADD(Q->head, Q->size, queue_capacity)] = item;
  Q->size++;
  return true;
}

bool dequeue_fn(queue_name* Q, queue_type* item) {
  if (is_empty_fn(Q)) {
    return false;
  }
  *item = Q->data[Q->head];
  size_t new_head = __Q_ADD(Q->head, 1, queue_capacity);
  Q->size--;
  Q->head = new_head;
  return true;
}

size_t size_fn (queue_name* Q) {
  return Q->size;
}

// #define QUEUE_INIT(type, capacity)                                             \
//   volatile typedef struct {                                                    \
//     type data[capacity];                                                       \
//     size_t head;                                                               \
//     size_t size;                                                               \
//   } type##_##capacity##_queue;                                                 \
//   void type##_##capacity##_queue_init(type##_##capacity##_queue* Q) {          \
//     Q->head = 0;                                                               \
//     Q->size = 0;                                                               \
//   }                                                                            \
//   bool type##_##capacity##_queue_is_full(type##_##capacity##_queue* Q) {       \
//     return Q->size == capacity;                                                \
//   }                                                                            \
//   bool type##_##capacity##_queue_is_empty(type##_##capacity##_queue* Q) {      \
//     return Q->size == 0;                                                       \
//   }                                                                            \
//   bool type##_##capacity##_queue_enqueue(                                      \
//       type##_##capacity##_queue* Q,                                            \
//       type item                                                                \
//   ) {                                                                          \
//     if (Q->size == capacity) {                                                 \
//       return false;                                                            \
//     }                                                                          \
//     Q->data[__Q_ADD(Q->head, Q->size, capacity)] = item;                       \
//     Q->size++;                                                                 \
//     return true;                                                               \
//   }                                                                            \
//   bool type##_##capacity##_queue_dequeue(                                      \
//       type##_##capacity##_queue* Q,                                            \
//       type* item                                                               \
//   ) {                                                                          \
//     if (Q->size == 0) {                                                        \
//       return false;                                                            \
//     }                                                                          \
//     *item = Q->data[Q->head];                                                  \
//     size_t new_head = __Q_ADD(Q->head, 1, capacity);                           \
//     Q->size--;                                                                 \
//     Q->head = new_head;                                                        \
//     return true;                                                               \
//   }                                                                            \
//   size_t type##_##capacity##_queue_size(type##_##capacity##_queue* Q) {        \
//     return Q->size;                                                            \
//   }
