# Helium

## Prerequisite
* header files for pugixml
* [shared library for pugixml](doc/document/pugixml.md)
* libctags: in ctags release,
  - prepare the `readtags.h`
  - compile shared lib for `readtags.c` into `libctags.dylib`

## compile
```sh
make
make install
```

## run
run `helium` to see help information.

## separate topics
* [env](doc/document/env.md)
* [preprocess](doc/document/preprocess.md)
* [code selection](doc/document/segment_code_selection.md)
* [configuration](doc/document/configuration.md)
