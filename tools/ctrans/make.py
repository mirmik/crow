#!/usr/bin/env python3
#coding: utf-8

import licant
from licant.cxx_modules import application
from licant.libs import include
import os

licant.libs.include("crow")
licant.libs.include("gxx")

application("ctrans", 
	sources = ["main.cpp"],
	include_modules = [
		("crow"),
		("crow.allocator", "malloc"),
		("crow.time", "chrono"),
		("crow.pubsub", "client"),
		
		("gxx", "posix"),
		("gxx.log2", "impl"),
		("gxx.inet", "posix"),
		("gxx.print", "cout"),
		("gxx.dprint", "cout"),
		("gxx.syslock", "mutex"),
		("gxx.serial"),
	],
	cxx_flags = "",
	libs = ["pthread"]
)

@licant.routine
def install():
	licant.do("ctrans")
	os.system("cp ctrans /usr/local/bin")

licant.ex("ctrans")