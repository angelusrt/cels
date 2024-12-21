#include "../source/errors.h"
#include "../source/strings.h"

__attribute_warn_unused_result__
error_report strings_test_init_and_push() {
	printf(__func__);
	printf("\n");

	size_t stat = 0, total = 0;

	string text = strings_init(vectors_min, null);
	string textpredict = strings_premake("00000");
	for (size_t i = 0; i < 5; i++) {
		strings_push(&text, '0', null);
	}

	stat += errors_assert("push('0')x5 == \"00000\"", strings_seems(&text, &textpredict));
	total++;

	strings_free(&text, null);
	return (error_report) {.total=total, .successfull=stat};
}

__attribute_warn_unused_result__
error_report strings_test_premake_and_check() {
	printf(__func__);
	printf("\n");

	size_t stat = 0, total = 0;

	string a = {0};
	stat += errors_assert("check({}) == true", strings_check(&a));
	total++;

	string b = strings_premake("test");
	stat += errors_assert("check(\"test\") == false", !strings_check(&b));
	total++;

	string c = strings_premake("");
	stat += errors_assert("check(\"\") == false", !strings_check(&c));
	total++;

	return (error_report) {.total=total, .successfull=stat};
}

__attribute_warn_unused_result__
error_report strings_test_check_extra() {
	printf(__func__);
	printf("\n");

	size_t stat = 0, total = 0;

	string a = {0};
	stat += errors_assert("check_extra({}) == true", strings_check_extra(&a));
	total++;

	string b = strings_premake("test");
	stat += errors_assert("check_extra(\"test\") == false", !strings_check_extra(&b));
	total++;

	string c = strings_premake("");
	stat += errors_assert("check_extra(\"\") == true", strings_check_extra(&c));
	total++;

	return (error_report) {.total=total, .successfull=stat};
}

__attribute_warn_unused_result__
error_report strings_test_check_charset() {
	printf(__func__);
	printf("\n");

	size_t stat = 0, total = 0;

	string text = strings_premake("exemplo");
	string charset = strings_premake("exmplo_");
	bool is_valid = strings_check_charset(&text, &charset);
	stat += errors_assert("check_charset(\"exemplo\", \"exmplo_\") == true", is_valid);
	total++;

	string charset_alt = strings_premake("_");
	is_valid = strings_check_charset(&text, &charset_alt);
	stat += errors_assert("check_charset(\"exemplo\", \"_\") == false", !is_valid);
	total++;

	string text_alt = strings_premake("_");
	is_valid = strings_check_charset(&text_alt, &charset_alt);
	stat += errors_assert("check_charset(\"_\", \"_\") == true", is_valid);
	total++;

	string text2 = strings_premake("coração");
	string charset2 = strings_premake("coraçã");
	is_valid = strings_check_charset(&text2, &charset2);
	stat += errors_assert("check_charset(\"coração\", \"coraçã\") == true", is_valid);
	total++;

	return (error_report) {.total=total, .successfull=stat};
}

__attribute_warn_unused_result__
error_report strings_test_make_and_free() {
	printf(__func__);
	printf("\n");

	size_t stat = 0, total = 0;

	string textlit = strings_premake("texto");
	string text = strings_make(textlit.data, null);
	bool textequals = strings_seems(&textlit, &text);

	stat += errors_assert("make(\"texto\") == \"texto\"", textequals);
	total++;

	strings_free(&text, null);
	bool textfreed = text.data == null;
	stat += errors_assert("free(&text) == null", textfreed);
	total++;

	return (error_report) {.total=total, .successfull=stat};
}

error_report strings_test_compare() {
	printf(__func__);
	printf("\n");

	size_t stat = 0, total = 0;

	string text0 = strings_premake("caro");
	string text1 = strings_premake("cara");
	string text2 = strings_premake("CARO");
	string text3 = strings_premake("çaro");

	bool is_bigger = strings_compare(&text0, &text1);
	stat += errors_assert("compare(\"caro\", \"cara\") == true", is_bigger);
	total++;

	is_bigger = strings_compare(&text0, &text0);
	stat += errors_assert("compare(\"caro\", \"caro\") == false", !is_bigger);
	total++;

	is_bigger = strings_compare(&text0, &text2);
	stat += errors_assert("compare(\"caro\", \"CARO\") == false", !is_bigger);
	total++;

	is_bigger = strings_compare(&text3, &text0);
	stat += errors_assert("compare(\"çaro\", \"caro\") == true", is_bigger);
	total++;

	return (error_report) {.total=total, .successfull=stat};
}

