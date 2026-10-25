// Cooperative (single stack per task, no preemption) thread scheduler.
//
// Every "thread" is a coroutine with its own heap allocated stack. The main
// program calls thread_run(), which runs the tasks round-robin: a task runs
// until it calls thread_yield() (directly, or indirectly through
// thread_server_read_timeout, sys_usleep, the socket waits in sys.c, ...),
// then the next runnable task is resumed. thread_run() returns when no task
// is runnable anymore (all finished, or all waiting on a mutex).
//
// Used on platforms without a thread library (MS-DOS/DJGPP). The task switch
// is done with setjmp/longjmp; a new task starts with the stack pointer set
// to the top of its own stack (x86 only).

#include "sys.h"
#include "thread.h"
#include "debug.h"

#define MAX_KEYS     256
#define MAX_THREADS  256
#define MAX_MESSAGES 256
#define STACK_SIZE   (1024 * 1024)

struct thread_key_t {
  int id, inuse;
};

typedef struct {
  int client;
  uint32_t len;
  void *buf;
} msg_t;

typedef enum {
  TASK_CREATED,
  TASK_RUNNING,
  TASK_WAITING
} task_status_e;

enum {
  INIT,
  SCHEDULE,
  EXIT_TASK,
};

typedef struct {
  int inuse;
  task_status_e status;
  int id;
  char *name;
  int (*action)(void *arg);
  void *arg;
  void *values[MAX_KEYS];
  msg_t messages[MAX_MESSAGES];
  uint32_t nmsg, imsg, omsg;
  void *stack_bottom;
  void *stack_top;
  uint32_t stack_size;
  jmp_buf jbuf;
} thread_arg_t;

static thread_arg_t tasks[MAX_THREADS];
static void *main_values[MAX_KEYS];  // thread local values of the main program (no task running)
static thread_key_t *local;
static thread_key_t *tname;
static int flags, status;

static int current;   // index of the running task, -1 when the main program runs
static int running;   // inside thread_run()
static jmp_buf jbuf;  // scheduler context (main stack)

static uint32_t num_threads;
static thread_ps_t ps[MAX_THREADS];

static uint32_t num_keys;
static thread_key_t keys[MAX_KEYS];

void thread_init(void) {
  sys_memset(tasks, 0, sizeof(tasks));
  sys_memset(main_values, 0, sizeof(main_values));
  sys_memset(keys, 0, sizeof(keys));
  sys_memset(ps, 0, sizeof(ps));

  current = -1;
  running = 0;

  local = thread_key();
  tname = thread_key();

  status = STATUS_SUCCESS;
  flags = 0;

  num_threads = 0;
  num_keys = 0;
}

void thread_close(void) {
  thread_key_delete(tname);
  thread_key_delete(local);
}

static thread_arg_t *choose_next_task(void) {
  int i, j, n, found = 0;

  if (num_threads > 0) {
    for (i = 0, n = 0; i < MAX_THREADS && !found && n < num_threads; i++) {
      j = (current + i + 1) % MAX_THREADS;
      if (tasks[j].inuse) {
        if (tasks[j].status != TASK_WAITING) {
          current = j;
          found = 1;
        }
        n++;
      }
    }
  }

  return found ? &tasks[current] : NULL;
}

// First activation of a task. Runs on the task's own stack, so it must not
// use anything from the caller's frame (schedule() below), only globals:
// that frame lives on the main stack and is reused by the scheduler as soon
// as this task yields for the first time. Never returns (longjmp).
static void __attribute__((noinline)) task_entry(void) {
  thread_arg_t *targ = &tasks[current];
  int id = targ->id;

  targ->status = TASK_RUNNING;
  thread_set(local, targ);
  thread_set_name(targ->name);
  debug(DEBUG_INFO, "THREAD", "thread id %d begin", id);

  targ->action(targ->arg);

  // the task may have yielded (and been resumed) many times: refetch it
  targ = &tasks[current];
  debug(DEBUG_INFO, "THREAD", "thread id %d end", id);
  thread_set(local, NULL);
  thread_set(tname, NULL);
  targ->inuse = 0;
  longjmp(jbuf, EXIT_TASK);
}

static void schedule(void) {
  thread_arg_t *next = choose_next_task();

  if (!next) return;

  if (next->status == TASK_CREATED) {
    // switch to the task's stack (16 byte aligned top) and start it
#if defined(__x86_64__)
    void *top = (void *)((uintptr_t)next->stack_top & ~(uintptr_t)15);
    asm volatile ("mov %0, %%rsp" : : "r" (top) : "memory");
#elif defined(__i386__)
    void *top = (void *)((uintptr_t)next->stack_top & ~(uintptr_t)15);
    asm volatile ("mov %0, %%esp" : : "r" (top) : "memory");
#else
#error "cooperative threads: unsupported architecture"
#endif
    task_entry();
  } else {
    longjmp(next->jbuf, 1);
  }
}

