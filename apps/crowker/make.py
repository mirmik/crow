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
]

mdepends = [
	"crow",
	"crow.netkeep_crowker",
	("igris.ctrobj", "linux")
]
mdepends.extend(gates)

application("crowker", 
	sources = [
		"main.cpp",
		"brocker.cpp"
	],
	mdepends = mdepends,
	cxx_flags = "-Wextra -Wall",
	libs = ["pthread", "readline"]
)

@licant.routine(deps = ["crowker"])
def install():
	os.system("cp crowker /usr/local/bin")

licant.ex("crowker")