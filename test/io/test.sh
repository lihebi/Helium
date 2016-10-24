#!/bin/bash


for code in "char var;" "char *var;"; do
    helium --show-instrument-code $code > char-var.output.c
    diff char-var.output.c char-var.sample.c >/dev/null
    if [[ ! $? == 0 ]]; then
        echo "char var; ... Failed"
    fi
done


helium --show-instrument-code "char* var;" > char-star-var.output.c
diff char-star-var.sample.c char-star-var.output.c >/dev/null

if [[ ! $? == 0 ]]; then
    echo "char *var; .... Failed"
fi

# helium --show-instrument-code "char var[]"


# helium --show-instrument-code "char **argv"


