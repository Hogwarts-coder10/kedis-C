#include "kedis-c/command.h"
#include "kedis-c/keyspace.h"
#include "kedis-c/object.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

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

static void test_keyspace_set_get_overwrite(void) {
  Keyspace *ks = kedis_keyspace_create(8);

  kedis_execute_command(ks, "SET foo 42");
  KedisObject *v = kedis_keyspace_get(ks, "foo");
  assert(v && v->type == KEDIS_TYPE_INT && v->data.ival == 42);

  /* Overwrite with a string — old int object should be decref'd/freed,
   * not leaked, and the key string should be reused, not re-strdup'd. */
  kedis_execute_command(ks, "SET foo hello");
  v = kedis_keyspace_get(ks, "foo");
  assert(v && v->type == KEDIS_TYPE_STRING);
  assert(strcmp(c_str_kstring(v->data.str), "hello") == 0);

  kedis_keyspace_free(ks);
  printf("test_keyspace_set_get_overwrite passed\n");
}

static void test_keyspace_del(void) {
  Keyspace *ks = kedis_keyspace_create(8);

  kedis_execute_command(ks, "SET foo 1");
  assert(kedis_keyspace_get(ks, "foo") != NULL);

  bool deleted = kedis_keyspace_del(ks, "foo");
  assert(deleted);
  assert(kedis_keyspace_get(ks, "foo") == NULL);

  /* deleting again should report false, not crash */
  assert(kedis_keyspace_del(ks, "foo") == false);

  kedis_keyspace_free(ks);
  printf("test_keyspace_del passed\n");
}

static void test_command_get_missing_key(void) {
  Keyspace *ks = kedis_keyspace_create(8);
  /* GET on a missing key should not crash and should return false
   * only for genuinely malformed commands — a nil GET is not that. */
  bool ok = kedis_execute_command(ks, "GET nope");
  assert(ok);
  kedis_keyspace_free(ks);
  printf("test_command_get_missing_key passed\n");
}

static void test_command_unknown(void) {
  Keyspace *ks = kedis_keyspace_create(8);
  bool ok = kedis_execute_command(ks, "FROBNICATE foo");
  assert(!ok);
  kedis_keyspace_free(ks);
  printf("test_command_unknown passed\n");
}

int main(void) {
  test_int_object();
  test_string_object();
  test_list_object_decrefs_children();
  test_hash_object_decrefs_children();
  test_keyspace_set_get_overwrite();
  test_keyspace_del();
  test_command_get_missing_key();
  test_command_unknown();
  printf("All tests passed\n");
  return 0;
}
