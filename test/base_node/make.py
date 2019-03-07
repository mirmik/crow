#!/usr/bin/env python3
#coding: utf-8

import licant
import licant.libs
from licant.cxx_modules import application
from licant.modules import submodule

licant.libs.include("owl")
licant.libs.include("crow")

application("target",
	sources = ["main.cpp"],
	include_paths = ["../.."],
	include_modules = [
		("owl", "posix"),
		("owl.print", "cout"),
		("owl.dprint", "cout"),
		("owl.syslock", "mutex"),

		("crow.minimal"),
		("crow.minimal_node"),
		("crow.udpgate"),
		("crow.serial_gstuff"),
		("crow.allocator", "malloc"),
		("crow.time", "chrono"),
		
	],
)

licant.ex("target")