// Intentionally not guarded
#include "macro.h"
#include "util.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef HANDLED_LIST_TYPENAME
#ifdef MACRO_DEBUG
#define HANDLED_LIST_TYPENAME debug_name
#else
#error This file should only be included if HANDLED_LIST_TYPENAME is set
#endif /* ifdef MACRO_DEBUG */
#endif /* ifndef HANDLED_LIST_TYPENAME */

#ifndef HANDLED_LIST_TYPE
#ifdef MACRO_DEBUG
#define HANDLED_LIST_TYPE int
#else
#error This file should only be included if HANDLED_LIST_TYPE is set
#endif /* ifdef MACRO_DEBUG */
#endif /* ifndef HANDLED_LIST_TYPE */

#ifndef HANDLED_LIST_CAPACITY
#ifdef MACRO_DEBUG
#define HANDLED_LIST_CAPACITY 1
#else
#error This file should only be included if HANDLED_LIST_CAPACITY is set
#endif /* ifdef MACRO_DEBUG */
#endif /* ifndef HANDLED_LIST_CAPACITY */

#ifndef HANDLED_LIST_HANDLE_NAME
#ifdef MACRO_DEBUG
#define HANDLED_LIST_HANDLE_NAME debug_name_handle
#else
#error This file should only be included if HANDLED_LIST_ASSOCIATION_NAME is set
#endif /* ifdef MACRO_DEBUG */
#endif

#if HANDLED_LIST_CAPACITY == 0
#error HANDLED_LIST_CAPACITY cannot be 0
#endif /* if HANDLED_LIST_CAPACITY == 0 */

#define HL_INIT join2(HANDLED_LIST_TYPENAME, init)
#define HL_SIZE join2(HANDLED_LIST_TYPENAME, size)
#define HL_CAPACITY join2(HANDLED_LIST_TYPENAME, capacity)
#define HL_IS_FULL join2(HANDLED_LIST_TYPENAME, is_full)
#define HL_INSERT join2(HANDLED_LIST_TYPENAME, insert)
#define HL_GET join2(HANDLED_LIST_TYPENAME, get)
#define HL_GET_REF join2(HANDLED_LIST_TYPENAME, get_ref)
#define HL_SET join2(HANDLED_LIST_TYPENAME, set)
#define HL_FIND join2(HANDLED_LIST_TYPENAME, find)
#define HL_IS_VALID join2(HANDLED_LIST_TYPENAME, is_valid)
#define HL_TO_ARRAY join2(HANDLED_LIST_TYPENAME, to_array)
#define HL_ADD_ARRAY join2(HANDLED_LIST_TYPENAME, add_array)

typedef size_t HANDLED_LIST_HANDLE_NAME;

typedef struct {
  HANDLED_LIST_HANDLE_NAME num_handles;
  HANDLED_LIST_TYPE data[HANDLED_LIST_CAPACITY];
} HANDLED_LIST_TYPENAME;

void HL_INIT(HANDLED_LIST_TYPENAME* hl) {
  hl->num_handles = 0;
}

size_t HL_SIZE(HANDLED_LIST_TYPENAME* hl) {
  return hl->num_handles;
}

size_t HL_CAPACITY() {
  return HANDLED_LIST_CAPACITY;
}

bool HL_IS_FULL(HANDLED_LIST_TYPENAME* hl) {
  return hl->num_handles == HANDLED_LIST_CAPACITY;
}

bool HL_INSERT(
    HANDLED_LIST_TYPENAME* hl,
    HANDLED_LIST_TYPE item,
    HANDLED_LIST_HANDLE_NAME* handle
) {
  if (HL_IS_FULL(hl)) {
    return false;
  }
  hl->data[hl->num_handles] = item;
  *handle = hl->num_handles;
  hl->num_handles++;
  return true;
}

bool HL_IS_VALID(HANDLED_LIST_TYPENAME* hl, HANDLED_LIST_HANDLE_NAME handle) {
  return handle < hl->num_handles;
}

bool HL_GET(
    HANDLED_LIST_TYPENAME* hl,
    HANDLED_LIST_HANDLE_NAME handle,
    HANDLED_LIST_TYPE* ret
) {
  NULL_GUARD(HANDLED_LIST_TYPE, ret);
  if (!HL_IS_VALID(hl, handle)) {
    return false;
  }
  *ret = hl->data[handle];
  return true;
}

bool HL_GET_REF(
    HANDLED_LIST_TYPENAME* hl,
    HANDLED_LIST_HANDLE_NAME handle,
    HANDLED_LIST_TYPE** ret
) {
  NULL_GUARD(HANDLED_LIST_TYPE*, ret);
  if (!HL_IS_VALID(hl, handle)) {
    return false;
  }
  *ret = &hl->data[handle];
  return true;
}

bool HL_SET(
    HANDLED_LIST_TYPENAME* hl,
    HANDLED_LIST_HANDLE_NAME handle,
    HANDLED_LIST_TYPE new_val
) {
  if (!HL_IS_VALID(hl, handle)) {
    return false;
  }
  hl->data[handle] = new_val;
  return true;
}

bool HL_FIND(
    HANDLED_LIST_TYPENAME* hl,
    bool (*filter)(HANDLED_LIST_TYPE),
    HANDLED_LIST_HANDLE_NAME* handle
) {
  NULL_GUARD(HANDLED_LIST_HANDLE_NAME, handle);
  for (HANDLED_LIST_HANDLE_NAME i = 0; i < hl->num_handles; i++) {
    if (filter(hl->data[i])) {
      *handle = i;
      return true;
    }
  }
  return false;
}

size_t HL_TO_ARRAY(
    HANDLED_LIST_TYPENAME* hl,
    size_t buffer_sizes,
    HANDLED_LIST_TYPE* buffer,
    HANDLED_LIST_HANDLE_NAME* handles
) {
  for (HANDLED_LIST_HANDLE_NAME i = 0; i < hl->num_handles && i < buffer_sizes;
       i++) {
    buffer[i] = hl->data[i];
    handles[i] = i;
  }
  return buffer_sizes < hl->num_handles ? buffer_sizes : hl->num_handles;
}

size_t HL_ADD_ARRAY(
    HANDLED_LIST_TYPENAME* hl,
    size_t array_len,
    HANDLED_LIST_TYPE* const arr,
    HANDLED_LIST_HANDLE_NAME* handles
) {
  for (int i = 0; i < array_len; i++) {
    if (!HL_INSERT(hl, arr[i], &handles[i])) {
      return i;
    }
  }
  return array_len;
}

#ifndef HANDLED_LIST_DONT_UNDEF
#include "handledlist_undef.h"
#endif /* ifndef LINKED_LIST_DONT_UNDEF */
