#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void print_error() {
  char error_message[30] = "An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message));
}

int main(int argc, char *argv[]) {

  FILE *input;
  int interactive;
  char *line = NULL;
  size_t len = 0;
  ssize_t nread;

  char *paths[100];
  int path_count;

  paths[0] = "/bin";
  path_count = 1;

  if (argc == 1) {
    interactive = 1;
  } else if (argc == 2) {
    interactive = 0; // batch mode
  } else {
    print_error();
    exit(1);
  }

  input = stdin;
  if (argc == 2) {
    input = fopen(argv[1], "r");
    if (input == NULL) {
      print_error();
      exit(1);
    }
  }

  while (1) {
    if (interactive) {
      printf("chill> ");
      fflush(stdout);
    }

    nread = getline(&line, &len, input);

    if (nread == -1) {
      exit(0);
    }

    line[strcspn(line, "\n")] = '\0'; // delete white space

    if (strcmp(line, "exit") == 0) { // compare
      exit(0);
    }

    char *args[100];
    int i = 0;
    char *rest = line;
    char *token;
    while ((token = strsep(&rest, " \t")) != NULL) {
      if (*token == '\0')
        continue;
      args[i++] = token;
    }
    args[i] = NULL;

    // cd
    if (args[0] != NULL && strcmp(args[0], "cd") == 0) {

      int count = 0;

      for (int j = 1; args[j] != NULL; j++)
        count++;

      if (count != 1) {
        print_error();
        continue; // skip to nextt command
      }

      if (chdir(args[1]) != 0) {
        print_error();
      }

      continue;
    }

    // path

    if (args[0] != NULL && strcmp(args[0], "path") == 0) {

      if (args[1] == NULL) {
        path_count = 0;
        paths[0] = NULL;
        continue;
      }

      path_count = 0;
      for (int j = 1; args[j] != NULL; j++) {
        paths[path_count++] = strdup(args[j]);
      }

      // path testing
      for (int i = 0; i < path_count; i++)
        printf("%s\n", paths[i]);

      continue;
    }

    if (path_count == 0) {
      print_error();
      continue;
    }

    pid_t pid = fork();

    if (pid < 0) {
      print_error();
    }

    if (pid == 0) {
      // search path and exec
      char full_path[1024];

      for (int j = 0; j < path_count; j++) {
        snprintf(full_path, sizeof(full_path), "%s/%s", paths[j], args[0]);

        if (access(full_path, X_OK) == 0) {
          execv(full_path, args);

          print_error();
          exit(1);
        }
      }

      print_error();
      exit(1);
    }

    if (pid > 0) {
      wait(NULL);
    }
  }
  return 0;
}
