#include "printf.h"
#include "rtos.h"
#include "stm32l476xx.h"
#include <unity.h>

#define CAPACITY 5

#define LINKED_LIST_TYPENAME test_list
#define LINKED_LIST_TYPE int
#define LINKED_LIST_CAPACITY CAPACITY
#include "linkedlist.h"

test_list list;

void setUp(void) {}

void tearDown(void) {}

void test_size_t_size(void) {
  TEST_ASSERT_EQUAL(sizeof(size_t), sizeof(uint32_t));
}

void test_init_size_0(void) {
  test_list_init(&list);
  TEST_ASSERT_EQUAL(0, test_list_size(&list));
}

void test_empty_after_init(void) {
  test_list_init(&list);
  int test[CAPACITY] = {0};
  int expected[CAPACITY] = {0};
  TEST_ASSERT_EQUAL(0, test_list_to_array(&list, CAPACITY, expected));
  TEST_ASSERT_EQUAL_UINT32_ARRAY(expected, test, CAPACITY);
}

void test_push_back(void) {
  test_list_init(&list);
  for (int i = 0; i < test_list_capacity(); i++) {
    TEST_ASSERT_TRUE(test_list_push_back(&list, i));
  }
  int actual[CAPACITY] = {0};
  int expected[CAPACITY] = {0, 1, 2, 3, 4};
  TEST_ASSERT_EQUAL(CAPACITY, test_list_to_array(&list, CAPACITY, actual));
  TEST_ASSERT_EQUAL_UINT32_ARRAY(expected, actual, CAPACITY);
}

void test_push_front(void) {
  test_list_init(&list);
  for (int i = 0; i < test_list_capacity(); i++) {
    TEST_ASSERT_TRUE(test_list_push_front(&list, i));
  }
  int expected[CAPACITY] = {4, 3, 2, 1, 0};
  int actual[CAPACITY] = {0};
  TEST_ASSERT_EQUAL(CAPACITY, test_list_to_array(&list, CAPACITY, actual));
  TEST_ASSERT_EQUAL_UINT32_ARRAY(expected, actual, CAPACITY);
}

void test_append_array(void) {
  test_list_init(&list);
  int expected[CAPACITY] = {1, 2, 3, 4, 5};
  int actual[CAPACITY] = {0};
  TEST_ASSERT_EQUAL(
      CAPACITY,
      test_list_append_array(&list, CAPACITY, expected)
  );
  TEST_ASSERT_EQUAL(CAPACITY, test_list_to_array(&list, CAPACITY, actual));
  TEST_ASSERT_EQUAL_UINT32_ARRAY(expected, actual, CAPACITY);
  // Test too big array
  test_list_init(&list);
  TEST_ASSERT_EQUAL(
      CAPACITY,
      test_list_append_array(&list, CAPACITY + 1, expected)
  );
  TEST_ASSERT_EQUAL(CAPACITY, test_list_to_array(&list, CAPACITY, actual));
  TEST_ASSERT_EQUAL_UINT32_ARRAY(expected, actual, CAPACITY);
}

void test_pop_back(void) {
  test_list_init(&list);
  int expected[CAPACITY] = {1, 2, 3, 4, 5};
  int actual[CAPACITY] = {0};
  TEST_ASSERT_EQUAL(
      CAPACITY,
      test_list_append_array(&list, CAPACITY, expected)
  );
  for (int i = CAPACITY; i > 0; i--) {
    int res;
    TEST_ASSERT_TRUE(test_list_pop_back(&list, &res));
    TEST_ASSERT_EQUAL(i, res);
    TEST_ASSERT_EQUAL(i - 1, test_list_to_array(&list, CAPACITY, actual));
    if (i != 1) {
      TEST_ASSERT_EQUAL_UINT32_ARRAY(expected, actual, i - 1);
    }
  }
}

void test_pop_front(void) {
  test_list_init(&list);
  int expected[CAPACITY] = {1, 2, 3, 4, 5};
  int actual[CAPACITY] = {0};
  TEST_ASSERT_EQUAL(
      CAPACITY,
      test_list_append_array(&list, CAPACITY, expected)
  );
  for (int i = 0; i < CAPACITY; i++) {
    int res;
    TEST_ASSERT_TRUE(test_list_pop_front(&list, &res));
    TEST_ASSERT_EQUAL(i + 1, res);
    TEST_ASSERT_EQUAL(
        CAPACITY - 1 - i,
        test_list_to_array(&list, CAPACITY, actual)
    );
    if (i != CAPACITY - 1) {
      TEST_ASSERT_EQUAL_UINT32_ARRAY(
          &expected[i + 1],
          actual,
          CAPACITY - 1 - i
      );
    }
  }
}

