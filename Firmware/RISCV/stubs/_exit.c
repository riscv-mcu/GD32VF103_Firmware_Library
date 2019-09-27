/* See LICENSE of license details. */

#include <unistd.h>

#include "stub.h"

#if 0
void _exit(int code)
{
  const char message[] = "\nProgram has exited with code:";

  write(STDERR_FILENO, message, sizeof(message) - 1);
  write_hex(STDERR_FILENO, code);
  write(STDERR_FILENO, "\n", 1);

  for (;;);
}
#endif
