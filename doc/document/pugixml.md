# Prepare pugixml library

`libpugi.so`(unix) or `libpugi.dylib`(mac)

On mac:

```sh
g++ -dynamiclib -o libpugi.dylib pugixml.cpp
```

On Unix:

```sh
g++ -Wall -fPIC -c *.c
g++ -shared -Wl,-soname,libpugi.so.1 -o libpugi.so.1.0 *.o
```
