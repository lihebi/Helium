#!/bin/bash

# globally replace a string with a pattern.

src="Config::Instance()->GetString"
replace="HEBI"

find . -name *.cc -exec sed -ne "s/$src/$replace/p" {} \;
