import licant.modules

licant.modules.module("crow",
	sources = [
		"crow/src/tower.cpp", "crow/src/packet.cpp",
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

licant.modules.module("crow.node",
	sources = [
		"crow/src/node.cpp"
	]
)