import licant.modules

licant.modules.module("crow",
	srcdir = "crow/src",
	sources = [
		"tower.cpp", 
		"packet.cpp",
		"node.cpp",
		"channel.cpp",
		"pubsub.cpp",
		"print.cpp",
		"hexer.c",
	],

	mdepends = [
		"crow.allocator",
		"crow.time",
		"crow.include",
		"igris.include",
		"igris.syslock",
		"igris.dprint",
		"nos"
	]
)

licant.modules.module("crow.minimal",
	srcdir = "crow/src",
	sources = [
		"packet.cpp",
		"print.cpp",
		"tower.cpp"
	],
	mdepends = ["crow.include"]
)

licant.modules.module("crow.include", 
	include_paths=["."])

licant.modules.module("crow.allocator", "malloc", 
	sources=[ "crow/src/allocation_malloc.cpp"], default=True)

licant.modules.module("crow.allocator", "pool", 
	sources=["crow/src/allocation_pool.cpp"])

licant.modules.module("crow.time", "chrono", 
	sources=["crow/src/stdtime.cpp"], default=True)

#######################################GATES#########################################

licant.modules.module("crow.udpgate", 
	sources=["crow/gates/udpgate.cpp"])

licant.modules.module("crow.serial_gstuff", 
	sources=["crow/gates/serial_gstuff.cpp"], 
	mdepends=["igris.protocols.gstuff"])

#####################################################################################