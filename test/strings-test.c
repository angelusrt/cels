#include "../source/errors.h"
#include "../source/strings.h"

void strings_push_test(error_report *report) {
	const string ban = strings_premake("ban");
	const string ana = strings_premake("ana");
	const string predict0 = strings_premake("banana");
	const string predict1 = strings_premake("banan");
	string text = strings_init(vector_min, null);

	strings_push(&text, ban, null);
	bool seems = strings_seems(&text, &ban);
	errors_expect("push('ban') == 'ban'", seems, report);

	strings_push(&text, ana, null);
	seems = strings_seems(&text, &predict0);
	errors_expect("push('ana') == 'banana'", seems, report);

	strings_pop(&text, null);
	seems = strings_seems(&text, &predict1);
	errors_expect("pop('a') == 'banan'", seems, report);

	strings_free(&text, null);
}

void strings_check_test(error_report *report) {
	bool expect = strings_check(&(string){0});
	errors_expect("check({}) == fail", expect, report);

	expect = strings_check(&strings_do("test"));
	errors_expect("check('test') == ok", !expect, report);

	expect = strings_check(&strings_do(""));
	errors_expect("check('') == ok", !expect, report);
}

void strings_check_extra_test(error_report *report) {
	bool expect = strings_check_extra(&(string){0});
	errors_expect("check_extra({}) == fail", expect, report);

	expect = strings_check_extra(&strings_do("test"));
	errors_expect("check_extra('test') == ok", !expect, report);
	
	expect = strings_check_extra(&strings_do(""));
	errors_expect("check_extra('') == fail", expect, report);
}

void strings_check_charset_test(error_report *report) {
	const string text0 = strings_premake("exemplo");
	const string text1 = strings_premake("_");
	const string text2 = strings_premake("coração");
	const string charset0 = strings_premake("exmplo_");
	const string charset1 = strings_premake("_");
	const string charset2 = strings_premake("coraçã");
	
	bool expect = strings_check_charset(&text0, charset0);
	errors_expect("check_charset('exemplo', 'exmplo_') == true", expect, report);

	expect = strings_check_charset(&text0, charset1);
	errors_expect("check_charset('exemplo', '_') == false", !expect, report);

	expect = strings_check_charset(&text1, charset1);
	errors_expect("check_charset('_', '_') == true", expect, report);

	expect = strings_check_charset(&text2, charset2);
	errors_expect("check_charset('coração', 'coraça') == true", expect, report);
}

void strings_make_and_free_test(error_report *report) {
	const string predict = strings_premake("texto");
	string text = strings_make(predict.data, null);

	bool equals = strings_seems(&predict, &text);
	errors_expect("make('texto') == 'texto'", equals, report);
	
	strings_free(&text, null);
	errors_expect("free(&text) == null", !text.data, report);
}

void strings_clone_test(error_report *report) {
	const string predict = strings_premake("texto");
	string text = strings_clone(&predict, null);

	bool equals = strings_seems(&predict, &text);
	errors_expect("clone('texto') == 'texto'", equals, report);
	strings_free(&text, null);
}

void strings_view_test(error_report *report) {
	const string text0 = strings_premake("texto");
	const string predict = strings_premake("tex");

	string text1 = strings_view(&text0, 0, 2);
	bool equals = strings_seems(&predict, &text1);
	errors_expect("view('texto', 0, 2) == 'tex'", equals, report);

	string text2 = strings_unview(&text1, null);
	equals = strings_seems(&predict, &text2);
	errors_expect("unview('tex') == 'tex'", equals, report);
	strings_free(&text2, null);
}

void strings_compare_test(error_report *report) {
	const string text0 = strings_premake("caro");
	const string text1 = strings_premake("cara");
	const string text2 = strings_premake("CARO");
	const string text3 = strings_premake("çaro");

	bool is_bigger = strings_compare(&text0, &text1);
	errors_expect("compare('caro', 'cara') == true", is_bigger, report);
	
	is_bigger = strings_compare(&text0, &text0);
	errors_expect("compare('caro', 'caro') == false", !is_bigger, report);
	
	is_bigger = strings_compare(&text0, &text2);
	errors_expect("compare('caro', 'CARO') == false", !is_bigger, report);
	
	is_bigger = strings_compare(&text3, &text0);
	errors_expect("compare('çaro', 'caro') == true", is_bigger, report);
}

void strings_equals_test(error_report *report) {
	const string text0 = strings_premake("caro");
	const string text1 = strings_premake("cara");
	const string text2 = strings_premake("CARO");
	const string text3 = strings_premake("çaro");

	bool equals = strings_equals(&text0, &text1);
	errors_expect("equals('caro', 'cara') == false", !equals, report);
	
	equals = strings_equals(&text0, &text0);
	errors_expect("equals('caro', 'caro') == true", equals, report);
	
	equals = strings_equals(&text0, &text2);
	errors_expect("equals('caro', 'CARO') == falso", !equals, report);
	
	equals = strings_equals(&text0, &text3);
	errors_expect("equals('caro', 'çaro') == false", !equals, report);
}

