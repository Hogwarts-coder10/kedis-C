#include "kedis-c/command.h"
#include "kedis-c/keyspace.h"
#include <stdio.h>
#include <string.h>
#include <strings.h> // for strcasecmp function

/*
 * Kedis-C — entry point
 *
 * Phase 1 REPL: reads command lines from stdin, dispatches through
 * the command engine into the keyspace/object model. Networking
 * (real client connections over sockets) is a later, separate piece
 * of work per the README's roadmap — this is purely for exercising
 * the storage layer end to end while it's being built.
 */

int main(void) {
  Keyspace *ks = kedis_keyspace_create(16);
  if (!ks) {
    fprintf(stderr, "kedis-c: failed to create keyspace\n");
    return 1;
  }

  printf("Kedis-C (Phase 1 REPL) — commands: SET key value | GET key | DEL key "
         "| QUIT\n");

  char line[512];
  while (1) {
    printf("kedis-c> ");
    fflush(stdout);

    if (!fgets(line, sizeof(line), stdin))
      break; /* EOF (Ctrl+D) */

    /* strip trailing newline for the QUIT comparison below */
    line[strcspn(line, "\r\n")] = '\0';
    if (strcasecmp(line, "QUIT") == 0)
      break;
    if (line[0] == '\0')
      continue; /* blank line */

    kedis_execute_command(ks, line);
  }

  kedis_keyspace_free(ks);
  return 0;
}
