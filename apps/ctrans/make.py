#!/usr/bin/env python3
#coding: utf-8

import licant
import licant.install
from licant.cxx_modules import application
from licant.libs import include
import os

defines = ["NOTRACE=1"]

application("ctrans", 
	sources = [
		"main.cpp"
	],
	mdepends= [
		"crow", 
		"crow.crowker", 
		"crow.udpgate", 
		"crow.serial_gstuff",
	],
	defines = defines,
	cxx_flags = "-Wextra -Wall",
	libs = ["pthread", "readline", "igris", "nos"],
	cxxstd = "c++17"
)


@licant.routine
def doc():
	os.system("cd doc; ./make.sh")

licant.install.install_application(tgt="install_ctrans", src="ctrans", dst="ctrans")
	
if __name__ == "__main__":
	licant.install.install_application(tgt="install", src="ctrans", dst="ctrans")
	licant.ex("ctrans")