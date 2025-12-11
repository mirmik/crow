#!/usr/bin/env python3
# coding: utf-8

import sys
import os

# Add build directory to path for testing
build_dir = os.path.join(os.path.dirname(os.path.dirname(__file__)),
                         'build', 'lib.linux-x86_64-cpython-310')
if os.path.exists(build_dir):
    sys.path.insert(0, build_dir)
