#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>


#define DELIM "\t\r\n\a"
#define BUFSZ 64

char** tokenize(char* s) {
  int i = 0;
  int buffer = BUFSZ;
  char** tokens = malloc(buffer * sizeof(char*));
  char* token;

  token = strtok(s, DELIM);
  while (token != NULL) {
    tokens[i] = token;
    i++;

    if (i >= buffer) {
      buffer += BUFSZ;
      tokens = realloc(tokens, buffer * sizeof(char*));
    }

  token = strtok(NULL, DELIM);
  }

  tokens[i] = NULL;
  return tokens;
}

void ioRedir(char** args, int n, int ioType) {
  pid_t pid;
  pid_t wPid;
  mode_t modes = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
  int direct = 0;
  int condition = 0;

  pid = fork();
  if (pid == 0) {
    if (ioType == 0) {
      direct = open(args[n + 1], O_RDONLY, modes);
    } else {
      direct = open(args[n + 1], O_WRONLY | O_CREAT | O_TRUNC, modes);
    }

    if (direct < 0) {
      perror("invalid file\n");
      exit(1);
    } else {
      dup2(direct, ioType);
      close(direct);
      args[n] = NULL;
      args[n + 1] = NULL;

      if (execvp(args[0], args) < 0) {
        perror("shell error");
      }
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("Failed to create child process.\n");
    exit(EXIT_FAILURE);
  } else {
    do {
      wPid = waitpid(pid, &condition, WUNTRACED);
    } while (!WIFEXITED(condition) && !WIFSIGNALED(condition));
  }
}

void pipeRedir(char** args, int n) {
  int pipefd[2];
  char** argscopy = malloc(sizeof(char*) * (n + 1));
  int i = 0;

  for (i = 0; i < args; ++i) {
    argscopy[i] = strdup(args[i]);
  }
  argscopy[i] = 0;

  if (pipe(pipefd) < 0) {
    perror("redirection failure\n");
    return;
  }

  if (fork() == 0) {
    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[0]);
    close(pipefd[1]);
    execvp(argscopy[0], argscopy);

    perror("execution failure\n\n");
    exit(1);
  }

  if (fork() == 0) {
    dup2(pipefd[0], STDIN_FILENO);
    close(pipefd[1]);
    close(pipefd[0]);
    execvp(args[n + 1], args + n + 1);

    perror("execution failure\n\n");
    exit(1);
  }

  close(pipefd[0]);
  close(pipefd[1]);
  wait(0);
  wait(0);
}

int main(int argc, const char * argv[]) {  
  char* input = NULL;
  char* last_command;
  char** args;
  char* ioType[4] = {"<", ">", "|", "&"};
  int ioSymbol = 0;
  int select;
  bool isCommandOrFgProc;

  bool finished = false;
  
  while (!finished) {
    isCommandOrFgProc = false;
    ssize_t sz = 0;

    printf("osh> ");
    fflush(stdout);
    getline(&input, &sz, stdin);
    args = tokenize(input);

    if (strncmp(args[0], "!!", 2) != 0) {
      last_command = strdup(args[0]);
    }

    if (args[0] == NULL) {
      free(input);
      free(args);
      continue;
    }
    printf("input was: \n'%s'\n", input);

    if (strcmp(args[0], "exit") == 0) {
      finished = true;
    }
    // check for history (!!) command
    if (strncmp(args[0], "!!", 2) == 0) {
      if (strlen(last_command) == 0) {
        fprintf(stderr, "no last command to execute\n");
      } else {
        printf("last command was: %s\n", last_command);
      }
    } else if (strncmp(args[0], "exit", 4) == 0) {   // only compare first 4 letters
      finished = true;
    } else {
      select = 1;
      while (args[select] != NULL) {
        for (ioSymbol = 0; ioSymbol < 4; ioSymbol++) {
          if (strcmp(args[select], ioType[ioSymbol]) == 0) {
            break;
          }
        }
        
        if (ioSymbol < 4) {
          isCommandOrFgProc = true;

          if (ioSymbol < 2) {
            ioRedir(args, select, ioSymbol);
          } else if (ioSymbol == 2) {
            pipeRedir(args, select);
          } else if (ioSymbol == 3) {
            //initProc(args, select);
          }
          break;
        }
        select++;

      }
      if (!isCommandOrFgProc) {
        initProc(args, 0);
      }

      free(args);
      free(input);
    }
  }
  
  printf("osh exited\n");
  printf("program finished\n");
  return 0;
}