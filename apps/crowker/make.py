#!/usr/bin/env python3
#coding: utf-8

import licant
import licant.install
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

licant.install.install_application(tgt="install_crowker", src="crowker", dst="crowker")

if __name__ == "__main__":
	licant.install.install_application(tgt="install", src="crowker", dst="crowker")
	licant.ex("crowker")