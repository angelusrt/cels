#include "../source/requests.h"
#include "../source/files.h"

#include "../source/files.c"
#include "../source/requests.c"
#include "../source/tasks.c"
#include "../source/strings.c"
#include "../source/vectors.c"
#include "../source/nodes.c"
#include "../source/errors.c"
#include "../source/mems.c"
#include "../source/maths.c"
#include <time.h>

int main(void) {
	clock_t start = clock();
	const allocator mem = arenas_init(2048);

	string url = strings_premake(
		"ia903105.us.archive.org"
		"/28/items/symphonyno.9_201912/"
		"01%20Allegro%20ma%20non%20troppo%20-%20un%20poco%20maestoso.mp3");

	string head = strings_premake("Accept:*/*\r\n");
	string port = strings_premake("443");
	request_option opts = {
		.port=port, 
		.head=head,
		.version=request_one_version
	};

	eresponse response = requests_make(&url, &opts, &mem);
	clock_t end = clock();

	if (response.error == request_successfull) {
		file *file = fopen("my-file.jpeg", "w+");
		if (!file) {
			printf("file wasn't created\n");
			goto cleanup;
		}

		file_error write_error = files_write(file, response.value.body);
		if (write_error != file_successfull) {
			printf("file wasn't written\n");
			goto cleanup;
		}

		printf("file successfully written\n");

		printf("head:\n");
		strings_println((string *)&response.value.head);
		printf("\nbody:\n");
		strings_println((string *)&response.value.body);
	} else {
		printf("response.error = %d\n", response.error);
		request_errors_println(response.error);
	}

	cleanup:
	printf("took: %lfs\n", ((double)(end - start))/CLOCKS_PER_SEC);
	mems_free(&mem, null);
	return 0;
}