error_report strings_test_equals() {
	printf(__func__);
	printf("\n");

	size_t stat = 0, total = 0;

	string text0 = strings_premake("caro");
	string text1 = strings_premake("cara");
	string text2 = strings_premake("CARO");
	string text3 = strings_premake("çaro");

	bool equals = strings_equals(&text0, &text1);
	stat += errors_assert("equals(\"caro\", \"cara\") == false", !equals);
	total++;

	equals = strings_equals(&text0, &text0);
	stat += errors_assert("equals(\"caro\", \"caro\") == true", equals);
	total++;

	equals = strings_equals(&text0, &text2);
	stat += errors_assert("equals(\"caro\", \"CARO\") == falso", !equals);
	total++;

	equals = strings_equals(&text0, &text3);
	stat += errors_assert("equals(\"caro\", \"çaro\") == false", !equals);
	total++;

	return (error_report) {.total=total, .successfull=stat};
}

error_report strings_test_seems() {
	printf(__func__);
	printf("\n");

	size_t stat = 0, total = 0;

	string text0 = strings_premake("caro");
	string text1 = strings_premake("cara");
	string text2 = strings_premake("CARO");
	string text3 = strings_premake("çaro");

	bool seems = strings_seems(&text0, &text1);
	stat += errors_assert("seems(\"caro\", \"cara\") == false", !seems);
	total++;

	seems = strings_seems(&text0, &text0);
	stat += errors_assert("seems(\"caro\", \"caro\") == true", seems);
	total++;

	seems = strings_seems(&text0, &text2);
	stat += errors_assert("seems(\"caro\", \"CARO\") == true", seems);
	total++;

	seems = strings_seems(&text0, &text3);
	stat += errors_assert("seems(\"caro\", \"çaro\") == false", !seems);
	total++;

	return (error_report) {.total=total, .successfull=stat};
}

error_report strings_test_find() {
	printf(__func__);
	printf("\n");

	size_t stat = 0, total = 0;

	string text0 = strings_premake("Um 'um'");
	string text1 = strings_premake("um");
	string text2 = strings_premake("_");

	ssize_t pos = strings_find(&text0, &text1, 0);
	stat += errors_assert("find(\"Um 'um'\", \"um\", 0) == 0", pos == 0);
	total++;

	pos = strings_find(&text0, &text1, 1);
	stat += errors_assert("find(\"Um 'um'\", \"um\", 1) == 4", pos == 4);
	total++;

	pos = strings_find(&text0, &text2, 0);
	stat += errors_assert("find(\"Um 'um'\", \"_\", 0) == -1", pos == -1);
	total++;

	return (error_report) {.total=total, .successfull=stat};
}

bool _sizes_equals(size_t *s0, size_t *s1) { return &s0 == &s1; }

error_report strings_test_make_find() {
	printf(__func__);
	printf("\n");

	size_t stat = 0, total = 0;

	string text0 = strings_premake("Um 'um' mais do que um.");
	string text1 = strings_premake("um");

	size_vec texts0 = strings_make_find(&text0, &text1, 0, null);
	size_vec positions = vectors_premake(size_t, 3, 0, 4, 20);
	bool matches = size_vecs_equals(&texts0, &positions);

	stat += errors_assert("make_find(\"Um 'um' mais do que um.\", \"um\", 0).data == [0, 4, 20]", matches);
	total++;

	size_vec texts1 = strings_make_find(&text0, &text1, 1, null);
	matches = texts1.size == 1;

	stat += errors_assert("make_find(\"Um 'um' mais do que um.\", \"um\", 1).size == 1", matches);
	total++;

	string text2 = strings_premake("coração");
	string text3 = strings_premake("ã");
	size_vec texts2 = strings_make_find(&text2, &text3, 0, null);
	matches = texts2.size == 1;

	stat += errors_assert("make_find(\"coração\", \"ã\", 1).size == 1", matches);
	total++;

	size_vecs_free(&texts0, null);
	size_vecs_free(&texts1, null);
	size_vecs_free(&texts2, null);
	return (error_report) {.total=total, .successfull=stat};
}

