#!/usr/bin/env python3
#coding: utf-8

import licant
from licant.cxx_modules import application
from licant.libs import include
import os

licant.libs.include("gxx")
licant.libs.include("crow")

application("crowker", 
	sources = ["main.cpp", "brocker.cpp"],
	include_modules = [
		("crow.minimal"),
		("crow.minimal_pubsub"),
		("crow.allocator", "malloc"),
		("crow.time", "chrono"),
		
		("crow.udpgate"),
		
		("gxx", "posix"),
#		("gxx.inet", "posix"),
		("gxx.print", "cout"),
		("gxx.dprint", "cout"),
		("gxx.syslock", "mutex"),
	],
	cxx_flags = ""
)

@licant.routine(deps=["crowker"])
def install():
	os.system("cp crowker /usr/local/bin")

licant.ex("crowker")