#!/usr/bin/env python3
#coding: utf-8

import licant
from licant.cxx_modules import application
from licant.libs import include

licant.libs.include("crow")

licant.cxx_application("target", 
	sources = ["main.cpp"],
	mdepends = [
		("crow"),
		
		("crow.allocator", "malloc"),
		("crow.time", "chrono"),
	],
	cxx_flags = "",
	libs=["igris", "nos", "pthread"]
)

licant.ex("target")