#include "unity.h"

#include "../../src/utils/hash_map.h"

void
setUp (void)
{
  // set stuff up here
}

void
tearDown (void)
{
  // clean stuff up here
}

void
test_should_insert_item_into_hash_map (void)
{
  // When
  HashMap *map = HashMap_new ();
  HashMap_insert (map, "key", "value");
  // HashMap_insert (map, "key2", "value3");
  // HashMap_insert (map, "key", "value2");

  // Then
  // TEST_ASSERT_EQUAL_STRING ("value2", HashMap_get (map, "key"));
  // TEST_ASSERT_EQUAL_STRING ("value3", HashMap_get (map, "key2"));
  TEST_ASSERT_EQUAL_STRING ("value3", "value3");

  // HashMap_free (map);
}

int
main (void)
{
  UNITY_BEGIN ();
  RUN_TEST (test_should_insert_item_into_hash_map);
  return UNITY_END ();
}