void strings_seems_test(error_report *report) {
	const string text0 = strings_premake("caro");
	const string text1 = strings_premake("cara");
	const string text2 = strings_premake("CARO");
	const string text3 = strings_premake("çaro");

	bool seems = strings_seems(&text0, &text1);
	errors_expect("seems('caro', 'cara') == false", !seems, report);
	
	seems = strings_seems(&text0, &text0);
	errors_expect("seems('caro', 'caro') == true", seems, report);
	
	seems = strings_seems(&text0, &text2);
	errors_expect("seems('caro', 'CARO') == true", seems, report);
	
	seems = strings_seems(&text0, &text3);
	errors_expect("seems('caro', 'çaro') == false", !seems, report);
}

void strings_find_test(error_report *report) {
	const string text0 = strings_premake("Um 'um'");
	const string text1 = strings_premake("um");
	const string text2 = strings_premake("_");
	const string text3 = strings_premake("{exemplo: {a: {}}}");

	ssize_t pos = strings_find(&text0, text1, 0);
	errors_expect("find('Um 'um'', 'um', 0) == 0", pos == 0, report);
	
	pos = strings_find(&text0, text1, 1);
	errors_expect("find('Um 'um'', 'um', 1) == 4", pos == 4, report);
	
	pos = strings_find(&text0, text2, 0);
	errors_expect("find('Um 'um'', '_', 0) == -1", pos == -1, report);

	pos = strings_find_matching(&text3, strings_do("{"), strings_do("}"), 0);
	errors_expect("find_matching('{exemplo: {a: {}}}', '{', '}', 0) == 17", pos == 17, report);
}

void strings_find_from_test(error_report *report) {
	const string text0 = strings_premake("Um 'um'");
	const string text1 = strings_premake("um");
	const string text2 = strings_premake("_");

	ssize_t pos = strings_find_from(&text0, text1, 0);
	errors_expect("find_from('Um 'um'', 'um', 0) == 0", pos == 0, report);
	
	pos = strings_find_from(&text0, text1, 1);
	errors_expect("find_from('Um 'um'', 'um', 1) == 1", pos == 1, report);
	
	pos = strings_find_from(&text0, text2, 0);
	errors_expect("find_from('Um 'um'', '_', 0) == -1", pos == -1, report);
}

void strings_find_all_test(error_report *report) {
	const string text0 = strings_premake("Um 'um' mais que um.");
	const string text1 = strings_premake("um");
	const string text2 = strings_premake("coração");
	const string text3 = strings_premake("ã");
	const size_vec positions = vectors_premake(size_t, 0, 4, 17);

	size_vec texts0 = strings_find_all(&text0, text1, 0, null);
	bool matches = size_vecs_equals(&texts0, &positions);
	errors_expect("find_all('Um 'um' mais que um.', 'um', 0) == [0, 4, 17]", matches, report);
	
	size_vec texts1 = strings_find_all(&text0, text1, 1, null);
	matches = texts1.size == 1;
	errors_expect("find_all('Um 'um' mais que um.', 'um', 1).size == 1", matches, report);
	
	size_vec texts2 = strings_find_all(&text2, text3, 0, null);
	matches = texts2.size == 1;
	errors_expect("find_all('coração', 'ã', 1).size == 1", matches, report);
	
	size_vecs_free(&texts0, null);
	size_vecs_free(&texts1, null);
	size_vecs_free(&texts2, null);
}

void strings_replace_from_test(error_report *report) {
	string text0 = strings_make("alehandrah", null);
	const string text1 = strings_premake(" leh  dr h");
	const string text2 = strings_premake("lehdrh");
	const string text3 = strings_premake("leedrh");
	const string charset0 = strings_premake("an");
	const string charset1 = strings_premake(" ");
	const string charset2 = strings_premake("h");

	strings_replace_from(&text0, charset0, ' ', 0);
	bool matches = strings_seems(&text0, &text1);
	errors_expect("replace_from('alehandrah', 'an', ' ', 0) == ' leh  dr h'", matches, report);
	
	strings_replace_from(&text0, charset1, -1, 0);
	matches = strings_seems(&text0, &text2);
	errors_expect("replace_from(' leh  dr h', ' ', -1, 0) == 'lehdrh'", matches, report);
	
	strings_replace_from(&text0, charset2, 'e', 1);
	matches = strings_seems(&text0, &text3);
	errors_expect("replace_from('lehdrh', 'h', 'e', 1) == 'leedrh'", matches, report);
	
	strings_free(&text0, null);
}

