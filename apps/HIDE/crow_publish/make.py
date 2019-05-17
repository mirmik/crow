#!/usr/bin/env python3
#coding: utf-8

import licant
from licant.cxx_modules import application
from licant.libs import include
import os

licant.libs.include("owl")
licant.libs.include("crow")

application("crow_publish", 
	sources = ["main.cpp"],
	mdepends = [
		"crow", 
		"crow.udpgate"
	]
)

@licant.routine(deps=["crow_publish"])
def install():
	os.system("cp crow_publish /usr/local/bin")

licant.ex("crow_publish")