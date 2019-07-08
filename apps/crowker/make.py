#!/usr/bin/env python3
#coding: utf-8

import licant
from licant.cxx_modules import application
from licant.libs import include
import os

application("crowker", 
	sources = [
		"main.cpp",
		"brocker.cpp"
	],
	cxx_flags = "-Wextra -Wall",
	libs = ["pthread", "readline", "nos", "igris", "crow"]
)

@licant.routine(deps = ["crowker"])
def install():
	os.system("cp crowker /usr/local/bin")

licant.ex("crowker")