# Helium

## Prerequisite
* header files for pugixml(>=1.6)
* [shared library for pugixml](doc/document/pugixml.md)
* libctags: in ctags release,
  - prepare the `readtags.h`
  - compile shared lib for `readtags.c` into `libctags.dylib`

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

## run
run `helium` to see help information.

## Common Workflow

```sh
export HELIUM_HOME=/path/to/Helium
make systype.tags # create system type tag file. Need only once on a new platform
cp helium.sample.xml helium.xml # and then change accordingly(especially interact option)
```

```sh
helium --pre <folder>
helium <folder>
```

Some scripts:

* `scripts/buildrate.sh`: use against a folder containing many benchmarks. Will output the buildrate. Need to have the correct helium.xml config file.


## separate topics
* [env](doc/document/env.md)
* [preprocess](doc/document/preprocess.md)
* [code selection](doc/document/segment_code_selection.md)
* [configuration](doc/document/configuration.md)
