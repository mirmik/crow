#!/usr/bin/env python3
#coding: utf-8

import licant
import licant.install
import shutil
import os

licant.include("crow", "crow.g.py")

target = "libcrow.so"
install_include_path = '/usr/local/include/crow' 
install_directory_path = '/usr/lib/'
install_library_path = os.path.join(install_directory_path, target) 
install_library_link = os.path.join(install_directory_path, 'libcrow.so')

licant.cxx_shared_library(target,
	mdepends = 
	[
		"crow",
		"crow.crowker",
		"crow.udpgate",
		"crow.serial_gstuff"
	],

	cxx_flags = '-fPIC -Wall',
	cc_flags = '-fPIC -Wall',
	libs=["igris"]
)

licant.install.install_library(tgt="install", libtgt=target, headers="crow", hroot="crow", 
	uninstall="uninstall")

licant.ex(target)