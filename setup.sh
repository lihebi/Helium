#!/bin/bash


if [ -d $HOME/.helium.d ]; then
    # echo "$HOME/.helium.d exist. Remove to continue."
    rm -r $HOME/.helium.d
    exit 1
fi

# check if the helium configuration is already there
# if [[ -n `cat $HOME/.bashrc | grep "Helium Configuration"` ]]; then
#     echo "Configuration already exists in current ~/.bashrc. Remove it to continue."
#     exit 1
# fi

mkdir -p $HOME/.helium.d

ln -sf `pwd`/helium.conf $HOME/.helium.d/helium.conf
ln -sf `pwd`/etc $HOME/.helium.d/


# add load to ~/.bashrc
echo "" >> $HOME/.bashrc
echo "## ========== Helium Configuration" >> $HOME/.bashrc
echo "" >> $HOME/.bahsrc
echo "export HELIUM_HOME=$(pwd)" >> $HOME/.bashrc


echo 'export PATH=$HELIUM_HOME/bin:$HELIUM_HOME/scripts:$PATH' >> $HOME/.bashrc
echo 'export PATH=$HELIUM_HOME/scripts/analyze::$HELIUM_HOME/scripts/hexp:$PATH' >> ~/.bashrc
echo 'export PATH=$HELIUM_HOME/scripts/experiment:$PATH' >> $HOME/.bashrc
echo 'export PATH=$HELIUM_HOME/scripts/pyhelium:$PATH' >> $HOME/.bashrc
echo 'export PATH=$HELIUM_HOME/lib/RCFReader:$HELIUM_HOME/lib/iclones:$PATH' >> $HOME/.bashrc
echo 'export PATH=$HELIUM_HOME/build/bin:$PATH' >> $HOME/.bashrc
echo 'export ASAN_OPTIONS=detect_leaks=0:detect_stack_use_after_scope=0' >> $HOME/.bashrc
echo 'export LIBC_FATAL_STDERR_=1' >> $HOME/.bashrc

echo "## ========== End of Helium Configuration" >> $HOME/.bashrc


echo "Set up completed. Reload $HOME/.bashrc to take effect."
