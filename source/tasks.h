#ifndef cels_tasks_h
#define cels_tasks_h

#include "nodes.h"

/*
 * The module 'tasks' deals with 
 * concurrency, threads, asynchronous 
 * programming and synchronization.
 */

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

pools_define(routine, task)

typedef task_state (*supervisorfunc)(routine *routine, task *task, void *params);

typedef struct supervisor_functor {
	supervisorfunc func;
	void *params;
} supervisor_functor;

typedef struct supervisor {
	supervisor_functor callback;
	task_state status;
} supervisor;

/*
 * Executes tasks in a concurrent manner.
 *
 * #to-review
 */
bool routines_make(routine *self, supervisor *supervisor);

/*
 * Convenience over 'push' that 
 * creates task with arguments.
 *
 * #to-review
 */
error routines_push_with(
	routine *self, 
	taskfunc callback, 
	void *args, 
	const allocator *mem
);

#endif
