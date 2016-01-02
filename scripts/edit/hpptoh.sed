#!/usr/bin/env sed -nf

# Now I want to rename all .hpp files to .h files
# all #include "xxx.hpp" should be mapped to "xxx.h"
# #ifndef __XXX_HPP__ should be mapped to __XXX_H__
# #define __XXX_HPP__ should be mapped to __XXX_H__

