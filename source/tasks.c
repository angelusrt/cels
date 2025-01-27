#include "tasks.h"

lists_generate_implementation(routine, task, defaults_free)

bool routines_make(routine *self) {
	while (true) {
		bool is_finished = true;

		routine_iterator it = {0};
		while (routines_next(self, &it)) {
			int status = it.data->callback.func(
				it.data->callback.params,
				&it.data->state);

			if (status != task_finished_state) {
				it.data->status = status;
				is_finished = false;
			} else {
				it.data->status = 0;
			}
		}

		if (is_finished) {
			break;
		}
	}

	return true;
}

error routines_push_with(
	routine *self, 
	taskfunc callback, 
	void *args, 
	const allocator *mem
) {
	task task = {
		.callback={.func=callback, .params=args}
	};

	error push_error = routines_push(self, task, mem);
	return push_error;
}
