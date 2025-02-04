#define cels_openssl 0

#include "../cels/source/requests.h"
#include "../cels/source/files.h"
#include "../cels/source/tasks.h"

#include "../cels/source/files.c"
#include "../cels/source/requests.c"
#include "../cels/source/tasks.c"
#include "../cels/source/strings.c"
#include "../cels/source/vectors.c"
#include "../cels/source/nodes.c"
#include "../cels/source/errors.c"
#include "../cels/source/mems.c"
#include "../cels/source/maths.c"
#include <time.h>

typedef struct downloader_param {
	const string url;
	request_async request;
	size_t id;
	const allocator mem;
	bool *has_ended;
} downloader_param;

task_state downloader(downloader_param *args) {
	bool may_continue = requests_make_async(&args->url, &args->request, &args->mem);
	if (!may_continue) { 
		*args->has_ended = true;
		printf(
			"downloader_%zu ended (error: %s).\n", 
			args->id, 
			request_error_messages[args->request.response.error]);

		return task_finished_state; 
	}

	printf(
		"downloader_%zu worked (size: %zu).\n", 
		args->id, 
		args->request.internal.response.size);

	return task_ready_state;
}

typedef struct writer_param {
	size_t id;
	file *file;
	string *text;
	file_write file_write;
	size_t state;
	bool *producer_has_ended;
} writer_param;

task_state writer(writer_param *args) {
	if (*args->producer_has_ended) { 
		printf("writer_%zu ended as producer stopped working.\n", args->id);
		return task_finished_state; 
	}

	if (args->state >= 3) {
		printf("writer_%zu panic'ed.\n", args->id);
		return task_finished_state; 
	}

	if (vectors_check((const vector *)args->text)) {
		args->state++;
		printf("writer_%zu skipped because string is malformed.\n", args->id);
		return task_waiting_state; 
	}

	if (args->text->size == 0) {
		args->state++;
		printf("writer_%zu skipped because string is malformed.\n", args->id);
		return task_waiting_state; 
	}

	args->state = 0;

	args->file_write.file = *args->text;
	bool may_continue = files_write_async(args->file, &args->file_write);
	*args->text = args->file_write.file;

	if (!may_continue) { 
		printf("writer_%zu ended.\n", args->id);
		return task_finished_state; 
	}

	printf("writer_%zu worked.\n", args->id);
	return task_ready_state;
}

int main(void) {
	static const string url = strings_premake(
		"ia600505.us.archive.org/7/items/"
		"EineKleineNachtmusik_201608/01_Allegro.mp3");

	file *file = fopen("my-file4.mp3", "w+");
	if (!file) {
		printf("file wasn't created\n");
		exit(1);
	}

	struct timespec start0 = {0};
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start0);

	/* shared memory */
	bool has_ended = false;
	const allocator mem = arenas_init(string_large_size);

	downloader_param param0 = {
		.mem=mem, 
		.url=url, 
		.has_ended=&has_ended, 
		.request={.option={.flags=request_async_raw_mode_flag}}
	};

	string *file_to_write = &param0.request.internal.response;
	writer_param param1 = {
		.file=file, 
		.text=file_to_write, 
		.producer_has_ended=&has_ended, 
		.file_write={.size=string_large_size, .consume=true}
	};

	routine routine = routines_init(vector_min, &mem);
	routines_push_with(&routine, (taskfunc)downloader, &param0, &mem);
	routines_push_with(&routine, (taskfunc)writer, &param1, &mem);
	routines_make(&routine, null);

	struct timespec end0 = {0};
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end0);

	double elapsed = (end0.tv_sec - start0.tv_sec);
    elapsed += (end0.tv_nsec - start0.tv_nsec) / 10000000.0;
	printf("took: %lfs\n", elapsed);

	mems_free(&mem, null);
	fclose(file);

	return 0;
}
