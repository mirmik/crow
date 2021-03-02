#!/usr/bin/env python3
#coding: utf-8

import licant
import licant.install
from licant.cxx_modules import application
from licant.libs import include
import os

defines = ["NOTRACE=1"]

licant.libs.include("crow")

application("crowrequest", 
	sources = [
		"main.cpp"
	],
	mdepends=["crow", "crow.udpgate"],
	defines = defines,
	cxx_flags = "-Wextra -Wall",
	libs = ["pthread", "readline", "igris", "nos"],
	cxxstd = "c++17"
)

licant.install.install_application(tgt="install_crowrequest", src="crowrequest", dst="crowrequest")

if __name__ == "__main__":
	licant.install.install_application(tgt="install", src="crowrequest", dst="crowrequest")
	licant.ex("crowrequest")