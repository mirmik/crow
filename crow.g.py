import licant.modules

licant.modules.module("crow",
	srcdir = "crow/src",
	sources = [
		"tower.cpp", 
		"packet.cpp",
		"node.cpp",
		"channel.cpp",
		"pubsub.cpp"
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