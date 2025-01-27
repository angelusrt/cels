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

typedef task_state (*taskfunc)(void *, size_t *state);

typedef struct task_functor {
	taskfunc func;
	void *params;
} task_functor;

typedef struct task {
	task_functor callback;
	size_t state;
	task_state status;
} task;

lists_generate_definition(routine, task)

/*
 * Executes tasks in a concurrent manner.
 *
 * #to-review
 */
bool routines_make(routine *self);

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
