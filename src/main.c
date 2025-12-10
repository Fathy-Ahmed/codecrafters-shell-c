#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
// #include "colors.h" // find it in "D:\" and use // #define PS1 BHGRN "$ " CRESET
// #include "linenoise.h"

#define SZ(arr) (sizeof(arr) / sizeof(arr[0]))

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

#ifdef _WIN32
const char *PATH_SEP = ";"; // Windows
#else
const char *PATH_SEP = ":"; // Linux, macOS, Unix
#endif

// typedef enum
// {
//   CMD_EXIT,
//   CMD_ECHO,
//   CMD_TYPE
// } ShellCommand;

#define EXIT_CMD "exit"
#define ECHO_CMD "echo"
#define TYPE_CMD "type"
#define TYPE_PWD "pwd"
#define PATH_EVN "PATH"

#define MAX_COMMAND_LEN 1024
#define MAX_PATH_LEN 4096

const char *builtin[] = {EXIT_CMD, ECHO_CMD, TYPE_CMD, TYPE_PWD};

char *find_executable(char *command)
{
  char *env_path = getenv(PATH_EVN);
  if (env_path == NULL)
    return NULL;

  char path_copy[MAX_PATH_LEN];
  // strcpy(path_copy, env_path);
  snprintf(path_copy, sizeof(path_copy), "%s", env_path); // this is more secure

  char *token = strtok(path_copy, PATH_SEP);

  while (token)
  {
    char full_path[MAX_PATH_LEN];

    int written = snprintf(full_path, sizeof(full_path), "%s/%s", token, command); // builds a formatted string safely into a buffer.
    if (written < 0 || written >= (int)sizeof(full_path))
    {
      // Path was too long → ignore and continue safely
      continue;
    }

    // printf("full_path: %s\n", full_path); // Debugging output

    if (access(full_path, X_OK) == 0) // Check execute permissions for a file
    {
      return strdup(full_path); // return a copy Make a heap-allocated copy, Allocates a new block of memory using malloc()
    }

    token = strtok(NULL, PATH_SEP); // get the next path
  }

  return NULL;
}

bool is_builtin_command(char *command)
{
  bool ret = false;
  for (size_t i = 0; i < SZ(builtin); i++)
  {
    if (strcmp(command, builtin[i]) == 0)
    {
      ret = true;
      break;
    }
  }
  return ret;
}

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

  bool is_builtin = is_builtin_command(rest);

  if (is_builtin)
  {
    printf("%s is a shell builtin\n", rest);
  }
  else
  {
    char *exe_path = find_executable(rest);
    if (exe_path != NULL)
    {
      printf("%s is %s\n", rest, exe_path);
      free(exe_path);
    }
    else
    {
      printf("%s: not found\n", rest);
    }
  }
}

void handle_pwd()
{
  char cwd[4096];
  if (getcwd(cwd, sizeof(cwd)) != NULL) // retrieves the absolute path of the current working directory.
  {
    printf("%s\n", cwd);
  }
  else
  {
    perror("getcwd");
  }
}

void handle_builtin_command(char *cmd, char *arguments)
{

  // if (strcmp(strtok_r(command, " "), "echo") == 0) // Use strtok_r in multithreaded code.
  if (strcmp(cmd, ECHO_CMD) == 0) // echo
  {
    handle_echo(arguments);
  }
  else if (strcmp(cmd, TYPE_CMD) == 0) // type
  {
    handle_type(arguments);
  }
  else if (strcmp(cmd, TYPE_PWD) == 0) // pwd
  {
    handle_pwd();
  }
  else // exit
  {
    exit(0);
  }
}

void run_external(char *cmd, char *arguments)
{
  pid_t pid = fork(); // creates a child process that is an exact copy of the parent.
  if (pid < 0)        // error (creation failed)
  {
    perror("fork");
    return;
  }

  if (pid == 0) // Child process
  {
    // Build argv[] array for exec
    char *args[64]; // max arguments
    int i = 0;

    // First argument must be the executable itself
    args[i++] = (char *)cmd;

    if (arguments != NULL)
    {
      char *arg = strtok(arguments, " ");
      while (arg != NULL && i < 63)
      {
        args[i++] = arg;
        arg = strtok(NULL, " ");
      }
    }

    args[i] = NULL; // exec requires a NULL-terminated array

    execvp(cmd, args);

    // // If execvp returns, it FAILED
    // perror("execvp");
    // exit(1);
  }
  else // PARENT PROCESS → wait for child to finish
  {
    // The parent (your shell):
    // waits for the child to finish
    // does NOT run the external program itself
    // only resumes printing $ after the program ends
    waitpid(pid, NULL, 0);
  }
}

int main(int argc, char *argv[])
{
  // Flush after every printf
  setbuf(stdout, NULL);

  (void)argc;
  (void)argv;

  char command[MAX_COMMAND_LEN];

  while (true) // REPL (Read-Eval-Print Loop)
  {
    printf(PS1);

    fgets(command, sizeof(command), stdin);

    if (command[0] == '\n') // empty command
    {
      continue;
    }

    // Remove the trailing newline
    command[strcspn(command, "\n")] = '\0';

    char cmd_copy[MAX_COMMAND_LEN];
    strcpy(cmd_copy, command);

    char *cmd = strtok(cmd_copy, " ");
    char *arguments = strtok(NULL, ""); // Continue from where you stopped last time.

    if (is_builtin_command(cmd))
    {
      handle_builtin_command(cmd, arguments);
    }
    else
    {
      char *exe_path = find_executable(cmd);

      // Running External Programs
      if (exe_path != NULL)
      {
        // system(command); // Insecure way to run external commands, Runs through /bin/sh, not the program directly
        run_external(cmd, arguments);
        free(exe_path);
      }
      else
      {
        printf("%s: command not found\n", command);
      }
    }
  }

  return 0;
}