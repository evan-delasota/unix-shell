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
            //ioRedir(args, select, ioSymbol);
          } else if (ioSymbol == 2) {
            //pipeRedir(args, select);
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