#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
// #include "colors.h" // find it in "D:\" and use // #define PS1 BHGRN "$ " CRESET
// #include "linenoise.h"

#define sz(obj) (sizeof(obj) / sizeof(obj[0]))

/*
#define USE_READLINE 0
#define USE_LINENOISE 1
#if USE_READLINE == 1
// # For Ubuntu/Debian
// sudo apt-get install libreadline-dev
// #include <readline/readline.h>
// #include <readline/history.h>
#endif*/

// color
#define GRN "\033[0;32m"
#define COLOR_RESET "\033[0m"

#define PS1 GRN "$ " COLOR_RESET
const char *builtin[] = {"exit", "echo", "type"};

char *exit_command = "exit";
char *echo_command = "echo";
char *type_command = "type";

void handle_echo(char *rest)
{
  if (rest != NULL)
    printf("%s\n", rest);
  else
    printf("\n");
}
void handle_type(char *rest)
{
  if (rest == NULL)
  {
    printf("\n");
    return;
  }

  bool is_builtin = false;

  for (int i = 0; i < sz(builtin); i++)
  {
    if (strcmp(rest, builtin[i]) == 0)
    {
      is_builtin = true;
      break;
    }
  }

  if (is_builtin)
    printf("%s is a shell builtin\n", rest);
  else
    printf("%s: not found\n", rest);
}

int main(int argc, char *argv[])
{
  // Flush after every printf
  setbuf(stdout, NULL);

  char command[1024];
  bool running = 1;

  while (running) // REPL (Read-Eval-Print Loop)
  {
    printf(PS1);

    fgets(command, sizeof(command), stdin);

    if (command[0] == '\n')
      continue;

    // Remove the trailing newline
    command[strcspn(command, "\n")] = '\0';

    if (strcmp(command, exit_command) == 0) // exit
    {
      running = false;
      continue;
    }

    // Save original command before tokenizing (strtok modifies the buffer)
    char original_cmd[1024];
    strcpy(original_cmd, command);

    char *cmd = strtok(command, " ");
    char *rest = strtok(NULL, ""); // Continue from where you stopped last time.

    // if (strcmp(strtok_r(command, " "), "echo") == 0) // Use strtok_r in multithreaded code.
    if (cmd != NULL && strcmp(cmd, echo_command) == 0) // echo
    {
      handle_echo(rest);
    }
    else if (cmd != NULL && strcmp(cmd, type_command) == 0) // type
    {
      handle_type(rest);
    }
    else
    {
      printf("%s: command not found\n", original_cmd);
    }
  }

  return 0;
}
