#!/bin/bash

# do not run this
# instead, load it in ~/.bashrc the following:
# source /path/to/helium_install/setup-shell.sh

export HELIUM_HOME=$HOME/github/helium
export PATH=$HELIUM_HOME/bin:$HELIUM_HOME/scripts:$PATH
export PATH=$HELIUM_HOME/scripts/analyze:$HELIUM_HOME/scripts/hexp:$PATH
export PATH=$HELIUM_HOME/build/bin:$PATH
export ASAN_OPTIONS=detect_leaks=0:detect_stack_use_after_scope=0