void thread_yield(int waiting) {
  thread_arg_t *targ;

  // nothing to switch to when called from the main program (before or
  // outside thread_run): there is no scheduler context to return to
  if (!running || current < 0) return;

  targ = &tasks[current];

  if (waiting) {
    targ->status = TASK_WAITING;
  }

  if (setjmp(targ->jbuf) == 0) {
    longjmp(jbuf, SCHEDULE);
  }
}

void thread_resume(int id) {
  thread_arg_t *targ;

  if (id >= 0 && id < MAX_THREADS) {
    targ = &tasks[id];
    if (targ->inuse) {
      targ->status = TASK_RUNNING;
    }
  }
}

static void free_current_task(void) {
  thread_arg_t *targ = &tasks[current];
  int i;

  sys_free(targ->stack_bottom);
  targ->stack_bottom = NULL;
  targ->stack_top = NULL;

  // messages nobody read
  for (i = 0; i < MAX_MESSAGES; i++) {
    if (targ->messages[i].buf) {
      sys_free(targ->messages[i].buf);
      targ->messages[i].buf = NULL;
    }
  }
  targ->nmsg = targ->imsg = targ->omsg = 0;

  if (num_threads > 0) num_threads--;
}

int thread_needs_run(void) {
  return 1;
}

void thread_run(void) {
  if (running) {
    // nested call (a task starting another scheduler): the tasks are
    // already being scheduled by the outer thread_run()
    return;
  }

  running = 1;
  current = -1;

  switch (setjmp(jbuf)) {
    case EXIT_TASK:
      free_current_task();
      // fall through
    case INIT:
    case SCHEDULE:
      schedule();
      break;
    default:
      break;
  }

  // no runnable task left
  current = -1;
  running = 0;
}

thread_key_t *thread_key(void) {
  thread_key_t *key = NULL;
  int i;

  for (i = 0; i < MAX_KEYS; i++) {
    if (keys[i].inuse == 0) {
      key = &keys[i];
      key->id = i+1;
      key->inuse = 1;
      num_keys++;
      break;
    }
  }

  return key;
}

void thread_key_delete(thread_key_t *key) {
  if (key && key->inuse) {
    key->id = 0;
    key->inuse = 0;
    num_keys--;
  }
}

static void **current_values(void) {
  return (running && current >= 0) ? tasks[current].values : main_values;
}

void thread_set(thread_key_t *key, void *value) {
  if (key && key->inuse) {
    current_values()[key->id - 1] = value;
  }
}

void *thread_get(thread_key_t *key) {
  void *value = NULL;

  if (key && key->inuse) {
    value = current_values()[key->id - 1];
  }

  return value;
}

void thread_setmain(void) {
}

int thread_must_end(void) {
  if (thread_get_flags(FLAG_FINISH)) {
    debug(DEBUG_INFO, "THREAD", "thread must end (flag)");
    return 1;
  }

  return 0;
}

int thread_get_handle(void) {
  thread_arg_t *targ;

  targ = (thread_arg_t *)thread_get(local);
  return targ ? targ->id : -1;
}

int thread_begin(char *tag, int action(void *arg), void *arg) {
  thread_arg_t *targ = NULL;
  int i;

  for (i = 0; i < MAX_THREADS; i++) {
    if (tasks[i].inuse == 0) {
      targ = &tasks[i];
      break;
    }
  }

  if (targ == NULL) {
    debug(DEBUG_ERROR, "THREAD", "too many threads");
    return -1;
  }

  if ((targ->stack_bottom = sys_malloc(STACK_SIZE)) == NULL) {
    debug(DEBUG_ERROR, "THREAD", "no memory for thread stack");
    return -1;
  }

  sys_memset(targ->values, 0, sizeof(targ->values));
  sys_memset(targ->messages, 0, sizeof(targ->messages));
  targ->nmsg = targ->imsg = targ->omsg = 0;

  targ->inuse = 1;
  targ->id = i;
  targ->name = tag;
  targ->action = action;
  targ->arg = arg;
  targ->status = TASK_CREATED;

  targ->stack_size = STACK_SIZE;
  targ->stack_top = (uint8_t *)targ->stack_bottom + targ->stack_size;

  num_threads++;

  return i;
}

