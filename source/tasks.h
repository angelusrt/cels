#ifndef cels_tasks_h
#define cels_tasks_h

#include "nodes.h"
#include "pthread.h"


/*
 * The module 'tasks' deals with 
 * concurrency, threads, asynchronous 
 * programming and synchronization.
 */


/* tasks */

typedef enum task_state {
	task_finished_state = -1,
	task_empty_state,
	task_starting_state,
	task_waiting_state,
	task_ready_state,
} task_state;

typedef task_state (*taskfunc)(void *params);

typedef struct task_functor {
	taskfunc func;
	void *params;
} task_functor;

typedef struct task {
	task_functor callback;
	task_state status;
} task;

pools(routine, task)

typedef task_state (*supervisorfunc)
	(routine *routine, task *task, void *params);

typedef struct supervisor_functor {
	supervisorfunc func;
	void *params;
} supervisor_functor;

typedef struct supervisor {
	supervisor_functor callback;
	task_state status;
} supervisor;

typedef struct routine_option {
	supervisor *supervisor;

	/* for threads */
	bool is_threads_enabled;
	allocator *mem;
} routine_option;

/* routines */

/*
 * Initializes a routine object.
 */
routine routines_init(size_t capacity, const allocator *mem);

/*
 * Executes tasks in a concurrent manner.
 *
 * #to-review
 */
error routines_make(routine *self, routine_option option);

/*
 * Convenience over 'push' that 
 * creates task with arguments.
 *
 * #to-review
 */
error routines_push_with(
	routine *self, taskfunc callback, void *args, const allocator *mem);

/*
 * Convenience over 'push_to' that 
 * creates task with arguments.
 *
 * #to-review
 */
error routines_push_to(
	routine *self, 
	taskfunc callback, 
	void *args, 
	size_t n, 
	const allocator *mem);

/*
 * Frees routine.
 *
 * #to-review
 */
void routines_free(routine *self, const allocator *mem);

#endif