error_report strings_test_replace() {
	printf(__func__);
	printf("\n");

	size_t stat = 0, total = 0;

	string charset0 = strings_premake("an");
	string charset1 = strings_premake(" ");
	string charset2 = strings_premake("h");
	string text0 = strings_make("alehandrah", null);

	strings_replace(&text0, &charset0, ' ', 0);
	string text1 = strings_premake(" leh  dr h");
	bool matches = strings_seems(&text0, &text1);
	stat += errors_assert("replace(\"alehandrah\", \"an\", \" \", 0) == \" leh  dr h\"", matches);
	total++;

	strings_replace(&text0, &charset1, -1, 0);
	string text2 = strings_premake("lehdrh");
	matches = strings_seems(&text0, &text2);
	stat += errors_assert("replace(\" leh  dr h\", \" \", -1, 0) == \"lehdrh\"", matches);
	total++;

	strings_replace(&text0, &charset2, 'e', 1);
	string text3 = strings_premake("leedrh");
	matches = strings_seems(&text0, &text3);
	stat += errors_assert("replace(\"lehdrh\", \"h\", \"e\", 1) == \"leedrh\"", matches);
	total++;

	strings_free(&text0, null);
	return (error_report) {.total=total, .successfull=stat};
}

error_report strings_test_make_replace() {
	printf(__func__);
	printf("\n");

	size_t stat = 0, total = 0;

	string text0 = strings_premake("o bão bom.");
	string text1 = strings_premake("o b");

	string text2predict = strings_premake("ãom.");
	string text2 = strings_make_replace(&text0, &text1, null, 0, null);

	bool matches = strings_seems(&text2, &text2predict);
	stat += errors_assert("make_replace(\"o bão bom.\", \"o b\", null, 0) == \"ãom.\"", matches);
	total++;

	string text3predict = strings_premake("ão bom.");
	string text3 = strings_make_replace(&text0, &text1, null, 1, null);

	matches = strings_seems(&text3, &text3predict);
	stat += errors_assert("make_replace(\"o bão bom.\", \"o b\", null, 1) == \"ão bom.\"", matches);
	total++;

	string text4 = strings_premake("o c");
	string text5predict = strings_premake("o cão bom.");
	string text5 = strings_make_replace(&text0, &text1, &text4, 1, null);

	matches = strings_seems(&text5, &text5predict);
	stat += errors_assert("make_replace(\"o bão bom.\", \"o b\", \"o c\", 1) == \"o cão bom.\"", matches);
	total++;

	string text7 = strings_premake("ã");
	string text6predict = strings_premake("ãããom.");
	string text6 = strings_make_replace(&text0, &text1, &text7, 0, null);

	matches = strings_seems(&text6, &text6predict);
	stat += errors_assert("make_replace(\"o bão bom.\", \"o b\", \"ã\", 0) == \"ãããom.\"", matches);
	total++;

	strings_free(&text2, null);
	strings_free(&text3, null);
	strings_free(&text5, null);
	strings_free(&text6, null);

	return (error_report) {.total=total, .successfull=stat};
}

error_report strings_test_make_split() {
	printf(__func__);
	printf("\n");

	size_t stat = 0, total = 0;

	string text0 = strings_premake("aumbumcumd");
	string text1 = strings_premake("um");

	string_vec text2 = strings_make_split(&text0, &text1, 0, null);
	string_vec text2predict = vectors_premake(
		string, 4, 
		strings_premake("a"), 
		strings_premake("b"), 
		strings_premake("c"),
		strings_premake("d"));

	bool matches = string_vecs_seems(&text2, &text2predict);
	stat += errors_assert(
		"make_split(\"aumbumcumd\", \"um\", 0) == [\"a\",\"b\",\"c\",\"d\"]", 
		matches);
	total++;

	string_vec text3 = strings_make_split(&text0, &text1, 1, null);
	string_vec text3predict = vectors_premake(
			string, 2, 
			strings_premake("a"), 
			strings_premake("bumcumd"));

	matches = string_vecs_seems(&text3, &text3predict);
	stat += errors_assert(
		"make_split(\"aumbumcumd\", \"um\", 1) == [\"a\",\"bumcumd\"]", 
		matches);
	total++;

	string_vecs_free(&text2, null);
	string_vecs_free(&text3, null);

	return (error_report) {.total=total, .successfull=stat};
}

error_report strings_test_make_format() {
	printf(__func__);
	printf("\n");

	size_t stat = 0, total = 0;

	string json = strings_make_format("{\"age\": %d}", null, 10);
	string jsonpredict = strings_premake("{\"age\": 10}");

	bool matches = strings_seems(&json, &jsonpredict);
	stat += errors_assert("make_format(\"{\"age\": %d}\", 10) == \"{\"age\": 10}\"", matches);
	total++;

	strings_free(&json, null);
	return (error_report) {.total=total, .successfull=stat};
}

