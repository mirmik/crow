#!/usr/bin/env python3
#coding: utf-8

import os
import shutil
import sys

import licant
import licant.install

licant.include("crow", "crow.g.py")

licant.execute("apps/crowker/make.py")
licant.execute("apps/ctrans/make.py")

target = "libcrow.so"
install_include_path = '/usr/local/include/crow' 
install_directory_path = '/usr/lib/'
install_library_path = os.path.join(install_directory_path, target) 
install_library_link = os.path.join(install_directory_path, 'libcrow.so')

modules = [
		"crow",
		"crow.crowker",
		"crow.udpgate",
		"crow.tcpgate",
		"crow.serial_gstuff",
	]
if sys.platform != "win32":
	modules.extend(["crow.realtime_threads"])

licant.cxx_shared_library(target,
	mdepends = modules,

	cxxstd="c++17",
	optimize = "-O2",
	cxx_flags = '-fPIC -Wall -pedantic -g -Weffc++ -Wextra',
	cc_flags = '-fPIC -Wall -pedantic -g -Wextra',
	libs=["igris", "nos"],
)

licant.fileset("apps", targets=[
	"ctrans",
	"crowker",
], deps=["libcrow.so"])

licant.fileset("all", targets=["apps", target])

licant.install.install_library(tgt="install_library", libtgt=target, headers="crow", hroot="crow", 
	uninstall="uninstall")

@licant.routine(deps=["apps"])
def install_apps():
	licant.do(["install_crowker", "makefile"])
	licant.do(["install_ctrans", "makefile"])

@licant.routine(deps=["install_apps", "install_library"])
def install():
	pass

licant.ex("all")