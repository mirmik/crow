#!/usr/bin/env python3
#coding: utf-8

import licant
from licant.cxx_modules import application
from licant.libs import include
import os

licant.libs.include("crow")

licant.libs.include("nos")
licant.libs.include("igris")

gates = [
	"crow.udpgate",
	"crow.serial_gstuff",
	"igris.protocols.msgtype"
]

mdepends = ["crow", ("igris.ctrobj", "linux")]
mdepends.extend(gates)

application("ctrans", 
	sources = [
		"main.cpp",
		"binin.cpp",
		"binout.cpp",
		"bincommon.cpp",
	],
	defines = ["NOTRACE=1"],
	mdepends = mdepends,
	cxx_flags = "-Wextra -Wall",
	libs = ["pthread", "readline"]
)

@licant.routine(deps = ["ctrans"])
def install():
	os.system("cp ctrans /usr/local/bin")

licant.ex("ctrans")