#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_ARG_LEN 80
#define MAX_NUM_ARGS 3

int alloc(char *module) {
  int cmp_gpio = strcmp(module, "gpio");
  if (cmp_gpio == 0) {
    printf("allocing for gpio...\n");
  }

  return 0;
}

int op_handle(int argc, char **argv) {
  int cmp_alloc = strcmp(argv[0], "alloc");

  if (cmp_alloc == 0) {
    return alloc(argv[1]);
  }

  return 0;
}

int pm_parse(char *cmd, pid_t pid) {
  printf("command gotten: %s\n", cmd);
  char *token = strtok(cmd, " ");
  token = strtok(NULL, " ");
  char *args[MAX_NUM_ARGS];
  int argc = -1;
  for (int i = 0; i < MAX_NUM_ARGS && token != NULL; i++) {
    args[i] = strtok(NULL, " ");
    argc++;
  }
  return op_handle(argc, args);
}
