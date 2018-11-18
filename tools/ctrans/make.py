#!/usr/bin/env python3
#coding: utf-8

import licant
from licant.cxx_modules import application
from licant.libs import include
import os

licant.libs.include("crow")
licant.libs.include("gxx")

application("ctrans", 
	sources = ["main.cpp"],
	include_modules = [
		("crow"),
		("crow.allocator", "malloc"),
		("crow.time", "chrono"),
	
		("crow.udpgate"),
		("crow.serial_gstuff"),
		
		("gxx", "posix"),
		("gxx.inet", "posix"),
		("gxx.print", "cout"),
		("gxx.dprint", "cout"),
		("gxx.syslock", "mutex"),
		("gxx.serial"),
	],
	#cxx_flags = "-Werror=all -Werror=extra -pedantic-errors -Werror=shadow -Werror=format=2 -Werror=float-equal -Werror=conversion -Werror=logical-op -Werror=shift-overflow=2 -Werror=duplicated-cond -Wno-cast-qual -Werror=cast-align -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_FORTIFY_SOURCE=2 -fsanitize=address -fsanitize=undefined -fno-sanitize-recover -fstack-protector",
	#cxx_flags = "-Werror=conversion",
	libs = ["pthread", "readline"]
)

@licant.routine(deps=["ctrans"])
def install():
	os.system("cp ctrans /usr/local/bin")

licant.ex("ctrans")