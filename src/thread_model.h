
#ifndef THREAD_MODEL_H

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
  connection_task *task_queue;
  int queue_size;
  int queue_head;
  int queue_tail;
  int shutdown;
} thread_pool;

#endif // !THREAD_MODEL_H
