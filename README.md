# Cels

Cels (C Extended Library's) is a libray for the C language
that proposes to provide convenience with fundamental 
utilities in a well-designed manner.

It requires a C99 compiler or C++11 one.

# Features

Cels has:
- vector and methods;
- string and methods;
- error-handling and error as values;
- file-handling;
- sets and maps;
- allocators;
- json and csv utilities;
- http request and server;
- a "package-manager" utility.

# Example

```C
int main(void) {
    const allocator mem = arenas_init(1024);

    string_vec words = {0};
    vectors_init(&words, sizeof(string), vector_min, &mem);

    string hello = strings_make("hello", &mem);
    vectors_push(&words, &hello, &mem);

    string world = strings_make("world", &mem);
    vectors_push(&words, &world, &mem);

    const string sep = strings_premake(", ");
    string hello_world = string_vecs_join(&words, sep, &mem);

    strings_println(&hello_world); // hello, world

    mems_free(&mem, null);
    return 0;
}
```
