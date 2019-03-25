import licant.modules
licant.include("igris")
licant.include("nos")

licant.modules.module("crow",
	srcdir = "crow/src",
	sources = [
		"tower.cpp", 
		"packet.cpp",
		"node.cpp",
		"channel.cpp",
		"pubsub.cpp",
		"print.cpp",
	],

	include_paths=["."],

	mdepends = [
		"crow.allocator",
		"crow.time",
		"igris.include",
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
	include_paths=["."]
)

licant.modules.module("crow.include", include_paths=["."])

licant.modules.module("crow.allocator", "malloc", sources=[ "crow/src/allocation_malloc.cpp"], default=True)
licant.modules.module("crow.allocator", "pool", sources=["crow/src/allocation_pool.cpp"])

licant.modules.module("crow.udpgate", sources=["crow/gates/udpgate.cpp"])
licant.modules.module("crow.serial_gstuff", sources=["crow/gates/serial_gstuff.cpp"])

licant.modules.module("crow.time", "chrono", sources=["crow/src/stdtime.cpp"], default=True)

#licant.modules.module("crow.minimal_pubsub",
#	srcdir = "crow/src",
#	sources = [
#		"pubsub.cpp"
#	],
#	include_paths=["."]
#)
#
#licant.modules.module("crow.minimal_node",
#	srcdir = "crow/src",
#	sources = [
#		"node.cpp"
#	],
#	include_paths=["."]
#)
#
#licant.modules.module("crow.minimal_channel",
#	srcdir = "crow/src",
#	sources = [
#		"channel.cpp"
#	],
#	include_paths=["."]
#)