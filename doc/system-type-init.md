System types usually use functions to init.
These functions may take the true parameters from outside.
We don't provide the value for every fields for a system type,
but rather identify which is the important part(outside parameter),
provide these as input,
and call the function sequence.

## Examples

### regex_t

```c
regcomp(regex_t*, const char* pattern, int flag);
regexec(regex_t*, const char* s, size_t nmatch, regmatch_t pmatch[], int eflags);
regerror();
regfree();
```

The regexec will also fill regmatch_t, so we do not need to fill it.
When output, we need to identify which part of the type is to be output.
We may need to call some function for output purpose.

We do not need to care about the `free` function,
because when the program terminates,
resource will be freed.
However, this matters because when we mine big code for the init statements,
we need to filter out the free statements, because we don't need that,
and we cannot use the variable after it is freed.
