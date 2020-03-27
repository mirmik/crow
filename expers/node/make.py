#!/usr/bin/env python3
#coding: utf-8

import licant
from licant.cxx_modules import application
from licant.libs import include

licant.libs.include("crow")
licant.libs.include("igris")
licant.libs.include("nos")

application("target", 
	sources = ["main.cpp"],
	mdepends = [
		("crow.minimal"),
		
		("crow.allocator", "malloc"),
		("crow.time", "chrono"),

		"igris", "nos"
	],
	cxx_flags = "",
)

licant.ex("target")