void test_insert(void) {
  int expected[CAPACITY] = {1, 2, 3, 4, 5};
  int actual[CAPACITY] = {0};
  // Test insert in between
  test_list_init(&list);
  int initial_in_between[3] = {1, 3, 5};
  TEST_ASSERT_EQUAL(3, test_list_append_array(&list, 3, initial_in_between));
  TEST_ASSERT_TRUE(test_list_insert_at(&list, 2, 1));
  TEST_ASSERT_TRUE(test_list_insert_at(&list, 4, 3));
  TEST_ASSERT_EQUAL(CAPACITY, test_list_to_array(&list, CAPACITY, actual));
  TEST_ASSERT_EQUAL_UINT32_ARRAY(expected, actual, CAPACITY);
  // Test insert at 0
  test_list_init(&list);
  int initial_at_0[4] = {2, 3, 4, 5};
  TEST_ASSERT_EQUAL(4, test_list_append_array(&list, 4, initial_at_0));
  TEST_ASSERT_TRUE(test_list_insert_at(&list, 1, 0));
  TEST_ASSERT_EQUAL(CAPACITY, test_list_to_array(&list, CAPACITY, actual));
  TEST_ASSERT_EQUAL_UINT32_ARRAY(expected, actual, CAPACITY);
  // Test insert at end
  test_list_init(&list);
  int initial_at_end[4] = {1, 2, 3, 4};
  TEST_ASSERT_EQUAL(4, test_list_append_array(&list, 4, initial_at_end));
  TEST_ASSERT_TRUE(test_list_insert_at(&list, 5, 4));
  TEST_ASSERT_EQUAL(CAPACITY, test_list_to_array(&list, CAPACITY, actual));
  TEST_ASSERT_EQUAL_UINT32_ARRAY(expected, actual, CAPACITY);
  // Test insert invalid
  test_list_init(&list);
  int initial_invalid[2] = {0, 1};
  TEST_ASSERT_EQUAL(2, test_list_append_array(&list, 2, initial_invalid));
  TEST_ASSERT_FALSE(test_list_insert_at(&list, 5, 4));
  TEST_ASSERT_EQUAL(2, test_list_to_array(&list, CAPACITY, actual));
  TEST_ASSERT_EQUAL_UINT32_ARRAY(initial_invalid, actual, 2);
}

void test_del(void) {
  int initial[CAPACITY] = {1, 2, 3, 4, 5};
  int actual[CAPACITY] = {0};
  int item;
  // Test delete in betweeen
  test_list_init(&list);
  TEST_ASSERT_EQUAL(CAPACITY, test_list_append_array(&list, CAPACITY, initial));
  int expected_in_between[3] = {1, 3, 5};
  TEST_ASSERT_TRUE(test_list_del_at(&list, 1, &item));
  TEST_ASSERT_EQUAL(2, item);
  TEST_ASSERT_TRUE(test_list_del_at(&list, 2, &item));
  TEST_ASSERT_EQUAL(4, item);
  TEST_ASSERT_EQUAL(3, test_list_to_array(&list, CAPACITY, actual));
  TEST_ASSERT_EQUAL_UINT32_ARRAY(expected_in_between, actual, 3);
  // Test delete end
  test_list_init(&list);
  TEST_ASSERT_EQUAL(CAPACITY, test_list_append_array(&list, CAPACITY, initial));
  int expected_at_end[4] = {1, 2, 3, 4};
  TEST_ASSERT_TRUE(test_list_del_at(&list, 4, &item));
  TEST_ASSERT_EQUAL(5, item);
  TEST_ASSERT_EQUAL(4, test_list_to_array(&list, CAPACITY, actual));
  TEST_ASSERT_EQUAL_UINT32_ARRAY(expected_at_end, actual, 4);
  // Test delete beginning
  test_list_init(&list);
  TEST_ASSERT_EQUAL(CAPACITY, test_list_append_array(&list, CAPACITY, initial));
  int expected_at_beginning[4] = {2, 3, 4, 5};
  TEST_ASSERT_TRUE(test_list_del_at(&list, 0, &item));
  TEST_ASSERT_EQUAL(1, item);
  TEST_ASSERT_EQUAL(4, test_list_to_array(&list, CAPACITY, actual));
  TEST_ASSERT_EQUAL_UINT32_ARRAY(expected_at_beginning, actual, 4);
  // Test invalid delete
  test_list_init(&list);
  int initial_invalid[2] = {0, 1};
  TEST_ASSERT_EQUAL(
      2,
      test_list_append_array(&list, 2, initial_invalid)
  );
  item = INT_MAX;
  TEST_ASSERT_FALSE(test_list_del_at(&list, 4, &item));
  TEST_ASSERT_EQUAL(INT_MAX, item);
  TEST_ASSERT_EQUAL(2, test_list_to_array(&list, CAPACITY, actual));
  TEST_ASSERT_EQUAL_UINT32_ARRAY(initial_invalid, actual, 2);
  // Test delete empty list
  test_list_init(&list);
  item = INT_MAX;
  for (int i = 0; i <= CAPACITY; i++) {
    TEST_ASSERT_FALSE(test_list_del_at(&list, i, &item));
    TEST_ASSERT_EQUAL(INT_MAX, item);
    TEST_ASSERT_EQUAL(0, test_list_size(&list));
    TEST_ASSERT_EQUAL(0, test_list_to_array(&list, CAPACITY, actual));
  }
}

int main() {
  SCnSCB->ACTLR |= SCnSCB_ACTLR_DISDEFWBUF_Msk;
  rtos_init();
  // printf("Beginning testing\n");

  UNITY_BEGIN();
  RUN_TEST(test_size_t_size);
  RUN_TEST(test_init_size_0);
  RUN_TEST(test_empty_after_init);
  RUN_TEST(test_push_back);
  RUN_TEST(test_push_front);
  RUN_TEST(test_append_array);
  RUN_TEST(test_pop_back);
  RUN_TEST(test_pop_front);
  RUN_TEST(test_insert);
  RUN_TEST(test_del);

  UNITY_END(); // stop unit testing

  while (1) {
  }
}
