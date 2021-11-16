#!/usr/bin/env python3
#coding: utf-8

import licant
import licant.install
import shutil
import os

licant.include("crow", "crow.g.py")

licant.execute("apps/crowker/make.py")
licant.execute("apps/ctrans/make.py")
licant.execute("apps/crowcalc/make.py")
licant.execute("apps/crowrequest/make.py")

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

	cxxstd="c++17",
	optimize = "-O2",
	cxx_flags = '-fPIC -Wall -pedantic',
	cc_flags = '-fPIC -Wall -pedantic',
	libs=["igris", "nos"],
	defines = ["CROW_PUBSUB_PROTOCOL_SUPPORTED=1"]
)

licant.fileset("apps", targets=[
	"ctrans",
	"crowker",
	"crowcalc",
	"crowrequest"
], deps=["libcrow.so"])

licant.fileset("all", targets=["apps", target])

licant.install.install_library(tgt="install_library", libtgt=target, headers="crow", hroot="crow", 
	uninstall="uninstall")

@licant.routine(deps=["apps"])
def install_apps():
	licant.do(["install_crowker", "makefile"])
	licant.do(["install_ctrans", "makefile"])
	licant.do(["install_crowcalc", "makefile"])
	licant.do(["install_crowrequest", "makefile"])

@licant.routine(deps=["install_apps", "install_library"])
def install():
	pass

licant.ex("all")