void strings_replace_test(error_report *report) {
	const string text0 = strings_premake("o bão bom.");
	const string substring0 = strings_premake("o b");
	const string rep0 = strings_premake("");
	const string rep1 = strings_premake("o c");
	const string rep2 = strings_premake("ã");
	const string predict1 = strings_premake("ãom.");
	const string predict2 = strings_premake("ão bom.");
	const string predict3 = strings_premake("o cão bom.");
	const string predict4 = strings_premake("ãããom.");

	string text1 = strings_replace(&text0, substring0, rep0, 0, null);
	bool matches = strings_seems(&text1, &predict1);
	errors_expect("replace('o bão bom.', 'o b', '', 0) == 'ãom.'", matches, report);
	
	string text2 = strings_replace(&text0, substring0, rep0, 1, null);
	matches = strings_seems(&text2, &predict2);
	errors_expect("replace('o bão bom.', 'o b', '', 1) == 'ão bom.'", matches, report);
	
	string text3 = strings_replace(&text0, substring0, rep1, 1, null);
	matches = strings_seems(&text3, &predict3);
	errors_expect("replace('o bão bom.', 'o b', 'o c', 1) == 'o cão bom.'", matches, report);
	
	string text4 = strings_replace(&text0, substring0, rep2, 0, null);
	matches = strings_seems(&text4, &predict4);
	errors_expect("replace('o bão bom.', 'o b', 'ã', 0) == 'ãããom.'", matches, report);
	
	strings_free(&text1, null);
	strings_free(&text2, null);
	strings_free(&text3, null);
	strings_free(&text4, null);
}

void strings_split_test(error_report *report) {
	const string text0 = strings_premake("aumbumcumd");
	const string sep0 = strings_premake("um");
	string_vec predict1 = string_vecs_make(null, "a", "b", "c", "d");
	string_vec predict2 = string_vecs_make(null, "a", "bumcumd");

	string_vec text1 = strings_split(&text0, sep0, 0, null);
	bool matches = string_vecs_seems(&text1, &predict1);
	errors_expect("make_split('aumbumcumd', 'um', 0) == ['a','b','c','d']", matches, report);
	
	string_vec text2 = strings_split(&text0, sep0, 1, null);
	matches = string_vecs_seems(&text2, &predict2);
	errors_expect("make_split('aumbumcumd', 'um', 1) == ['a','bumcumd']", matches, report);
	
	string_vecs_free(&text1, null);
	string_vecs_free(&text2, null);
	string_vecs_free(&predict1, null);
	string_vecs_free(&predict2, null);
}

void strings_format_test(error_report *report) {
	string json = strings_format("{'age': %d}", null, 10);
	const string predict = strings_premake("{'age': 10}");

	bool matches = strings_seems(&json, &predict);
	errors_expect("format('{'age': %d}', 10) == '{'age': 10}'", matches, report);
	
	strings_free(&json, null);
}

void strings_hasherize_test(error_report *report) {
	const size_t hash = strings_prehash("idea");
	errors_expect("hasherize('idea') == 9840", hash == 9840, report);
}

void strings_lower_and_upper_test(error_report *report) {
	string text = strings_make("eXeMpLo", null);
	const string textlower = strings_premake("exemplo");
	const string textupper = strings_premake("EXEMPLO");

	strings_upper(&text);
	bool matches = strings_equals(&text, &textupper);
	errors_expect("upper('eXeMpLo') == 'EXEMPLO'", matches, report);
	
	strings_lower(&text);
	matches = strings_equals(&text, &textlower);
	errors_expect("lower('EXEMPLO') == 'exemplo'", matches, report);
	
	strings_free(&text, null);
}

void strings_next_test(error_report *report) {
	const string text0 = strings_premake("a, b, c");
	const string text1 = strings_premake(", ");
	const string textalt = strings_premake("_");
	string_vec predict = string_vecs_make(null, "a", "b", "c");
	string text2 = {0};

	strings_next(&text0, text1, &text2);
	bool matches = strings_equals(&text2, &predict.data[0]);
	errors_expect("next('a, b, c', ', ', {0}) == 'a'", matches, report);
	
	strings_next(&text0, text1, &text2);
	matches = strings_equals(&text2, &predict.data[1]);
	errors_expect("next('a, b, c', ', ', 'a') == 'b'", matches, report);
	
	strings_next(&text0, text1, &text2);
	matches = strings_equals(&text2, &predict.data[2]);
	errors_expect("next('a, b, c', ', ', 'b') == 'c'", matches, report);
	
	text2.data = null;
	strings_next(&text0, textalt, &text2);
	matches = strings_equals(&text2, &text0);
	errors_expect("next('a, b, c', '_', {0}) == 'a, b, c'", matches, report);
	
	size_t count = 0;
	text2.data = null;
	while(!strings_next(&text0, text1, &text2)) { count++; }
	errors_expect("next('a, b, c', ', ', {0}).count == 3", count == 3, report);
	string_vecs_free(&predict, null);
}

