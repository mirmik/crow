#!/usr/bin/env python3
#coding: utf-8

import licant
from licant.cxx_modules import application
from licant.libs import include

licant.libs.include("owl")
licant.libs.include("crow")

application("target", 
	sources = ["main.cpp"],
	include_modules = [
		("crow.minimal"),
		("crow.minimal_node"),
		("crow.minimal_channel"),
		("crow.allocator", "malloc"),
		("crow.time", "chrono"),
#		("crow.node"),
		
		("owl", "posix"),
		("owl.inet", "posix"),
		("owl.print", "cout"),
		("owl.dprint", "cout"),
		("owl.syslock", "mutex"),
	],
	cxx_flags = ""
)

licant.ex("target")