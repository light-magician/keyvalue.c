
#ifndef THREAD_MODEL_H
#define THREAD_MODEL_H

#include <pthread.h>
#include <stdlib.h>

#define MAX_WORKERS 8
#define TASK_QUEUE_CAPACITY 128

/*
 * Enum for defining spectrum of task types
 */
typedef enum {
  TASK_GET,
  TASK_PUT,
  TASK_DELETE,
} TaskType;
/*
 * A connection_task contains all data related to
 *       an executing task
 * client_fd: reference to client's file descriptor
 * key: a reference to the key to the stored object
 * value: a reference to the value of a stored object
 * value_len: data length of stored value
 */
typedef struct {
  TaskType type;
  int client_fd;
  char key[256];
  char *value;
  size_t value_len;
} connection_task;

/*
 * A struct for managing a queue of requests
 *
 * threads: an array of thread ids
 * thread_count: number of threads in the pool
 * pthread_mutex_t: mutext to protect the work queue_mutex
 *                  prevents race conditions when adding
 *                  or removing tasks from the queue
 * queue_cond: a signal variable to alert threads when
 *             there is new work, when thread pool is
 *             shutting down, and allows threads to wait
 *             efficiently without "busy-waiting"
 * work_queue: typically a linked list or circular buffer
 *             data structure, threads pull tasks from this
 *             queue.
 * shutdown: flag alerting threads not to take on new tasks,
 *           and alerting threads to stop their main task loop
 * pending_tasks: tracks the number of tasks waiting in the queue
 *                for monitoring load and applying backpressure
 */
typedef struct {
  pthread_t threads[MAX_WORKERS];
  int thread_count;
  pthread_mutex_t queue_mutex;
  pthread_cond_t queue_cond;
  connection_task *task_queue[TASK_QUEUE_CAPACITY];
  int queue_size;
  int queue_head;
  int queue_tail;
  int shutdown;
} thread_pool;

/*
 * Thread pool functions
 */
int thread_pool_init(thread_pool *pool, int num_threads);
void thread_pool_enqueue(thread_pool *pool, connection_task *task);
void thread_pool_destroy(thread_pool *pool);

#endif // !THREAD_MODEL_H
