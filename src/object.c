#include "kedis-c/object.h"
#include <CDSA/kstring.h>
#include <CDSA/vector.h>
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

KedisObject *kedis_create_str_object(const char *value) {
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
  case KEDIS_TYPE_LIST: {
    /*
     * cdsa_vector stores KedisObject* BY VALUE (memcpy'd into its
     * backing array, per cdsa_push_vector's ownership note), so
     * cdsa_next_vector yields a pointer INTO that array — i.e. a
     * KedisObject**. Deref once to get the actual KedisObject*.
     */
    cdsa_vector_iterator *it = cdsa_create_vector_iterator(obj->data.list);
    if (it) {
      void *slot;
      while (cdsa_has_next_vector(it)) {
        if (cdsa_next_vector(it, &slot) != CDSA_OK)
          break;
        kedis_decr_ref(*(KedisObject **)slot);
      }
      cdsa_free_vector_iterator(it);
    }
    cdsa_free_vector(obj->data.list);
    break;
  }
  case KEDIS_TYPE_HASH: {
    /*
     * cdsa_hashmap stores the VALUE POINTER directly (per
     * insert_hashmap's ownership note — "the hashmap only stores
     * the pointer"), so cdsa_next_hashmap's out_value IS the
     * KedisObject* itself — no extra deref, unlike the vector case.
     *
     * Keys are NOT freed here: per hashmap.h, the caller owns key
     * memory and it must outlive the map. Kedis-C's keyspace layer
     * is responsible for key string lifetime, not this object.
     */
    cdsa_hashmap_iterator *it = cdsa_create_hashmap_iterator(obj->data.hash);
    if (it) {
      const char *key;
      void *value;
      while (cdsa_has_next_hashmap(it)) {
        if (cdsa_next_hashmap(it, &key, &value) != CDSA_OK)
          break;
        kedis_decr_ref((KedisObject *)value);
      }
      cdsa_free_hashmap_iterator(it);
    }
    cdsa_free_hashmap(obj->data.hash);
    break;
  }
  }

  free(obj);
}
