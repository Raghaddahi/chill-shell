#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

void print_error() {
  char error_message[30] = "An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message));
}

int main(int argc, char *argv[]) {

  printf("                           \n");
  printf(" ▄▄▄▄ ▄▄ ▄▄ ▄▄ ▄▄    ▄▄    \n");
  printf("██▀▀▀ ██▄██ ██ ██    ██    \n");
  printf("▀████ ██ ██ ██ ██▄▄▄ ██▄▄▄ \n");
  printf("                           \n");
  printf("                           \n");

  FILE *input;
  int interactive;
  char *line = NULL;
  size_t len = 0;
  ssize_t nread;

  char *paths[100];
  int path_count;

  paths[0] = "/bin";
  path_count = 1;

  pid_t pids[100];
  int pid_count = 0;

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

  // random emoji

  const char *emojis[] = {"🌊", "❄️", "😌", "🌿"};
  srand(time(NULL));
  int emoji_count = sizeof(emojis) / sizeof(emojis[0]);

  int index = rand() % emoji_count;

  while (1) {
    if (interactive) {
      printf("%s chill> ", emojis[index]);
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
    char *saveptr1, *saveptr2;

    pid_count = 0;

    // spilt by &
    char *cmd_str = strtok_r(line, "&", &saveptr1);
    while (cmd_str != NULL) {

      char *args[100];
      int i = 0;
      char *token = strtok_r(cmd_str, " \t", &saveptr2);
      while (token != NULL) {
        args[i++] = token;
        token = strtok_r(NULL, " \t", &saveptr2);
      }
      args[i] = NULL;

      // cd
      if (args[0] != NULL && strcmp(args[0], "cd") == 0) {

        int args_count = 0;

        for (int j = 1; args[j] != NULL; j++)
          args_count++;

        if (args_count > 1) {
          print_error();
          continue; // skip to nextt command
        }

        if (args[1] == NULL || strcmp(args[1], "~") == 0) {
          args[1] = getenv("HOME");
        }

        if (chdir(args[1]) != 0) {
          print_error();
        }
        cmd_str = strtok_r(NULL, "&", &saveptr1);
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

        cmd_str = strtok_r(NULL, "&", &saveptr1);
        continue;
      }

      if (path_count == 0) {
        print_error();
        continue;
      }

      // redirection
      char *redirect_file = NULL;

      for (int j = 1; args[j] != NULL; j++) {
        if (strcmp(args[j], ">") == 0) {

          if (args[j + 1] == NULL || args[j + 2] != NULL) {
            print_error();
            redirect_file = NULL;
          } else {
            args[j] = NULL;
            redirect_file = args[j + 1];
          }
          break;
        }
      }

      pid_t pid = fork();

      if (pid < 0) {
        print_error();
      }

      if (pid == 0) {

        if (redirect_file != NULL) {
          int fd = open(redirect_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

          if (fd == -1) {
            print_error();
          }

          dup2(fd, STDOUT_FILENO);
          dup2(fd, STDERR_FILENO);
          close(fd);
        }

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
      } else {
        pids[pid_count++] = pid;
      }
      cmd_str = strtok_r(NULL, "&", &saveptr1);
    }

    for (int i = 0; i < pid_count; i++) {
      waitpid(pids[i], NULL, 0);
    }
  }
  if (argc == 2) {
    fclose(input);
  }
  return 0;
}