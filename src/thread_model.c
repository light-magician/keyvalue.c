

#include "thread_model.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/*
 * Worker loop waits for a new task
 * when a new task arrives it
 *      pops from the queue
 *      unlocks the queue
 *      processes the work
 *      prints a log
 *      frees the task
 *      exits if shutdown is set
 * arg: a generic but expects a thread_pool
 */
static void *worker_loop(void *arg) {
  thread_pool *pool = (thread_pool *)arg;

  while (1) {
    // enable the queue mutex
    pthread_mutex_lock(&pool->queue_mutex);
    // threads wait when there is no work and not shutdown
    while (pool->queue_size == 0 && !pool->shutdown) {
      pthread_cond_wait(&pool->queue_cond, &pool->queue_mutex);
    }
    // if shutdown flag, return thread pool to waiting state
    if (pool->shutdown) {
      pthread_mutex_unlock(&pool->queue_mutex);
      break;
    }
    // pop from the front of the queue
    connection_task *task = pool->task_queue[pool->queue_head];
    pool->queue_head = (pool->queue_head + 1) % TASK_QUEUE_CAPACITY;
    pool->queue_size--;
    pthread_mutex_unlock(&pool->queue_mutex);
    // TODO: process the task (temporary debug log)
    // pthread_self() returns the calling thread's ID
    printf("[Thread %ld] Processing task: type=%d key=%s\n", pthread_self(),
           task->type, task->key);

    // task handling
    // TODO: call actual task handlers here
    free(task->value);
    free(task);
  }
  return NULL;
}

/*
 * Initializes a tread_pool struct
 */
int thread_pool_init(thread_pool *pool, int num_threads) {
  pool->thread_count = num_threads;
  pool->queue_head = pool->queue_tail = pool->queue_size = 0;
  pool->shutdown = 0;

  pthread_mutex_init(&pool->queue_mutex, NULL);
  pthread_cond_init(&pool->queue_cond, NULL);

  for (int i = 0; i < num_threads; i++) {
    if (pthread_create(&pool->threads[i], NULL, worker_loop, pool) != 0) {
      return -1;
    }
  }
  return 0;
}

/*
 * Enqueues a task on the thread pool.
 * Accesses the thread_pool in a thread safe way.
 */
void thread_pool_enqueue(thread_pool *pool, connection_task *task) {
  // lock the queue mutex
  pthread_mutex_lock(&pool->queue_mutex);
  // drops the task if the queue of tasks is full
  // unlock the queue mutex in this case
  if (pool->queue_size == TASK_QUEUE_CAPACITY) {
    fprintf(stderr, "task queue full. Task is dropped.\n");
    free(task);
    pthread_mutex_unlock(&pool->queue_mutex);
    return;
  }
  // add task to task_queue tail
  pool->task_queue[pool->queue_tail] = task;
  pool->queue_tail = (pool->queue_tail + 1) % TASK_QUEUE_CAPACITY;
  pool->queue_size++;
  // signal threads there is a new task
  pthread_cond_signal(&pool->queue_cond);
  // unlock the queue mutex
  pthread_mutex_unlock(&pool->queue_mutex);
}

/*
 * Destroys the thread_pool.
 *
 * Locks the queue_mutex for thread safety.
 * Marks shutdown flag.
 * Braodcasts to threads the altered condition.
 * Unlocks queue_mutex.
 * Joins threads.
 * Destroys queue_mutex.
 * Destroys queue_cond.
 */
void thread_pool_destroy(thread_pool *pool) {
  pthread_mutex_lock(&pool->queue_mutex);
  pool->shutdown = 1;
  pthread_cond_broadcast(&pool->queue_cond);
  pthread_mutex_unlock(&pool->queue_mutex);

  for (int i = 0; i < pool->thread_count; i++) {
    pthread_join(pool->threads[i], NULL);
  }

  pthread_mutex_destroy(&pool->queue_mutex);
  pthread_cond_destroy(&pool->queue_cond);
}
