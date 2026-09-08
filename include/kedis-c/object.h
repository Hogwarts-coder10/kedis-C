#ifndef KEDIS_C_OBJECT_H
#define KEDIS_C_OBJECT_H

#include <CDSA/hashmap.h>
#include <CDSA/kstring.h>
#include <CDSA/vector.h>
#include <stddef.h>

/*
 * Kedis-C — core value representation (tagged union), modeled on
 * Redis's robj.
 *
 * Every value stored in the keyspace is a KedisObject. The `type`
 * field says which member of the union is live. `refcount` exists
 * up front even though Phase 1 won't do real ref-sharing yet —
 * retrofitting refcounting later touches every call site, so it's
 * cheaper to carry the field from day one and start it at 1.
 */

typedef enum {
  KEDIS_TYPE_INT,    // native long -> avoids alloc for small integers
  KEDIS_TYPE_STRING, // backed by CDSA's cdsa_kstring
  KEDIS_TYPE_LIST,   // backed by CDSA's cdsa_vector
  KEDIS_TYPE_HASH,   // backed by CDSA's cdsa_hashmap
} KedisType;

typedef struct KedisObject {
  KedisType type;
  int refcount;

  union {
    long ival;
    cdsa_kstring *str;
    cdsa_vector *list;
    cdsa_hashmap *hash;
  } data;
} KedisObject;

/* --- Construction --- */
KedisObject *kedis_create_int_object(long value);
KedisObject *kedis_create_list_object(
    void); /* wraps cdsa_create_vector(sizeof(KedisObject*)) */
KedisObject *kedis_create_str_object(
    const char *value); /* wraps cdsa_create_kstring + append_kstring */
KedisObject *kedis_create_hash_object(
    size_t initial_capacity); /* wraps cdsa_create_hashmap(capacity) */

/* --- RefCounting --- */
void kedis_incr_ref(KedisObject *obj);
void kedis_decr_ref(KedisObject *obj); // frees at refcount 0

/* --- TearDown --- */
void kedis_free_object(KedisObject *obj);

#endif
