# compile

## Prerequisite
* header files for pugixml(>=1.6)
* [shared library for pugixml](doc/document/pugixml.md)
* libctags: in ctags release,
  - prepare the `readtags.h`
  - compile shared lib for `readtags.c` into `libctags.dylib`
* boost

## compile
compile ctags and pugixml dynamic library:

```sh
cd lib
cd ctags-<tab>
make
make install
cd ../pugixml-<tab>
make
make install
```

compile Helium

```sh
make
make install
```
