#!/usr/bin/env python3
#coding: utf-8

import licant

licant.include("gxx")

licant.cxx_application("target",
	sources = ["main.cpp"],
	libs = ["crow", "gxx"],
)

licant.ex("target")