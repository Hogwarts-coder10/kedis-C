#ifndef KEDIS_C_COMMAND_H
#define KEDIS_C_COMMAND_H

#include "kedis-c/keyspace.h"
#include <stdbool.h>

/*
 * Kedis-C — basic command engine (Phase 1)
 *
 * Minimal line-based dispatch: SET <key> <value>, GET <key>, DEL <key>.
 * No quoting/escaping support yet — tokens split on plain whitespace.
 * This is intentionally the simplest thing that exercises the object
 * model + keyspace end to end; the real KESP protocol parser is a
 * separate, later piece of work (see README's networking/protocol
 * sections).
 *
 * Writes its reply directly to stdout. Returns false only on a
 * malformed/unknown command line (not on e.g. GET of a missing key —
 * that's a normal "(nil)" reply, not a failure).
 */
bool kedis_execute_command(Keyspace *ks, const char *line);

#endif /* KEDIS_C_COMMAND_H */
