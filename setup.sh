#!/bin/bash

SCRIPT=$(readlink -f "$0")
DIR=$(dirname "$SCRIPT")

ln -sf $DIR/helium.conf $HOME/.heliumrc

echo "" >> $HOME/.bashrc
echo "## ========== Helium Configuration" >> $HOME/.bashrc
echo "" >> $HOME/.bahsrc
echo "export HELIUM_HOME=$DIR" >> $HOME/.bashrc
echo "export PATH=$HELIUM_HOME/bin:$HELIUM_HOME/scripts:$PATH" >> $HOME/.bashrc
echo "export ASAN_OPTIONS=detect_leaks=0:detect_stack_use_after_scope=0" >> $HOME/.bashrc

echo "## ========== End of Helium Configuration" >> $HOME/.bashrc

echo "Set up completed. Reload $HOME/.bashrc to take effect."
