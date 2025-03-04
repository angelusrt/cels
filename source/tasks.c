#include "tasks.h"


/* routines */

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

bool routines_make(routine *self, supervisor *supervisor) {
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

			if (supervisor && supervisor->status != task_finished_state) {
				int super_status = supervisor->callback.func(
					self, it.data, supervisor->callback.params);

				supervisor->status = super_status;
 			}

		}

		if (is_finished) {
			break;
		}
	}

	return true;
}

error routines_push_with(
	routine *self, taskfunc callback, void *args, const allocator *mem) {

	task task = {
		.callback={.func=callback, .params=args}
	};

	error push_error = pools_push(self, &task, mem);
	return push_error;
}

void routines_free(routine *self, const allocator *mem) {
	pools_free(&self, null, mem);
}
