#!/usr/bin/env python3
#coding: utf-8

import licant
from licant.cxx_modules import application
from licant.libs import include
import os

licant.libs.include("owl")
licant.libs.include("crow")

application("crowker", 
	sources = ["main.cpp", "brocker.cpp"],
	mdepends = [
		"crow", 
		"crow.udpgate"
	]
)

@licant.routine(deps=["crowker"])
def install():
	os.system("cp crowker /usr/local/bin")

licant.ex("crowker")