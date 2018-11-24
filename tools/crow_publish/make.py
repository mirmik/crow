#!/usr/bin/env python3
#coding: utf-8

import licant
from licant.cxx_modules import application
from licant.libs import include
import os

licant.libs.include("gxx")
licant.libs.include("crow")

application("crow_publish", 
	sources = ["main.cpp"],
	include_modules = [
		("crow.minimal"),
		("crow.minimal_pubsub"),
		("crow.allocator", "malloc"),
		("crow.time", "chrono"),
		
		("crow.udpgate"),
		
		("gxx", "posix"),
		("gxx.trent"),
		("gxx.inet", "posix"),
		("gxx.print", "cout"),
		("gxx.dprint", "cout"),
		("gxx.syslock", "mutex"),
	],	
	cxx_flags = "-Werror=all -Werror=extra -pedantic-errors -Werror=shadow -Werror=format=2 -Werror=conversion -Werror=logical-op -Werror=shift-overflow=2 -Werror=duplicated-cond -Wno-cast-qual -Werror=cast-align",

	cc_flags = "-pedantic",
)

@licant.routine(deps=["crow_publish"])
def install():
	os.system("cp crow_publish /usr/local/bin")

licant.ex("crow_publish")