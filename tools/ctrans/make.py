#!/usr/bin/env python3
#coding: utf-8

import licant
from licant.cxx_modules import application
from licant.libs import include
import os

licant.libs.include("crow")
licant.libs.include("gxx")

application("ctrans", 
	sources = ["main.c"],
	include_modules = [
		("crow"),
		("crow.allocator", "malloc"),
		("crow.time", "chrono"),
	
		("crow.udpgate"),
#		("crow.serialgate"),
		
		("gxx", "posix"),
		("gxx.inet", "posix"),
		("gxx.print", "cout"),
		("gxx.dprint", "cout"),
		("gxx.syslock", "mutex"),
		("gxx.serial"),
	],
	cxx_flags = "",
	libs = ["pthread", "readline"]
)

@licant.routine
def install():
	licant.do("ctrans")
	os.system("cp ctrans /usr/local/bin")

licant.ex("ctrans")