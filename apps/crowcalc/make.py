#!/usr/bin/env python3
#coding: utf-8

import licant
import licant.install
from licant.cxx_modules import application
from licant.libs import include
import os

licant.libs.include("crow")

defines = ["NOTRACE=1"]

application("crowcalc", 
	sources = [
		"main.cpp"
	],
	mdepends=["crow", "crow.udpgate"],
	defines = defines,
	cxx_flags = "-Wextra -Wall -g",
	cc_flags = "-Wextra -Wall -g",
	libs = ["pthread", "readline", "igris", "nos"],
	cxxstd = "c++17"
)

licant.install.install_application(tgt="install_crowcalc", src="crowcalc", dst="crowcalc")

if __name__ == "__main__":
	licant.install.install_application(tgt="install", src="crowcalc", dst="crowcalc")
	licant.ex("crowcalc")