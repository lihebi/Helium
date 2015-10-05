
Define a macro for a typedef ...

```c
// lighttpd
#define ARRAY_STATIC_DEF(name, type, extra) \
typedef struct { \
	type **ptr; \
	size_t used; \
	size_t size; \
	extra\
} name
```
