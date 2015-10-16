# Simplify code

* slice for segment on generated code
* based on segment, limit function call level
* remove IO stmt and functions
* assess a score with functions(or statements):
if the function is mainly logging something, we can discard it.
* replace function with statements:
Within the function, we just care about return,assert,if statements.

TODO

* GNU projects (382)
