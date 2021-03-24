#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void parse(char* s) {
  const char break_chars[] = " \t";
  char* p;
  p = strtok(s, break_chars);
  while (p != NULL) {
    printf("token was: %s\n", p);
    p = strtok(NULL, break_chars);
  }
}

// test
int main(int argc, const char * argv[]) {  
  char input[BUFSIZ];
  char last_command[BUFSIZ];
  
  memset(input, 0, BUFSIZ * sizeof(char));
  memset(input, 0, BUFSIZ * sizeof(char));
  bool finished = false;
  
  while (!finished) {
    printf("osh> ");
    fflush(stdout);

    if ((fgets(input, BUFSIZ, stdin)) == NULL) {   // or gets(input, BUFSIZ);
      fprintf(stderr, "no command entered\n");
      exit(1);
    }
    input[strlen(input) - 1] = '\0';          // wipe out newline at end of string
    printf("input was: \n'%s'\n", input);

    // check for history (!!) command
    if (strncmp(input, "!!", 2) == 0) {
      if (strlen(last_command) == 0) {
        fprintf(stderr, "no last command to execute\n");
      }
      printf("last command was: %s\n", last_command);
    } else if (strncmp(input, "exit", 4) == 0) {   // only compare first 4 letters
      finished = true;
    } else {
      strcpy(last_command, input);
      printf("You entered: %s\n", input);   // you will call fork/exec
      parse(input);
			printf("after parse, what is input: %s\n", input);
      printf("\n");
    }
  }
  
  printf("osh exited\n");
  printf("program finished\n");
  
  return 0;
}