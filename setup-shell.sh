#!/bin/bash

# add load to ~/.bashrc
echo "export HELIUM_HOME=$(pwd)" >> ~/.bashrc
echo "source $(pwd)/setup-shell-load.sh" >> ~/.bashrc
