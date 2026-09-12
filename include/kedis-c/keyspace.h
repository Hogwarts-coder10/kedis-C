#ifndef KEDIS_C_KEYSPACE_H
#define KEDIS_C_KEYSPACE_H

#include "kedis-c/object.h"
#include <stdbool.h>
#include <stddef.h>

/*
 * Kedis-C — keyspace (the global key -> KedisObject* map)
 *
 * Wraps a cdsa_hashmap. CDSA's hashmap explicitly does NOT own key or
 * value memory (see hashmap.h's @ownership notes) — it only stores the
 * pointers it's given. That makes the keyspace responsible for two
 * separate lifetimes:
 *   - VALUES: KedisObject* refcounting, via kedis_incr_ref/kedis_decr_ref.
 *   - KEYS: plain C strings, which the keyspace strdup's on first insert
 *     and must free itself on delete/overwrite/teardown.
 */

typedef struct Keyspace {
  cdsa_hashmap *map;
} Keyspace;

Keyspace *kedis_keyspace_create(size_t initial_capacity);
void kedis_keyspace_free(Keyspace *ks);

/*
 * Sets key to obj. Takes ownership of obj — the caller should
 * kedis_incr_ref beforehand if it needs to keep its own reference.
 * If key already exists, the old value is decref'd and the existing
 * (owned) key string is reused rather than strdup'd again.
 */
bool kedis_keyspace_set(Keyspace *ks, const char *key, KedisObject *obj);

/*
 * Returns the object for key, or NULL if absent. Does NOT incr_ref —
 * this is a borrowed reference, valid only until the next mutation of
 * this key. Callers that need to retain it must incr_ref explicitly.
 */
KedisObject *kedis_keyspace_get(Keyspace *ks, const char *key);

/*
 * Removes key if present: decrefs its value and frees the owned key
 * string. Returns true if a key was actually removed.
 */
bool kedis_keyspace_del(Keyspace *ks, const char *key);

#endif /* KEDIS_C_KEYSPACE_H */
