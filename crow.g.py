import licant.modules

licant.modules.module("crow.include",
	include_paths=["."]
)

licant.modules.module("crow",
	srcdir = "crow/src",
	sources = [
		"tower.c", 
		"packet.c",
		"node.c",
		"channel.c",
		"pubsub.c"
	],
	include_paths=["."]
)

licant.modules.module("crow.allocator", "malloc",
	sources = [
		"crow/src/allocation_malloc.c"
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