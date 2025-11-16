// Intntionally not guarded
#include "macro.h"
#include <stddef.h>
#include <stdint.h>

#ifndef LINKED_LIST_TYPENAME
#ifdef MACRO_DEBUG
#define LINKED_LIST_TYPENAME debug_name
#else
#error This file should only be included if LINKED_LIST_TYPENAME is set
#endif /* ifdef MACRO_DEBUG */
#endif /* ifndef LINKED_LIST_TYPENAME */

#ifndef LINKED_LIST_TYPE
#ifdef MACRO_DEBUG
#define LINKED_LIST_TYPE int
#else
#error This file should only be included if LINKED_LIST_TYPE is set
#endif /* ifdef MACRO_DEBUG */
#endif /* ifndef LINKED_LIST_TYPE */

#ifndef LINKED_LIST_CAPACITY
#ifdef MACRO_DEBUG
#define LINKED_LIST_CAPACITY 1
#else
#error This file should only be included if LINKED_LIST_CAPACITY is set
#endif /* ifdef MACRO_DEBUG */
#endif /* ifndef LINKED_LIST_CAPACITY */

#if LINKED_LIST_CAPACITY == 0
#error LINKED_LIST_CAPACITY cannot be 0
#endif /* if LINKED_LIST_CAPACITY == 0 */

#define LL_INIT join2(LINKED_LIST_TYPENAME, init)
#define LL_SIZE join2(LINKED_LIST_TYPENAME, size)
#define LL_CAPACITY join2(LINKED_LIST_TYPENAME, capacity)
#define LL_INSERT_AT join2(LINKED_LIST_TYPENAME, insert_at)
#define LL_DEL_AT join2(LINKED_LIST_TYPENAME, del_at)
#define LL_PUSH_BACK join2(LINKED_LIST_TYPENAME, push_back)
#define LL_PUSH_FRONT join2(LINKED_LIST_TYPENAME, push_front)
#define LL_POP_BACK join2(LINKED_LIST_TYPENAME, pop_back)
#define LL_POP_FRONT join2(LINKED_LIST_TYPENAME, pop_front)
#define LL_TO_ARRAY join2(LINKED_LIST_TYPENAME, to_array)
#define LL_APPEND_ARRAY join2(LINKED_LIST_TYPENAME, append_array)
#define LL_GET join2(LINKED_LIST_TYPENAME, get)
#define LL_GET_FRONT join2(LINKED_LIST_TYPENAME, get_front)
#define LL_GET_BACK join2(LINKED_LIST_TYPENAME, get_back)

#define LL_NONE LINKED_LIST_CAPACITY
#define LL_NEXT(n) ll->nodes[n].next
#define LL_NODE_NAME join2(LINKED_LIST_TYPENAME, node)
#define LL_QUEUE_NAME join3(_, LINKED_LIST_TYPENAME, free_queue)

#define LL_NEW_NODE join3(_, LINKED_LIST_TYPENAME, new_node)
#define LL_DEL_NODE join3(_, LINKED_LIST_TYPENAME, del_node)
#define LL_END join3(_, LINKED_LIST_TYPENAME, end)
#define LL_PREV join3(_, LINKED_LIST_TYPENAME, prev)

#define QUEUE_TYPENAME LL_QUEUE_NAME
#define QUEUE_TYPE size_t
#define QUEUE_CAPACITY LINKED_LIST_CAPACITY
#define QUEUE_DONT_UNDEF
#include "queue.h"

typedef struct {
  LINKED_LIST_TYPE val;
  size_t next;
} LL_NODE_NAME;

typedef struct {
  LL_NODE_NAME nodes[LINKED_LIST_CAPACITY];
  size_t head;
  LL_QUEUE_NAME free;
} LINKED_LIST_TYPENAME;

void LL_INIT(LINKED_LIST_TYPENAME* ll) {
  Q_INIT_FN(&ll->free);
  for (int i = 0; i < LINKED_LIST_CAPACITY; i++) {
    Q_ENQUEUE_FN(&ll->free, i);
  }
  ll->head = LL_NONE;
}

size_t LL_CAPACITY() {
  return LINKED_LIST_CAPACITY;
}

size_t LL_SIZE(LINKED_LIST_TYPENAME* ll) {
  return Q_FREE_FN(&ll->free);
}

bool LL_NEW_NODE(
    LINKED_LIST_TYPENAME* ll,
    LINKED_LIST_TYPE item,
    size_t prev,
    size_t next,
    size_t* index
) {
  if (next != ll->head && prev >= LL_NONE) {
    return false;
  }
  size_t discard_index;
  if (index == NULL) {
    index = &discard_index;
  }
  if (!Q_DEQUEUE_FN(&ll->free, index)) {
    return false;
  }
  if (next == ll->head) {
    ll->head = *index;
  } else {
    // prev is known to be valid
    LL_NEXT(prev) = *index;
  }
  ll->nodes[*index] = (LL_NODE_NAME){.val = item, .next = next};
  return true;
}

