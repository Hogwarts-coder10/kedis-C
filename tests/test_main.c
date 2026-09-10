#include "kedis-c/object.h"
#include <assert.h>
#include <stdio.h>

/*
 * Kedis-C — object model tests
 *
 * Focus: list/hash teardown must decr_ref every contained element,
 * not just free the container. We can't directly assert "no leak"
 * without a sanitizer, but we CAN assert refcounts drop as expected
 * before the container frees them — that's the behavior the bug was.
 */

static void test_int_object(void) {
  KedisObject *i = kedis_create_int_object(42);
  assert(i->type == KEDIS_TYPE_INT);
  assert(i->data.ival == 42);
  kedis_decr_ref(i);
  printf("test_int_object passed\n");
}

static void test_string_object(void) {
  KedisObject *s = kedis_create_str_object("hello");
  assert(s->type == KEDIS_TYPE_STRING);
  kedis_decr_ref(s);
  printf("test_string_object passed\n");
}

static void test_list_object_decrefs_children(void) {
  KedisObject *list = kedis_create_list_object();
  KedisObject *a = kedis_create_int_object(1);
  KedisObject *b = kedis_create_int_object(2);

  /* extra ref on `a` so we can prove decr_ref actually ran, without
   * reading freed memory: after the list is torn down, `a`'s refcount
   * should have dropped by exactly one (from the list's implicit ref). */
  kedis_incr_ref(a);

  cdsa_push_vector(list->data.list, &a);
  cdsa_push_vector(list->data.list, &b);

  kedis_decr_ref(list); /* should walk and decr_ref both a and b */

  assert(a->refcount ==
         1);         /* started at 2 (create + incr), list decref'd once */
  kedis_decr_ref(a); /* clean up our extra ref */

  printf("test_list_object_decrefs_children passed\n");
}

static void test_hash_object_decrefs_children(void) {
  KedisObject *hash = kedis_create_hash_object(8);
  KedisObject *v = kedis_create_int_object(99);

  kedis_incr_ref(v); /* same trick as the list test */

  insert_hashmap(hash->data.hash, "k1", v);

  kedis_decr_ref(hash); /* should walk and decr_ref v */

  assert(v->refcount == 1);
  kedis_decr_ref(v);

  printf("test_hash_object_decrefs_children passed\n");
}

int main(void) {
  test_int_object();
  test_string_object();
  test_list_object_decrefs_children();
  test_hash_object_decrefs_children();
  printf("All tests passed\n");
  return 0;
}
