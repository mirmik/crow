import licant.modules

licant.modules.module("crow",
	srcdir = "crow/src",
	sources = [
		"tower.cpp", 
		"packet.cpp",
		"node.cpp",
		"channel.cpp"
	],
	include_paths=["."]
)

licant.modules.module("crow.allocator", "malloc",
	sources = [
		"crow/src/malloc_allocator.cpp"
	]
)

licant.modules.module("crow.udpgate",
	sources = [
		"crow/src/udpgate.cpp"
	]
)

licant.modules.module("crow.time", "chrono",
	sources = [
		"crow/src/stdtime.cpp"
	]
)

#licant.modules.module("crow.node",
#	sources = [
#		"crow/src/node.cpp"
#	]
#)

#licant.modules.module("crow.channel",
#	sources = [
#		"crow/src/node.cpp"
#	]
#)