#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char command[1024];

int main(int argc, char *argv[])
{
  // Flush after every printf
  setbuf(stdout, NULL);

  while (1)
  {
    printf("$ ");
    fgets(command, sizeof(command), stdin);

    // Remove the trailing newline
    command[strcspn(command, "\n")] = '\0';

    if (strcmp(command, "exit") == 0) // exit
      return 0;

    // else if (strcmp(strtok_r(command, " "), "echo") == 0) // Use strtok_r in multithreaded code.
    else if (strcmp(strtok(command, " "), "echo") == 0) // echo
    {
      printf("%s\n", strtok(NULL, "")); // Continue from where you stopped last time.
    }

    else
      printf("%s: command not found\n", command);
  }

  return 0;
}
