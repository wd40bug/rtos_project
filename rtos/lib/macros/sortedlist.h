// Intentionally not guarded
#include "macro.h"

#ifndef SORTED_LIST_TYPENAME
#ifdef MACRO_DEBUG
#define SORTED_LIST_TYPENAME debugdebug_name
#else
#error This file should only be included if SORTED_LIST_TYPENAME is set
#endif /* ifndef MACRO_DEBUG */
#endif /* SORTED_LIST_TYPENAME */

#ifndef SORTED_LIST_COMPARATOR
#ifdef MACRO_DEBUG
int debug_comparator(int a, int b) {
  return a - b;
}
#define SORTED_LIST_COMPARATOR debug_comparator
#else
#error This file should only be included if SORTED_LIST_COMPARATOR is set
#endif /* ifdef MACRO_DEBUG */
#endif /* ifndef SORTED_LIST_COMPARATOR */

#ifndef SORTED_LIST_TYPE
#ifdef MACRO_DEBUG
#define SORTED_LIST_TYPE int
#else
#error This file should only be included if SORTED_LIST_TYPE is set
#endif /* ifdef MACRO_DEBUG */
#endif /* ifndef SORTED_LIST_TYPE */

#ifndef SORTED_LIST_CAPACITY
#ifdef MACRO_DEBUG
#define SORTED_LIST_CAPACITY 1
#else
#error This file should only be included if SORTED_LIST_CAPACITY is set
#endif /* ifdef MACRO_DEBUG */
#endif /* ifndef SORTED_LIST_CAPACITY */

#if SORTED_LIST_CAPACITY == 0
#error SORTEDSORTED_LIST_CAPACITY cannot be 0
#endif /* if SORTED_LIST_CAPACITY == 0 */

#define SL_INIT join2(SORTED_LIST_TYPENAME, init)
#define SL_SIZE join2(SORTED_LIST_TYPENAME, size)
#define SL_CAPACITY join2(SORTED_LIST_TYPENAME, capacity)
#define SL_INSERT join2(SORTED_LIST_TYPENAME, insert)
#define SL_POP_FRONT join2(SORTED_LIST_TYPENAME, pop_front)
#define SL_POP_BACK join2(SORTED_LIST_TYPENAME, pop_back)
#define SL_DEL_AT join2(SORTED_LIST_TYPENAME, del_at)
#define SL_TO_ARRAY join2(SORTED_LIST_TYPENAME, to_array)
#define SL_ADD_ARRAY join2(SORTED_LIST_TYPENAME, add_array)
#define SL_GET join2(SORTED_LIST_TYPENAME, get)
#define SL_GET_FRONT join2(SORTED_LIST_TYPENAME, get_front)
#define SL_GET_BACK join2(SORTED_LIST_TYPENAME, get_back)

#define SL_LL_NAME join3(_, SORTED_LIST_TYPENAME, ll)

#define LINKED_LIST_DONT_UNDEF
#define LINKED_LIST_TYPENAME SL_LL_NAME
#define LINKED_LIST_TYPE SORTED_LIST_TYPE
#define LINKED_LIST_CAPACITY SORTED_LIST_CAPACITY
#include "linkedlist.h"

typedef struct {
  SL_LL_NAME list;
} SORTED_LIST_TYPENAME;

void SL_INIT(SORTED_LIST_TYPENAME* sl) {
  LL_INIT(&sl->list);
}

size_t SL_SIZE(SORTED_LIST_TYPENAME* sl) {
  return LL_SIZE(&sl->list);
}

size_t SL_CAPACITY() {
  return SORTED_LIST_CAPACITY;
}

bool SL_INSERT(SORTED_LIST_TYPENAME* sl, SORTED_LIST_TYPE item) {
  SL_LL_NAME* ll = &sl->list;
  if (ll->head >= LL_NONE || SORTED_LIST_COMPARATOR(item, ll->nodes[ll->head].val) <= 0){
    return LL_PUSH_FRONT(ll, item);
  }
  size_t prev = ll->head;
  size_t next = LL_NEXT(prev);
  while (next < LL_NONE && SORTED_LIST_COMPARATOR(item, ll->nodes[next].val) > 0) {
    prev = next;
    next = LL_NEXT(prev);
  }
  return LL_NEW_NODE(ll, item, prev, next, NULL);
}

bool SL_POP_FRONT(SORTED_LIST_TYPENAME* sl, SORTED_LIST_TYPE* ret) {
  return LL_POP_FRONT(&sl->list, ret);
}

bool SL_POP_BACK(SORTED_LIST_TYPENAME* sl, SORTED_LIST_TYPE* ret) {
  return LL_POP_BACK(&sl->list, ret);
}

bool SL_DEL_AT(SORTED_LIST_TYPENAME* sl, size_t index, SORTED_LIST_TYPE* ret) {
  return LL_DEL_AT(&sl->list, index, ret);
}

size_t SL_TO_ARRAY(
    SORTED_LIST_TYPENAME* sl, size_t buffer_len, SORTED_LIST_TYPE* buffer
) {
  return LL_TO_ARRAY(&sl->list, buffer_len, buffer);
}

size_t SL_ADD_ARRAY(SORTED_LIST_TYPENAME* sl, size_t array_len, SORTED_LIST_TYPE* arr){
  for (int i = 0; i < array_len; i++) {
    if (!SL_INSERT(sl, arr[i])){
      return i;
    }
  }
  return array_len;
}

bool SL_GET(SORTED_LIST_TYPENAME* sl, size_t index, SORTED_LIST_TYPE* item){
  return LL_GET (&sl->list, index, item);
}

bool SL_GET_FRONT(SORTED_LIST_TYPENAME* sl, SORTED_LIST_TYPE* item) {
  return LL_GET_FRONT (&sl->list, item);
}

bool SL_GET_BACK(SORTED_LIST_TYPENAME* sl, SORTED_LIST_TYPE* item){
  return LL_GET_BACK (&sl->list, item);
}

#ifndef SORTED_LIST_DONT_UNDEF
#include "sortedlist_undef.h"
#endif /* ifndef SORTED_LIST_DONT_UNDEF */
