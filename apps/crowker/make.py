#!/usr/bin/env python3
#coding: utf-8

import licant
import licant.install
from licant.cxx_modules import application
from licant.libs import include
import os

licant.include("crow")

application("crowker", 
	sources = [
		"main.cpp",
		"control_node.cpp"
	],
	mdepends= [
		"crow", 
		"crow.crowker", 
		"crow.udpgate"
	],
	optimize = "-O3",
	cxx_flags = "-flto -Wextra -Wall",
	libs = ["pthread", "readline", "nos", "igris"]
)

licant.install.install_application(tgt="install_crowker", src="crowker", dst="crowker")

if __name__ == "__main__":
	licant.install.install_application(tgt="install", src="crowker", dst="crowker")
	licant.ex("crowker")