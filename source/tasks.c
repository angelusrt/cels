#include "tasks.h"

pools_generate(routine, task, defaults_free)

bool routines_make(routine *self, supervisor *supervisor) {
	while (true) {
		bool is_finished = true;

		routine_iterator it = {0};
		while (routines_next(self, &it)) {
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
