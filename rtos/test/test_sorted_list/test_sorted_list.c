#include "rtos.h"
#include <unity.h>

#define CAPACITY 5

int debug_comparator(int a, int b){
  return a - b;
}
#define SORTED_LIST_TYPENAME test_list
#define SORTED_LIST_TYPE int
#define SORTED_LIST_CAPACITY CAPACITY
#define SORTED_LIST_COMPARATOR debug_comparator
#include "sortedlist.h"

test_list list;

void setUp(void) {}
void tearDown(void) {}

void test_sorted_creation(void) {
  test_list_init(&list);
  int ini_rev[CAPACITY] = {5,4,3,2,1};
  TEST_ASSERT_EQUAL(CAPACITY, test_list_add_array(&list, CAPACITY, ini_rev));
  int expected[CAPACITY] = {1,2,3,4,5};
  int actual[CAPACITY] = {0};
  TEST_ASSERT_EQUAL(CAPACITY, test_list_to_array(&list, CAPACITY, actual));
  TEST_ASSERT_EQUAL_INT_ARRAY(expected, actual, CAPACITY);

  test_list_init(&list);
  int ini[CAPACITY] = {1,2,3,4,5};
  TEST_ASSERT_EQUAL(CAPACITY, test_list_add_array(&list, CAPACITY, ini));
  TEST_ASSERT_EQUAL(CAPACITY, test_list_to_array(&list, CAPACITY, actual));
  TEST_ASSERT_EQUAL_INT_ARRAY(expected, actual, CAPACITY);
}

void test_insert(void) {
  test_list_init(&list);
  int ini[2] = {2,4};
  int actual[CAPACITY] = {0};
  TEST_ASSERT_EQUAL(2, test_list_add_array(&list, 2, ini));

  //first
  int expected_first[3] = {1,2,4};
  TEST_ASSERT_TRUE(test_list_insert(&list, 1));
  TEST_ASSERT_EQUAL(3, test_list_to_array(&list, CAPACITY, actual));
  TEST_ASSERT_EQUAL_INT_ARRAY(expected_first, actual, 3);

  //second
  int expected_second[4] = {1,2,4,5};
  TEST_ASSERT_TRUE(test_list_insert(&list, 5));
  TEST_ASSERT_EQUAL(4, test_list_to_array(&list, CAPACITY, actual));
  TEST_ASSERT_EQUAL_INT_ARRAY(expected_second, actual, 4);

  //last
  int expected_last[5] = {1,2,3,4,5};
  TEST_ASSERT_TRUE(test_list_insert(&list, 3));
  TEST_ASSERT_EQUAL(CAPACITY, test_list_to_array(&list, CAPACITY, actual));
  TEST_ASSERT_EQUAL_INT_ARRAY(expected_last, actual, CAPACITY);
}

int main() {
  rtos_init();

  UNITY_BEGIN();
  RUN_TEST(test_sorted_creation);
  RUN_TEST(test_insert);

  UNITY_END();

  while (1) {
  }
}
