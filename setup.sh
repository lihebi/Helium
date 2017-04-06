#!/bin/bash

mkdir -p $HOME/.helium.d
mkdir -p $HOME/.helium.d/etc
ln -sf `pwd`/helium.conf $HOME/.heliumrc
ln -sf `pwd`/etc/headers.conf.d/system.conf $HOME/.helium.d/etc/
ln -sf `pwd`/etc/headers.conf.d/third-party.conf $HOME/.helium.d/etc/
ln -sf `pwd`/etc/blacklist.conf $HOME/.helium.d/etc