error_report strings_test_hasherize() {
	printf(__func__);
	printf("\n");

	size_t stat = 0, total = 0;

	size_t hash = strings_hasherize(&(string)strings_premake("idea"));
	stat += errors_assert("hasherize(\"idea\") == 273175", hash == 273175);
	total++;

	return (error_report) {.total=total, .successfull=stat};
}

error_report strings_test_lower_and_upper() {
	printf(__func__);
	printf("\n");

	size_t stat = 0, total = 0;

	string text = strings_make("eXeMpLo", null);
	string textlower = strings_premake("exemplo");
	string textupper = strings_premake("EXEMPLO");

	strings_upper(&text);
	bool matches = strings_equals(&text, &textupper);
	stat += errors_assert("upper(\"eXeMpLo\") == \"EXEMPLO\"", matches);
	total++;

	strings_lower(&text);
	matches = strings_equals(&text, &textlower);
	stat += errors_assert("lower(\"EXEMPLO\") == \"exemplo\"", matches);
	total++;

	strings_free(&text, null);
	return (error_report) {.total=total, .successfull=stat};
}

error_report strings_test_next() {
	printf(__func__);
	printf("\n");

	size_t stat = 0, total = 0;

	string text0 = strings_premake("a, b, c");
	string text1 = strings_premake(", ");
	string text2 = {0}; //view
	string_vec predict = vectors_premake(
		string, 3, 
		strings_premake("a"),
		strings_premake("b"),
		strings_premake("c"));

	strings_next(&text0, &text1, &text2);
	bool matches = strings_equals(&text2, &predict.data[0]);
	stat += errors_assert("next(\"a, b, c\", \", \", {0}) == \"a\"", matches);
	total++;

	strings_next(&text0, &text1, &text2);
	matches = strings_equals(&text2, &predict.data[1]);
	stat += errors_assert("next(\"a, b, c\", \", \", \"a\") == \"b\"", matches);
	total++;

	strings_next(&text0, &text1, &text2);
	matches = strings_equals(&text2, &predict.data[2]);
	stat += errors_assert("next(\"a, b, c\", \", \", \"b\") == \"c\"", matches);
	total++;

	string textalt = strings_premake("_");

	text2.data = null;
	strings_next(&text0, &textalt, &text2);
	matches = strings_equals(&text2, &text0);
	stat += errors_assert("next(\"a, b, c\", \"_\", {0}) == \"a, b, c\"", matches);
	total++;

	size_t count = 0;
	text2.data = null;
	while(!strings_next(&text0, &text1, &text2)) { count++; }
	stat += errors_assert("next(\"a, b, c\", \", \", {0}).count == 3", count == 3);
	total++;

	return (error_report) {.total=total, .successfull=stat};
}

void string_maps_print(string_map *self) {
	strings_print(&self->data.key);
	strings_print(&self->data.value);
	printf("\n");
}

error_report string_maps_test_get_and_push() {
	printf(__func__);
	printf("\n");

	size_t stat = 0, total = 0;

	string_map *json = null;
	string namepredict = strings_premake("angelus");

	string_maps_make_push(&json, "name", "angelus", null);
	string_maps_make_push(&json, "age", "10", null);

	string key = strings_premake("name");
	string *name = string_maps_get(json, key);

	bool is_valid = name != null;
	if (is_valid) { 
		is_valid = strings_equals(name, &namepredict); 
	}

	stat += errors_assert("string_maps_get(json, \"name\") == \"angelus\"", is_valid);
	total++;

	string_maps_make_push(&json, "name", "angelus", null);

	size_t name_freq = string_maps_get_frequency(json, key);
	is_valid = name_freq == 2; 

	stat += errors_assert("string_maps_push(json, \"name\", \"angelus\").frequency == 2", is_valid);
	total++;

	string_maps_free(json, null);
	return (error_report) {.total=total, .successfull=stat};
}

void strings_test() {
	printf("=======\n");
	printf("strings\n");
	printf("=======\n\n");

	error_report rep;
	reportfunc functions[] = {
		strings_test_init_and_push,
		strings_test_premake_and_check,
		strings_test_check_extra,
		strings_test_check_charset,
		strings_test_make_and_free,
		strings_test_compare,
		strings_test_equals,
		strings_test_seems,
		strings_test_find,
		strings_test_make_find,
		strings_test_replace,
		strings_test_make_replace,
		strings_test_make_split,
		strings_test_make_format,
		strings_test_hasherize,
		strings_test_lower_and_upper,
		strings_test_next,
		string_maps_test_get_and_push,
		null,
	};

	size_t i = 0;
	while (functions[i]) {
		rep = functions[i]();
		error_reports_print(&rep);
		i++;
	}
}