int thread_client_write(int id, unsigned char *buf, unsigned int len) {
  thread_arg_t *targ;
  int r = -1;

  if (buf && len) {
    if (id >= 0 && id < MAX_THREADS) {
      targ = &tasks[id];

      if (targ->inuse) {
        if (targ->nmsg < MAX_MESSAGES) {
          targ->messages[targ->imsg].client = thread_get_handle();
          targ->messages[targ->imsg].len = len;
          targ->messages[targ->imsg].buf = sys_malloc(len);
          sys_memcpy(targ->messages[targ->imsg].buf, buf, len);
          targ->imsg++;
          if (targ->imsg == MAX_MESSAGES) targ->imsg = 0;
          targ->nmsg++;
          r = len;
        } else {
          debug(DEBUG_ERROR, "THREAD", "thread_client_write id %d queue overflow", id);
        }
      } else {
        debug(DEBUG_ERROR, "THREAD", "thread_client_write id %d not in use", id);
      }
    } else {
      debug(DEBUG_ERROR, "THREAD", "thread_client_write invalid id %d", id);
    }
  }

  return r;
}

int thread_server_read(unsigned char **buf, unsigned int *len) {
  return thread_server_read_timeout(0, buf, len);
}

int thread_server_read_timeout(uint32_t usec, unsigned char **buf, unsigned int *len) {
  return thread_server_read_timeout_from(usec, buf, len, NULL);
}

int thread_server_read_timeout_from(uint32_t usec, unsigned char **buf, unsigned int *len, int *client) {
  thread_arg_t *targ;
  int64_t t0, t;
  int r = 0;
  
  if (buf && len) {
    *buf = NULL;
    *len = 0;
    if (current < 0) return 0;
    targ = &tasks[current];
    t0 = sys_get_clock();

    for (;;) {
      if (targ->nmsg > 0) break;
      t = sys_get_clock();
      if ((uint64_t)(t - t0) >= usec) return 0;
      thread_yield(0);
      if (thread_must_end()) return -1;
      sys_usleep(usec < 1000 ? usec : 1000);
    }

    if (client) *client = targ->messages[targ->omsg].client;
    *len = targ->messages[targ->omsg].len;
    *buf = targ->messages[targ->omsg].buf;
    targ->messages[targ->omsg].buf = NULL;
    targ->messages[targ->omsg].len = 0;
    targ->messages[targ->omsg].client = 0;
    targ->omsg++;
    if (targ->omsg == MAX_MESSAGES) targ->omsg = 0;
    targ->nmsg--;
    r = 1;

    if (*len == 1 && (*buf)[0] == 0) {
      debug(DEBUG_INFO, "THREAD", "received finish packet");
      r = -1;
    }
  }

  return r;
}

int thread_server_peek(void) {
  if (current < 0) return 0;

  return tasks[current].nmsg > 0;
}

int thread_end(char *tag, int id) {
  uint8_t packet;
  int r = 0;

  if (id >= 0) {
    debug(DEBUG_INFO, "THREAD", "closing thread with id %d", id);
    packet = 0;
    r = thread_client_write(id, &packet, 1);
  }

  return r;
}

void thread_set_name(char *name) {
  thread_set(tname, name);
}

void thread_get_name(char *name, int len) {
  char *s;
  int i;

  s = (char *)thread_get(tname);
  if (s == NULL) {
    s = "MAIN";
  }

  for (i = 0; s[i] && i < len-1; i++) {
    name[i] = s[i];
  }
  name[i] = 0;
}

void thread_wait_all(void) {
  // the tasks run inside thread_run(), nothing to wait for here
}

void thread_set_flags(unsigned int mask) {
  flags |= mask;
}

void thread_reset_flags(unsigned int mask) {
  flags &= ~mask;
}

void thread_set_status(int _status) {
  status = _status;
}

int thread_get_status(void) {
  return status;
}

unsigned int thread_get_flags(unsigned int mask) {
  return flags & mask;
}

thread_ps_t *thread_ps(void) {
  thread_ps_t *r = NULL;
  int i, j;

  if ((r = sys_calloc(num_threads + 2, sizeof(thread_ps_t))) != NULL) {
    r[0].tid = 0;
    r[0].handle = -1;
    r[0].name = sys_strdup("MAIN");
    r[0].p = 0;

    for (i = 0, j = 0; i < MAX_THREADS && j < num_threads; i++) {
      if (tasks[i].inuse) {
        r[j+1].tid = i;
        r[j+1].handle = tasks[i].id;
        r[j+1].name = sys_strdup(tasks[i].name);
        r[j+1].p = 0;
        j++;
      }
    }
  }

  return r;
}
