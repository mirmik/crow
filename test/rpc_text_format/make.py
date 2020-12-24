#!/usr/bin/env python3
#coding: utf-8

import licant
from licant.cxx_modules import application
from licant.libs import include

licant.libs.include("crow")

licant.cxx_application("rpc", 
	sources = ["rpc.cpp"],
	mdepends = [
		("crow"),
		
		("crow.allocator", "malloc"),
		("crow.time", "chrono"),
		"crow.udpgate"
	],
	cxx_flags = "",
	libs=["igris", "nos", "pthread"]
)

licant.cxx_application("request", 
	sources = ["request.cpp"],
	mdepends = [
		("crow"),
		
		("crow.allocator", "malloc"),
		("crow.time", "chrono"),
		"crow.udpgate"
	],
	cxx_flags = "",
	libs=["igris", "nos", "pthread"]
)

licant.ex("rpc")
licant.ex("request")