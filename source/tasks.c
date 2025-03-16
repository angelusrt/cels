#include "tasks.h"


/* public */

typedef struct routine_thread {
	routine *self;
	supervisor *supervisor;
	size_t thread_id;
} routine_thread;

void *routines_make_thread_private(void *args) {
	routine_thread *thread = args;

	routine *self = thread->self;
	supervisor *sup = thread->supervisor;
	size_t id = thread->thread_id;

	while (true) {
		bool is_finished = true;

		routine_iterator it = {0};
		while (pools_next_in(self, id, &it)) {
			if (it.data->status == task_finished_state) {
				continue;
			}

			int status = it.data->callback.func(
				it.data->callback.params);

			it.data->status = status;
			if (status != task_finished_state) {
				is_finished = false;
			} 

			if (sup && sup->status != task_finished_state) {
				int super_status = sup->callback.func(
					self, it.data, sup->callback.params);

				sup->status = super_status;
 			}

		}

		if (is_finished) {
			break;
		}
	}

	return null;
}

error routines_make_with_threads_private(
	routine *self, routine_option option) {

	supervisor *sup = option.supervisor;


	size_t total = 1;
	routine_block *node = self->data;
	if (!node) { return false; }

	while (true) {
		node = node->next;
		++total;
	}


	node = self->data;
	pthread_t *threads = mems_alloc(
		option.mem, sizeof(pthread_t) * total);

	#if cels_debug
		errors_abort("threads", !threads);
	#else
		if (threads) { return fail; }
	#endif


	for (size_t i = 0; i < total; i++) {
		routine_thread rt = {
			.self=self,
			.thread_id=i,
			.supervisor=sup,
		};

		pthread_create(
			&threads[i], 
			null, 
			routines_make_thread_private, 
			&rt);

		node = node->next;
	}


	for (size_t i = 0; i < total; i++) {
		pthread_join(threads[i], null);
	}


	mems_dealloc(
		option.mem, threads, sizeof(pthread_t) * total);

	return true;
}

error routines_make_private(routine *self, routine_option option) {
	supervisor *sup = option.supervisor;

	while (true) {
		bool is_finished = true;

		routine_iterator it = {0};
		while (pools_next(self, &it)) {
			if (it.data->status == task_finished_state) {
				continue;
			}

			int status = it.data->callback.func(
				it.data->callback.params);

			it.data->status = status;
			if (status != task_finished_state) {
				is_finished = false;
			} 

			if (sup && sup->status != task_finished_state) {
				int super_status = sup->callback.func(
					self, it.data, sup->callback.params);

				sup->status = super_status;
 			}

		}

		if (is_finished) {
			break;
		}
	}

	return ok;
}


/* public */

routine routines_init(size_t capacity, const allocator *mem) {
	routine self = {0}; 

	pools_init(
		&self, 
		sizeof(routine_block_item), 
		sizeof(task), 
		offsetof(routine_block_item, data), 
		capacity, 
		mem);

	return self;
}

error routines_make(routine *self, routine_option option) {
	if (option.is_threads_enabled) {
		return routines_make_with_threads_private(self, option);
	} 

	return routines_make_private(self, option);
}

error routines_push_with(
	routine *self, taskfunc callback, void *args, const allocator *mem) {

	task task = {
		.callback={.func=callback, .params=args}
	};

	error push_error = pools_push(self, &task, mem);
	return push_error;
}

error routines_push_to(
	routine *self, 
	taskfunc callback, 
	void *args, 
	size_t n, 
	const allocator *mem) {

	task task = {
		.callback={.func=callback, .params=args}
	};

	error push_error = pools_push_to(self, &task, n, mem);
	return push_error;
}

void routines_free(routine *self, const allocator *mem) {
	pools_free(&self, null, mem);
}
