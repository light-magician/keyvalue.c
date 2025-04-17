
// test_thread_model.c
#include "thread_model.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/*
 * Command for compiling and linking thread_model_test
 * gcc thread_model_test.c thread_model.c -o thread_model_test.o -lpthread
 *
 * Command for running
 * ./thread_model_test
 */

int main() {
  thread_pool pool;
  thread_pool_init(&pool, 4);

  for (int i = 0; i < 10; i++) {
    connection_task *task = malloc(sizeof(connection_task));
    task->type = TASK_PUT;
    snprintf(task->key, sizeof(task->key), "file_%d.txt", i);
    task->value = strdup("example data");
    task->value_len = strlen(task->value);
    thread_pool_enqueue(&pool, task);
  }

  sleep(2); // let threads work
  thread_pool_destroy(&pool);
  return 0;
}
