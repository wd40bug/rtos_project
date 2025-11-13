#define __Q_ADD(A, B, cap) ((A + B) % cap)

#define QUEUE_INIT(type, capacity)                                             \
  volatile typedef struct {                                                    \
    type data[capacity];                                                       \
    size_t head;                                                               \
    size_t size;                                                               \
  } type##_##capacity##_queue;                                                 \
  void type##_##capacity##_queue_init(type##_##capacity##_queue* Q) {          \
    Q->head = 0;                                                               \
    Q->size = 0;                                                               \
  }                                                                            \
  bool type##_##capacity##_queue_is_full(type##_##capacity##_queue* Q) {       \
    return Q->size == capacity;                                                \
  }                                                                            \
  bool type##_##capacity##_queue_is_empty(type##_##capacity##_queue* Q) {      \
    return Q->size == 0;                                                       \
  }                                                                            \
  bool type##_##capacity##_queue_enqueue(                                      \
      type##_##capacity##_queue* Q,                                            \
      type item                                                                \
  ) {                                                                          \
    if (Q->size == capacity) {                                                 \
      return false;                                                            \
    }                                                                          \
    Q->data[__Q_ADD(Q->head, Q->size, capacity)] = item;                       \
    Q->size++;                                                                 \
    return true;                                                               \
  }                                                                            \
  bool type##_##capacity##_queue_dequeue(                                      \
      type##_##capacity##_queue* Q,                                            \
      type* item                                                               \
  ) {                                                                          \
    if (Q->size == 0) {                                                        \
      return false;                                                            \
    }                                                                          \
    *item = Q->data[Q->head];                                                  \
    size_t new_head = __Q_ADD(Q->head, 1, capacity);                           \
    Q->size--;                                                                 \
    Q->head = new_head;                                                        \
    return true;                                                               \
  }                                                                            \
  size_t type##_##capacity##_queue_size(type##_##capacity##_queue* Q) {        \
    return Q->size;                                                            \
  }
