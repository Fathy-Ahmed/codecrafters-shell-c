/***************************************************************************/ /**

    @file         main.c

    @author       Fathy Ahmed

    @date         Thursday,  8 January 2015

    @brief        Shelly ()

  *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
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
#define TYPE_CD "cd"

const char *builtin[] = {EXIT_CMD, ECHO_CMD, TYPE_CMD, TYPE_PWD, TYPE_CD};

#define PATH_EVN "PATH"

#define MAX_COMMAND_LEN 1024
#define MAX_PATH_LEN 4096
#define MAX_ARGS 128

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

void handle_echo(char **args)
{
  for (int i = 1; args[i] != NULL; i++)
  {
    printf("%s ", args[i]);
  }
  printf("\n");
}

void handle_type(char **args)
{
  for (int i = 1; args[i] != NULL; i++)
  {
    bool is_builtin = is_builtin_command(args[i]);

    if (is_builtin)
    {
      printf("%s is a shell builtin\n", args[i]);
    }
    else
    {
      char *exe_path = find_executable(args[i]);
      if (exe_path != NULL)
      {
        printf("%s is %s\n", args[i], exe_path);
        free(exe_path);
      }
      else
      {
        printf("%s: not found\n", args[i]);
      }
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

void handle_cd(char **args)
{
  char *path = args[1];

  if (path == NULL)
  {
    // Default: go to HOME directory
    char *home = getenv("HOME");
    if (home == NULL)
      home = "/";
    if (chdir(home) != 0)
      perror("cd");
    return;
  }

  if (strcmp(path, "~") == 0)
  {
    chdir(getenv("HOME"));
  }
  else if (chdir(path) != 0)
  {
    // errno is a global variable used in C to indicate what error happened when a system call or library function fails.
    fprintf(stderr, "cd: %s: %s\n", path, strerror(errno)); // errno -> here = ENOENT (No such file or directory)
  }
}

void handle_builtin_command(char **args)
{
  // if (strcmp(strtok_r(command, " "), "echo") == 0) // Use strtok_r in multithreaded code.
  if (strcmp(args[0], ECHO_CMD) == 0) // echo
  {
    handle_echo(args);
  }
  else if (strcmp(args[0], TYPE_CMD) == 0) // type
  {
    handle_type(args);
  }
  else if (strcmp(args[0], TYPE_PWD) == 0) // pwd
  {
    handle_pwd();
  }
  else if (strcmp(args[0], TYPE_CD) == 0) // cd
  {
    handle_cd(args);
  }
  else // exit
  {
    exit(0);
  }
}

void run_external(char **args)
{
  pid_t pid = fork(); // creates a child process that is an exact copy of the parent.
  if (pid < 0)        // error (creation failed)
  {
    perror("fork");
    return;
  }

  if (pid == 0) // Child process
  {

    // CHILD
    execvp(args[0], args);

    // Only reached if execvp fails
    perror("execvp");
    exit(1);
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

char **split_args(char *line)
{
  char **args = malloc(MAX_ARGS * sizeof(char *)); // max 64 arguments

  bool is_single_quote = false;
  bool is_double_quote = false;

  int arg_index = 0;
  char *p = line;
  char *start = NULL;
  while (*p)
  {
    // Skip spaces outside quotes
    while (*p == ' ' && !is_single_quote && !is_double_quote)
      p++;

    if (*p == '\0')
      break;

    start = p;

    while (*p)
    {
      if (*p == '\'' && !is_double_quote)
      {
        is_single_quote = !is_single_quote;
        memmove(p, p + 1, strlen(p)); // remove quote
        continue;
      }

      if (*p == '"' && !is_single_quote)
      {
        is_double_quote = !is_double_quote;
        memmove(p, p + 1, strlen(p)); // remove quote
        continue;
      }

      if (*p == ' ' && !is_single_quote && !is_double_quote)
        break;

      p++;
    }
    if (*p)
    {
      *p = '\0';
      p++;
    }

    args[arg_index++] = start;

    if (arg_index >= 63)
      break;
  }

  args[arg_index] = NULL;
  return args;
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

    char **args = split_args(command);

    if (args[0] == NULL)
    {
      free(args);
      continue;
    }

    if (is_builtin_command(args[0]))
    {
      handle_builtin_command(args);
    }
    else
    {
      char *exe_path = find_executable(args[0]);

      // Running External Programs
      if (exe_path != NULL)
      {
        // system(command); // Insecure way to run external commands, Runs through /bin/sh, not the program directly
        run_external(args);
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