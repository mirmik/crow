#!/usr/bin/env python3
#coding: utf-8

import licant
from licant.cxx_modules import application
from licant.libs import include
import os

licant.libs.include("crow")

licant.libs.include("nos")
licant.libs.include("igris")

with_msgtype = False

mdepends = [
	"crow", 
	"crow.udpgate",
	"crow.serial_gstuff",
	"igris",
	("igris.ctrobj", "linux")
]

if with_msgtype:
	mdepends.append("igris.protocols.msgtype")

defines = ["NOTRACE=1"]

if with_msgtype:
	defines.append("WITH_MSGTYPE=1")

application("ctrans", 
	sources = [
		"main.cpp",
		"binin.cpp",
		"binout.cpp",
		"bincommon.cpp",
	],
	defines = defines,
	mdepends = mdepends,
	cxx_flags = "-Wextra -Wall",
	libs = ["pthread", "readline"],
	cxxstd = "c++17"
)

@licant.routine(deps = ["ctrans", "doc"])
def install():
	os.system("cp ctrans /usr/local/bin")
	os.system("cp doc/crow.1 /usr/local/share/man/man1/")

@licant.routine
def doc():
	os.system("cd doc; ./make.sh")

licant.ex("ctrans")