void strings_slice_test(error_report *report) {
	const string predict0 = strings_premake("exemplo");
	const string predict1 = strings_premake("exe");
	const string predict2 = strings_premake("ee");
	string text0 = strings_make(" exemplo", null);

	strings_trim(&text0);
	bool matches = strings_seems(&text0, &predict0);
	errors_expect("trim(' exemplo') == 'exemplo'", matches, report);

	strings_slice(&text0, 0, 3);
	matches = strings_seems(&text0, &predict1);
	errors_expect("slice('exemplo', 0, 3) == 'exe'", matches, report);

	strings_shift(&text0, 1, 1);
	matches = strings_seems(&text0, &predict2);
	errors_expect("shift('exe', 1) == 'ee'", matches, report);

	strings_free(&text0, null);
}

void strings_cut_test(error_report *report) {
	const string text0 = strings_premake("  exemplo ");
	const string predict0 = strings_premake("exemplo");

	const string trimmed_view = strings_cut(&text0);
	bool matches = strings_seems(&trimmed_view, &predict0);
	errors_expect("cut('  exemplo ') == 'exemplo'", matches, report);
}

void strings_has_test(error_report *report) {
	const string text0 = strings_premake("exemplo.c");
	const string has0 = strings_premake("exemplo");
	const string has1 = strings_premake(".c");
	const string has2 = strings_premake(".d");

	bool matches = strings_has_prefix(&text0, has0);
	errors_expect("has_prefix('exemplo.c', 'exemplo') == true", matches, report);

	matches = strings_has_prefix(&text0, has1);
	errors_expect("has_prefix('exemplo.c', '.c') == false", !matches, report);

	matches = strings_has_suffix(&text0, has1);
	errors_expect("has_suffix('exemplo.c', '.c') == true", matches, report);

	matches = strings_has_suffix(&text0, has2);
	errors_expect("has_suffix('exemplo.c', '.d') == false", !matches, report);
}

void string_vecs_join_test(error_report *report) {
	string_vec names = string_vecs_make(null, "a", "b", "c");
	string predict = strings_premake("a, b, c");
	const string sep = strings_premake(", ");

	string name_list = string_vecs_join(&names, sep, null);
	bool matches = strings_seems(&name_list, &predict);
	errors_expect("join(['a', 'b', 'c'], ', ') == 'a, b, c'", matches, report);

	string_vecs_free(&names, null);
	strings_free(&name_list, null);
}

void string_maps_get_and_push_test(error_report *report) {
	string_map json = string_maps_init();
	string namepredict = strings_premake("angelus");
	string key = strings_premake("name");

	string_maps_push_with(&json, "name", "angelus", null);
	string_maps_push_with(&json, "age", "10", null);
	string *name = string_maps_get(&json, key);

	bool is_valid = name != null;
	if (is_valid) { 
		is_valid = strings_equals(name, &namepredict); 
	}

	errors_expect("get(json, 'name') == 'angelus'", is_valid, report);
	string_maps_push_with(&json, "name", "angelus", null);

	string_map_node *node = bynary_trees_get(&json, strings_hasherize(&key));
	size_t name_freq = node ? node->frequency : 0;
	is_valid = name_freq == 2; 
	errors_expect("push(json, 'name', 'angelus').frequency == 2", is_valid, report);
	
	string_maps_free(&json, null);
}

void reportfuncs_do(reportfunc *functions, error_report *report) {
	size_t i = 0;
	while (functions[i]) {
		functions[i](report);
		printf("\n");
		i++;
	}
}

void strings_test(void) {
	printf("=======\n");
	printf("strings\n");
	printf("=======\n\n");

	error_report report = {0};
	reportfunc functions[] = {
		strings_push_test,
		strings_check_test,
		strings_check_extra_test,
		strings_check_charset_test,
		strings_make_and_free_test,
		strings_clone_test,
		strings_compare_test,
		strings_equals_test,
		strings_seems_test,
		strings_find_test,
		strings_find_from_test,
		strings_find_all_test,
		strings_replace_from_test,
		strings_replace_test,
		strings_split_test,
		strings_format_test,
		strings_hasherize_test,
		strings_lower_and_upper_test,
		strings_next_test,
		strings_slice_test,
		strings_has_test,
		strings_cut_test,
		string_vecs_join_test,
		string_maps_get_and_push_test,
		null,
	};

	reportfuncs_do(functions, &report);
	error_reports_print(&report);
}
