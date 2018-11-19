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
		"pubsub.c",
		"print.cpp",
	],
	include_paths=["."]
)

licant.modules.module("crow.minimal",
	srcdir = "crow/src",
	sources = [
		"packet.cpp",
		"print.cpp",
		"tower.cpp"
	],
	include_paths=["."]
)

licant.modules.module("crow.allocator", "malloc",
	sources = [
		"crow/src/allocation_malloc.cpp"
	]
)

licant.modules.module("crow.udpgate",
	sources = [
		"crow/gates/udpgate.cpp"
	]
)

licant.modules.module("crow.serial_gstuff",
	sources = [
		"crow/gates/serial_gstuff.cpp"
	]
)

licant.modules.module("crow.time", "chrono",
	sources = [
		"crow/src/stdtime.cpp"
	]
)