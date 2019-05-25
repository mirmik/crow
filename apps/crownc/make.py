#!/usr/bin/env python3

import licant

licant.libs.include("igris")
licant.libs.include("nos")
licant.libs.include("crow")

licant.cxx_application("target", 
	sources = [
		"main.cpp"
	],
	mdepends = [
		"crow",
		"crow.udpgate"
	],
	cxx_flags = "-Wextra -Wall",
	libs = ["pthread", "readline"]
)

@licant.routine(deps = ["target"])
def install():
	os.system("cp target /usr/local/bin")

licant.ex("target")