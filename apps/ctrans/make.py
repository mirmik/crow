#!/usr/bin/env python3
#coding: utf-8

import licant
from licant.cxx_modules import application
from licant.libs import include
import os

licant.libs.include("crow")

application("ctrans", 
	sources = ["main.cpp"],

	mdepends = [
		"crow",
		"crow.udpgate",
		"crow.serial_gstuff",

		("gxx.print", "__none__")
	],

	#cxx_flags = "-Werror=all -Werror=extra -pedantic-errors -Werror=shadow -Werror=format=2 -Werror=float-equal -Werror=conversion -Werror=logical-op -Werror=shift-overflow=2 -Werror=duplicated-cond -Wno-cast-qual -Werror=cast-align",
	#cxx_flags = "-Werror=conversion",
	libs = ["pthread", "readline"]
)

@licant.routine(deps=["ctrans"])
def install():
	os.system("cp ctrans /usr/local/bin")

licant.ex("ctrans")