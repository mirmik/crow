import licant.modules
import licant

#licant.module("crow.netkeep_crowker", 
#	sources=["crow/src/netkeep_crowker.cpp"]
#)

licant.module("crow.select", "impl",
	sources=["crow/src/select.cpp"],
	defines = ["CROW_WITHOUT_SELECT_FDS=0"], default=True
)

licant.module("crow.select", "stub",
	defines = ["CROW_WITHOUT_SELECT_FDS=1"]
)

licant.module("crow.diagnostic", "nos",
	sources=["crow/src/variants/print.cpp"],
	default = True
)

licant.module("crow.threads", "linux", default=True,
	sources = ["crow/src/threads-posix.cpp"]
)

licant.modules.module("crow",
	srcdir = "crow",
	sources = [
		"src/tower.cpp",
		"src/packet.cpp",
		"src/variants/warn.cpp",
		"src/packet_ptr.cpp",
		"src/hostaddr.cpp",
		"src/hostaddr_view.cpp",
		"src/hexer.c",
		"src/address.cpp",
		"src/gateway.cpp",
		"src/iter.cpp",

		"nodes/rshell_cli_node.cpp",
	
		"proto/acceptor.cpp",
		"proto/node.cpp",
		"proto/socket.cpp",
		"proto/node-sync.cpp",
		"proto/channel.cpp",
		"proto/msgbox.cpp",
		"proto/channel-sync.cpp",
		
		"pubsub/pubsub.cpp",
		"proto/service.cpp",
		"proto/rpc.cpp",

		"nodes/subscriber_node.cpp",
		"nodes/publisher_node.cpp",
		"nodes/crowker_pubsub_node.cpp",

		"nodes/service_node.cpp",
		"nodes/request_node.cpp",

		"addons/noslogger.cpp",
	],

	mdepends = [
		"crow.allocator",
		"crow.time",
		"crow.include",
		"crow.threads",
		"crow.diagnostic",
		"crow.select",
		"crow.crowker",

		"crow.chardev_gateway"

		#"crow.crowker.service_node"
	]
)

licant.module("crow.crowker",
	srcdir = "crow",
	sources= [
		"pubsub/crowker_support.cpp",
		"brocker/crowker.cpp",
		"brocker/theme.cpp",
		"brocker/client.cpp",
		"brocker/crow_client.cpp",
		"brocker/tcp_client.cpp",
		"brocker/crowker_api.cpp",
	]
)

licant.modules.module("crow.minimal",
	srcdir = "crow",
	sources = [
		"src/packet.cpp",
		"src/variants/warn.cpp",
		"src/packet_ptr.cpp",
		"src/tower.cpp",
		"src/gateway.cpp",
		"src/hexer.c",
		"src/hostaddr_view.cpp",

		"proto/node.cpp",
		"proto/node-sync.cpp",
		"proto/channel.cpp",

		"nodes/rshell_cli_node.cpp",
	],
	mdepends = ["crow.include", "crow.diagnostic"]
)

licant.modules.module("crow.diagnostic", "debug",
	sources=["crow/src/variants/print-debug.cpp"]
)

licant.modules.module("crow.include", 
	include_paths=["."])

licant.modules.module("crow.allocator", "malloc", 
	sources=[ "crow/src/variants/allocation_malloc.cpp"], default=True)

licant.modules.module("crow.allocator", "pool", 
	sources=["crow/src/variants/allocation_pool.cpp"])

licant.modules.module("crow.time", "chrono", 
	sources=["crow/src/variants/stdtime.cpp"], default=True)

#######################################GATES#########################################

licant.modules.module("crow.udpgate", 
	sources=["crow/gates/udpgate.cpp"])

licant.modules.module("crow.serial_gstuff", 
	sources=[
		"crow/gates/serial_gstuff.cpp",
		"crow/gates/serial_gstuff_v1.cpp"
	], 
	mdepends=[
		#"igris.protocols.gstuff"
	])

licant.module("crow.chardev_gateway", 
	sources = [
		"crow/gates/chardev_gateway.cpp",
		"crow/gates/chardev/chardev_protocol.cpp",
	]
)

#####################################################################################
####################################PROTOCOLS########################################

licant.module("crow.protocol.pubsub",
	sources = ["crow/pubsub/pubsub.cpp"]
)

#####################################################################################

licant.module("crow.crowker.service_node", sources=["crow/brocker/service.cpp"])
