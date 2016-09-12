#!/bin/bash

# This is intend to set up necessary environment for Helium to run.
# you need to source this file in order to use in the current shell
# everytime you open a new shell, you need to go to this folder to run this file, if you want this env to be in effect.
echo "Make sure you are doing this in the Helium root directory."
echo "Setting environment .."

##############################
## Clear old settings
##############################
echo "clearning old config file ..."
if [ -f ~/.bashrc.local ]; then
    cat ~/.bashrc.local | awk '!/HELIUM|ASAN_OPTIONS/ {print}' > ~/.bashrc.local
fi


##############################
## Setting
##############################
# First, set HELIUM_HOME to current environment
echo "export HELIUM_HOME=$(pwd)" >> ~/.bashrc.local
# export HELIUM_HOME=`pwd`

# add $HELIUM_HOME/bin to the begginning of $PATH, so that, no need to install, `heluim` command will automatically redirect to the correct path.
# export PATH=$HELIUM_HOME/bin:$PATH
echo "export PATH=\$HELIUM_HOME/bin:\$HELIUM_HOME/scripts:\$PATH" >> ~/.bashrc.local
echo "export ASAN_OPTIONS=detect_leaks=0:detect_stack_use_after_scope=0" >> ~/.bashrc.local
echo "source ~/.bashrc.local" >> ~/.bashrc

##############################
## Ending
##############################
echo "Insert configuration into ~/.bashrc.local"
