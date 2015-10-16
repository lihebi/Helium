

# Thread to validate
* the error message by compiler is not precise, i.e. may not be the root cause
* error message may be much more, e.g. 1 error, but compiler complains about more than 20 errors.
* The number of errors of each type is not necessary the frequency. e.g. redefine a enum, every enum field will count for an error. For the 10 errors below, it is actually the same enum with 7 fields

# data
```sh
head -1000 compile.txt | grep error (total 820000)
```

types of errors(total 248):

* `unknown type name`(32)
  - TODO grep all type names, see the detail component
* `expected .* before .*` (47)
  - `expected specifier-qualifier-list before 'XXX'` (5)
   + XXX is a type, perhaps used in a struct { ... } definition.
   + but XXX is not defined(perhaps miss a header file, or forward declaration)
 - `expected ‘\}’ before` (5)
 - `expected expression before ‘)’ token` (10)
 - `expected ‘=’, ‘,’, ‘;’, ‘asm’ or ‘__attribute__’ before ‘A’`(27)
* `'xxx' undeclared \(first use in this function\)` (32)
fail to resolve type, or init of that type is unknown
* `redeclaration of enumerator` (7)
* `initializer element is not constant` (50)
but they are in the same compilation error
This appears in support.h file, not known why the init in existing functions have such error.

# fixed
unknown type name

* libevent-devel.i686
* avahi-devel.x86_64
* alsa-lib-devel.x86_64
* openssl-devel.x86_64
* glib-devel.x86_64
* SDL-devel.x86_64
* SDL_image-devel.x86_64

enum inside a struct:

```c
struct xxx {
  enum xxx {
    AAA;
    BBB;
  } var1;
}
```

Buildrate for 100 projects: from 25% to 41%
