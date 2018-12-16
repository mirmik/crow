#!/usr/bin/env python3
#coding: utf-8

import licant
from licant.cxx_modules import application
from licant.libs import include
import os

licant.libs.include("gxx")
licant.libs.include("crow")

application("crow_subscribe",
	sources = ["main.cpp"],
	mdepends = [
		"crow", 
		"crow.udpgate"
	]
)

@licant.routine(deps=["crow_subscribe"])
def install():
	os.system("cp crow_subscribe /usr/local/bin")

licant.ex("crow_subscribe")