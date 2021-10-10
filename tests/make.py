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
		"brocker/*.cpp"
	],
	mdepends=["crow", "crow.udpgate"],

	cxxstd="c++20",
	ccstd="c11",
	cxx_flags = "-g -Werror=all -Werror=pedantic -Wno-gnu-zero-variadic-macro-arguments",
	cc_flags = "-g -Werror=all -Werror=pedantic -Wno-gnu-zero-variadic-macro-arguments",

	include_paths = ["."],
	libs = ["igris", "nos", "pthread"],
)

licant.ex("runtests")