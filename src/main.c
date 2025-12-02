#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char command[1024];

int main(int argc, char *argv[])
{
  // Flush after every printf
  setbuf(stdout, NULL);

  printf("$ ");

  fgets(command, sizeof(command), stdin);

  // Remove the trailing newline
  command[strcspn(command, "\n")] = '\0';

  printf("%s: command not found\n", command);
  return 0;
}