bool LL_DEL_NODE(LINKED_LIST_TYPENAME* ll, size_t index, size_t prev, LINKED_LIST_TYPE* ret) {
  if (index >= LL_NONE) {
    return false;
  }
  LINKED_LIST_TYPE ret_backup;
  if (ret == NULL) {
    ret = &ret_backup;
  }
  *ret = ll->nodes[index].val;
  if (ll->head == index) {
    ll->head = LL_NEXT(ll->head);
  }else {
    if (prev >= LL_NONE) {
      return false;
    }
    LL_NEXT(prev) = LL_NEXT(index);
  }
  return Q_ENQUEUE_FN(&ll->free, index);
}

size_t LL_END(LINKED_LIST_TYPENAME* ll, size_t* prev) {
  size_t backup_prev;
  if (prev == NULL) {
    prev = &backup_prev;
  }
  *prev = LL_NONE;
  size_t node = ll->head;
  while (node < LL_NONE && LL_NEXT(node) < LL_NONE) {
    *prev = node;
    node = LL_NEXT(node);
  }
  return node;
}

size_t LL_PREV (LINKED_LIST_TYPENAME* ll, size_t index) {
  size_t node = ll->head;
  while (node < LL_NONE && LL_NEXT(node) != index) {
    node = LL_NEXT(node);
  }
  return node;
}

bool LL_PUSH_FRONT(LINKED_LIST_TYPENAME* ll, LINKED_LIST_TYPE item) {
  return LL_NEW_NODE(ll, item, LL_NONE, ll->head, NULL);
}

bool LL_POP_FRONT(LINKED_LIST_TYPENAME* ll, LINKED_LIST_TYPE* ret) {
  return LL_DEL_NODE(ll, ll->head, LL_NONE, ret);
}

bool LL_PUSH_BACK(LINKED_LIST_TYPENAME* ll, LINKED_LIST_TYPE item) {
  return LL_NEW_NODE(ll, item, LL_END(ll, NULL), LL_NONE, NULL);
}

bool LL_POP_BACK(LINKED_LIST_TYPENAME* ll, LINKED_LIST_TYPE* ret) {
  size_t prev;
  size_t end = LL_END(ll, &prev);
  return LL_DEL_NODE(ll, end, prev, ret);
}

bool LL_INSERT_AT (LINKED_LIST_TYPENAME* ll, LINKED_LIST_TYPE item, size_t index) {
  if (index == 0) {
    return LL_PUSH_FRONT(ll, item);
  }
  if (index == LL_SIZE(ll)) {
    return LL_PUSH_BACK(ll, item);
  }
  if (index > LL_SIZE(ll)) {
    return false;
  }
  size_t prev = ll->head;
  size_t next = LL_NEXT(prev);
  size_t current_index = 1;
  while (current_index < index) {
    prev = next;
    next = LL_NEXT(prev);
    current_index++;
  }
  return LL_NEW_NODE(ll, item, prev, next, NULL);
}

bool LL_DEL_AT (LINKED_LIST_TYPENAME* ll, size_t index, LINKED_LIST_TYPE* ret) {
  if (index == 0) {
    return LL_POP_FRONT(ll, ret);
  }
  if (index == LL_SIZE(ll)) {
    return LL_POP_BACK(ll, ret);
  }
  if (index > LL_SIZE(ll)) {
    return false;
  }
  size_t prev = ll->head;
  size_t node = LL_NEXT(prev);
  size_t current_index = 1;
  while (current_index < index) {
    prev = node;
    node = LL_NEXT(prev);
    current_index++;
  }
  return LL_DEL_NODE(ll, node, prev, ret);
}

size_t LL_TO_ARRAY(LINKED_LIST_TYPENAME* ll, size_t buffer_len, LINKED_LIST_TYPE* buffer) {
  size_t node = ll->head;
  size_t i = 0;
  while (node < LL_NONE) {
    buffer[i++] = ll->nodes[node].val;
    node = LL_NEXT(node);
  }
  return i;
}

size_t LL_APPEND_ARRAY(LINKED_LIST_TYPENAME* ll, size_t array_len, LINKED_LIST_TYPE* const arr){
  for (int i = 0; i < array_len; i++) {
    if (!LL_PUSH_BACK(ll, arr[i])){
      return i;
    }
  }
  return array_len;
}

bool LL_GET (LINKED_LIST_TYPENAME* ll, size_t index, LINKED_LIST_TYPE* item) {
  if (index >= LL_SIZE(ll)) {
    return false;
  }
  size_t node = ll->head;
  size_t current_index = 0;
  while (current_index < index){
    node = LL_NEXT(node);
    current_index++;
  }
  *item = ll->nodes[node].val;
  return true;
}

bool LL_GET_FRONT (LINKED_LIST_TYPENAME* ll, LINKED_LIST_TYPE* item) {
  return LL_GET(ll, 0, item);
}

bool LL_GET_BACK (LINKED_LIST_TYPENAME* ll, LINKED_LIST_TYPE* item) {
  return LL_GET(ll, LL_SIZE(ll) - 1, item);
}

#ifndef LINKED_LIST_DONT_UNDEF
#include "linkedlist_undef.h"
#endif /* LINKED_LIST_DONT_UNDEF */
