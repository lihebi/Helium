# Helium user interface

Helium is a tool to analyze dynamic features of segment of program.

Helium is run against a folder containing the benchmark.
Helium works by go through all files in the benchmark,
extract segment of interests,
build environment necessary for it the segment, make it compilable,
then generate tests.

So the library API should provide:
* get_source_files(folder)
* get_segments(file)

## library API

Reader(file)
