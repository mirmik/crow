#!/usr/bin/env python3
#coding: utf-8

import licant
import licant.libs
from licant.cxx_modules import application
from licant.modules import submodule

licant.libs.include("gxx")
licant.libs.include("crow")

application("target",
	sources = ["main.cpp"],
	include_paths = ["../.."],
	include_modules = [
		("gxx", "posix"),
		("gxx.print", "cout"),
		("gxx.dprint", "cout"),
		("gxx.log2", "impl"),
		("gxx.syslock", "mutex"),

		("crow"),
		("crow.allocator", "malloc"),
		("crow.time", "chrono"),
		
	],
)

licant.ex("target")