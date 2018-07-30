#!/usr/bin/env python3
#coding: utf-8

import licant
from licant.cxx_modules import application
from licant.libs import include
import os

licant.libs.include("gxx")
licant.libs.include("crow")

application("crowker", 
	sources = ["main.cpp"],
	include_modules = [
		("crow"),
		("crow.allocator", "malloc"),
		("crow.time", "chrono"),
		("crow.pubsub", "brocker"),
		
		("gxx", "posix"),
		("gxx.inet", "posix"),
		("gxx.print", "cout"),
		("gxx.dprint", "cout"),
		("gxx.syslock", "mutex"),
	],
	cxx_flags = ""
)

@licant.routine
def install():
	licant.do("crowker")
	os.system("cp crowker /usr/local/bin")

licant.ex("crowker")