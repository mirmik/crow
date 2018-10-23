#!/usr/bin/env python3
#coding: utf-8

import licant
import licant.libs
from licant.cxx_modules import application
from licant.modules import submodule

licant.libs.include("gxx")
licant.libs.include("crow")

application("target",
	sources = ["main.c", 
		"../../crow/src/packet.c",
		"../../crow/src/allocation_malloc.c",
		"../../crow/src/tower.c",
		"../../crow/src/node.c",
		"../../crow/src/pubsub.c",
		"../../crow/src/stdtime.cpp",
	],
	include_paths = ["../.."],
	include_modules = [
		("gxx", "posix"),
		("gxx.print", "cout"),
		("gxx.dprint", "cout"),
		("gxx.syslock", "mutex"),

		("crow.include"),
		#("crow.allocator", "malloc"),
		#("crow.time", "chrono"),
		
	],
)

licant.ex("target", colorwrap = True)