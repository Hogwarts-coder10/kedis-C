#include "kedis-c/object.h"
#include <stdlib.h>

KedisObject *kedis_create_int_object(long value) {
  KedisObject *obj = malloc(sizeof(KedisObject));
  if (!obj)
    return NULL;
  obj->type = KEDIS_TYPE_INT;
  obj->refcount = 1;
  obj->data.ival = value;
  return obj;
}

KedisObject *kedis_create_list_object(void) {
  KedisObject *obj = malloc(sizeof(KedisObject));
  if (!obj)
    return NULL;
  obj->type = KEDIS_TYPE_LIST;
  obj->refcount = 1;
  obj->data.list = cdsa_create_vector(sizeof(KedisObject *));
  if (!obj->data.list) {
    free(obj);
    return NULL;
  }
  return obj;
}

KedisObject *kedis_create_string_object(const char *value) {
  KedisObject *obj = malloc(sizeof(KedisObject));
  if (!obj)
    return NULL;

  cdsa_kstring *str = cdsa_create_kstring();
  if (!str) {
    free(obj);
    return NULL;
  }
  if (value && append_kstring(str, value) != CDSA_OK) {
    cdsa_free_kstring(str);
    free(obj);
    return NULL;
  }

  obj->type = KEDIS_TYPE_STRING;
  obj->refcount = 1;
  obj->data.str = str;
  return obj;
}

KedisObject *kedis_create_hash_object(size_t initial_capacity) {
  KedisObject *obj = malloc(sizeof(KedisObject));
  if (!obj)
    return NULL;

  obj->data.hash = cdsa_create_hashmap(initial_capacity);
  if (!obj->data.hash) {
    free(obj);
    return NULL;
  }

  obj->type = KEDIS_TYPE_HASH;
  obj->refcount = 1;
  return obj;
}

void kedis_incr_ref(KedisObject *obj) {
  if (obj)
    obj->refcount++;
}

void kedis_decr_ref(KedisObject *obj) {
  if (!obj)
    return;
  obj->refcount--;
  if (obj->refcount <= 0) {
    kedis_free_object(obj);
  }
}

void kedis_free_object(KedisObject *obj) {
  if (!obj)
    return;

  switch (obj->type) {
  case KEDIS_TYPE_INT:
    /* nothing to free — native long */
    break;
  case KEDIS_TYPE_STRING:
    cdsa_free_kstring(obj->data.str);
    break;
  case KEDIS_TYPE_LIST:
    /*
     * NOTE: this only frees the vector's own backing array, not
     * the KedisObject* elements it holds. Whoever tears down a
     * list is responsible for kedis_decr_ref'ing every element
     * first (or wire that walk in here once the vector iterator
     * is in play) — CDSA vectors don't own their element memory.
     */
    cdsa_free_vector(obj->data.list);
    break;
  case KEDIS_TYPE_HASH:
    /*
     * Same caveat as lists: cdsa_hashmap doesn't own its values
     * (see hashmap.h ownership note), so stored KedisObject*
     * values need decr_ref'ing by the caller before this runs.
     */
    cdsa_free_hashmap(obj->data.hash);
    break;
  }

  free(obj);
}
