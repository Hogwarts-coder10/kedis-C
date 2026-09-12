#include "kedis-c/keyspace.h"
#include <stdlib.h>
#include <string.h>

Keyspace *kedis_keyspace_create(size_t initial_capacity) {
  Keyspace *ks = malloc(sizeof(Keyspace));
  if (!ks)
    return NULL;

  ks->map = cdsa_create_hashmap(initial_capacity);
  if (!ks->map) {
    free(ks);
    return NULL;
  }
  return ks;
}

bool kedis_keyspace_set(Keyspace *ks, const char *key, KedisObject *obj) {
  if (!ks || !key || !obj)
    return false;

  if (contains_hashmap(ks->map, key)) {
    /* internal_insert only overwrites the VALUE slot on a matching
     * key, per CDSA's own hashmap.c — the previously-stored key
     * pointer is untouched. So we reuse it and never re-strdup here. */
    KedisObject *old = get_hashmap(ks->map, key);
    kedis_decr_ref(old);
    insert_hashmap(ks->map, key, obj);
    return true;
  }

  /* New key: we own this string for as long as it lives in the map. */
  char *key_copy = strdup(key);
  if (!key_copy)
    return false;

  if (insert_hashmap(ks->map, key_copy, obj) != CDSA_OK) {
    free(key_copy);
    return false;
  }
  return true;
}

KedisObject *kedis_keyspace_get(Keyspace *ks, const char *key) {
  if (!ks || !key)
    return NULL;
  return (KedisObject *)get_hashmap(ks->map, key);
}

bool kedis_keyspace_del(Keyspace *ks, const char *key) {
  if (!ks || !key)
    return false;
  if (!contains_hashmap(ks->map, key))
    return false;

  KedisObject *old = get_hashmap(ks->map, key);

  /*
   * CDSA's hashmap has no "give me the key pointer you're holding"
   * call, so we walk the iterator to find our own strdup'd string and
   * free it after removal. O(n) — fine for now; worth revisiting if
   * DEL becomes a hot path (CDSA could expose a pop_hashmap that
   * hands back the owned key on removal instead).
   */
  char *owned_key = NULL;
  cdsa_hashmap_iterator *it = cdsa_create_hashmap_iterator(ks->map);
  if (it) {
    const char *k;
    void *v;
    while (cdsa_has_next_hashmap(it)) {
      if (cdsa_next_hashmap(it, &k, &v) != CDSA_OK)
        break;
      if (strcmp(k, key) == 0) {
        owned_key = (char *)k;
        break;
      }
    }
    cdsa_free_hashmap_iterator(it);
  }

  remove_hashmap(ks->map, key);
  kedis_decr_ref(old);
  free(owned_key);

  return true;
}

void kedis_keyspace_free(Keyspace *ks) {
  if (!ks)
    return;

  cdsa_hashmap_iterator *it = cdsa_create_hashmap_iterator(ks->map);
  if (it) {
    const char *k;
    void *v;
    while (cdsa_has_next_hashmap(it)) {
      if (cdsa_next_hashmap(it, &k, &v) != CDSA_OK)
        break;
      kedis_decr_ref((KedisObject *)v);
      free((char *)k);
    }
    cdsa_free_hashmap_iterator(it);
  }

  cdsa_free_hashmap(ks->map);
  free(ks);
}
