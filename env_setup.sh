#!/bin/bash

# This is intend to set up necessary environment for Helium to run.
# you need to source this file in order to use in the current shell
# everytime you open a new shell, you need to go to this folder to run this file, if you want this env to be in effect.
echo "Make sure you are doing this in the Helium root directory."
echo "Setting environment .."

# First, set HELIUM_HOME to current environment
echo "export HELIUM_HOME=$(pwd)" >> ~/.bashrc.local
# export HELIUM_HOME=`pwd`

# add $HELIUM_HOME/bin to the begginning of $PATH, so that, no need to install, `heluim` command will automatically redirect to the correct path.
# export PATH=$HELIUM_HOME/bin:$PATH
echo "export PATH=\$HELIUM_HOME/bin:\$PATH" >> ~/.bashrc.local

echo "Insert configuration into ~/.bashrc.local"
