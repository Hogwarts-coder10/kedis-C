#include "kedis-c/command.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> /* strcasecmp */

/* Tries to parse `s` as a whole integer with no trailing junk.
 * Used to decide whether SET stores KEDIS_TYPE_INT or KEDIS_TYPE_STRING —
 * a small piece of the "tagged union, not everything's a string" design. */
static bool parse_long(const char *s, long *out) {
  if (!s || *s == '\0')
    return false;
  char *end;
  long v = strtol(s, &end, 10);
  if (*end != '\0')
    return false; /* trailing non-digit chars */
  *out = v;
  return true;
}

static void cmd_set(Keyspace *ks, char *key, char *value) {
  if (!key || !value) {
    printf("(error) ERR wrong number of arguments for 'set' command\n");
    return;
  }

  long ival;
  KedisObject *obj = parse_long(value, &ival) ? kedis_create_int_object(ival)
                                              : kedis_create_str_object(value);

  if (!obj) {
    printf("(error) ERR out of memory\n");
    return;
  }

  if (!kedis_keyspace_set(ks, key, obj)) {
    kedis_decr_ref(obj);
    printf("(error) ERR failed to set key\n");
    return;
  }

  printf("OK\n");
}

static void cmd_get(Keyspace *ks, char *key) {
  if (!key) {
    printf("(error) ERR wrong number of arguments for 'get' command\n");
    return;
  }

  KedisObject *obj = kedis_keyspace_get(ks, key);
  if (!obj) {
    printf("(nil)\n");
    return;
  }

  switch (obj->type) {
  case KEDIS_TYPE_INT:
    printf("(integer) %ld\n", obj->data.ival);
    break;
  case KEDIS_TYPE_STRING:
    printf("\"%s\"\n", c_str_kstring(obj->data.str));
    break;
  case KEDIS_TYPE_LIST:
    printf("(list with %zu elements)\n", cdsa_size_vector(obj->data.list));
    break;
  case KEDIS_TYPE_HASH:
    printf("(hash with %zu fields)\n", cdsa_size_hashmap(obj->data.hash));
    break;
  }
}

static void cmd_del(Keyspace *ks, char *key) {
  if (!key) {
    printf("(error) ERR wrong number of arguments for 'del' command\n");
    return;
  }
  printf("(integer) %d\n", kedis_keyspace_del(ks, key) ? 1 : 0);
}

bool kedis_execute_command(Keyspace *ks, const char *line) {
  if (!ks || !line)
    return false;

  /* strtok mutates its input, so work on a local copy rather than
   * the caller's string. No quoting support yet — plain whitespace
   * split is enough to exercise the object model end to end. */
  char buf[512];
  strncpy(buf, line, sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = '\0';

  char *cmd = strtok(buf, " \t\r\n");
  if (!cmd)
    return false; /* blank line */

  char *arg1 = strtok(NULL, " \t\r\n");
  char *arg2 = strtok(NULL, " \t\r\n");

  if (strcasecmp(cmd, "SET") == 0) {
    cmd_set(ks, arg1, arg2);
  } else if (strcasecmp(cmd, "GET") == 0) {
    cmd_get(ks, arg1);
  } else if (strcasecmp(cmd, "DEL") == 0) {
    cmd_del(ks, arg1);
  } else {
    printf("(error) ERR unknown command '%s'\n", cmd);
    return false;
  }

  return true;
}
