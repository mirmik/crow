#!/usr/bin/env python3
#coding: utf-8

import licant
from licant.cxx_modules import application
from licant.libs import include
import os

defines = ["NOTRACE=1"]

application("crowpulse", 
	sources = [
		"main.cpp"
	],
	defines = defines,
	cxx_flags = "-Wextra -Wall",
	libs = ["pthread", "readline", "igris", "nos", "crow"],
	cxxstd = "c++17"
)

@licant.routine(deps = ["crowpulse"])
def install():
	os.system("cp crowpulse /usr/local/bin")

licant.ex("crowpulse")