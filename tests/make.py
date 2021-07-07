#!/usr/bin/env python3

import licant
from licant.cxx_modules import application
from licant.modules import submodule, module
from licant.libs import include

licant.include("crow")
#licant.include("linalg")
#licant.include("igris")

tests_c = [
]

application("runtests",
	sources = [
		"*.cpp",
	],
	mdepends=["crow", "crow.udpgate"],

	ld_flags = "-L/usr/local/lib/",

	include_paths = ["."],
	libs = ["igris", "nos", "pthread"],
)

licant.ex("runtests")