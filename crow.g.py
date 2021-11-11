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

licant.modules.module("crow.minimal",
	srcdir = "crow",
	sources = [
		"src/packet.cpp",
		"src/variants/warn.cpp",
		"src/packet_ptr.cpp",
		"src/tower.cpp",
		"src/gateway.cpp",
		"src/hexer.c",
		"src/address.cpp",
		"src/hostaddr_view.cpp",
		"src/hostaddr.cpp",
		"proto/node.cpp",
		"proto/node-sync.cpp",
		"proto/channel.cpp",
		"nodes/rshell_cli_node.cpp",
		"nodes/subscriber_node.cpp",
		"nodes/publisher_node.cpp",
		"nodes/crowker_pubsub_node.cpp",
		"nodes/service_node.cpp",
		"nodes/request_node.cpp",
	],
	mdepends = ["crow.include", "crow.diagnostic"]
)

licant.modules.module("crow",
	srcdir = "crow",
	sources = [
		"src/iter.cpp",
		"proto/acceptor.cpp",
		"proto/socket.cpp",
		"proto/msgbox.cpp",
		"proto/channel-sync.cpp",		
		"proto/rpc.cpp",
		"addons/noslogger.cpp",
	],

	mdepends = [
		"crow.allocator",
		"crow.time",
		"crow.threads",
		"crow.select",
		"crow.crowker",
		"crow.chardev_gateway",
		"crow.minimal",
	#	"crow.protocol.pubsub",
	]
)

licant.module("crow.crowker",
	srcdir = "crow",
	sources= [
		"brocker/crowker.cpp",
		"brocker/theme.cpp",
		"brocker/client.cpp",
		"brocker/crow_client.cpp",
		"brocker/tcp_client.cpp",
		"brocker/crowker_api.cpp",
	]
)

licant.modules.module("crow.diagnostic", "debug",
	sources=["crow/src/variants/print-debug.cpp"]
)

licant.modules.module("crow.diagnostic", "stub",
	sources=["crow/src/variants/print-stub.cpp"]
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
		"crow/gates/serial_gstuff.cpp"
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

#licant.module("crow.protocol.pubsub", "impl", default=True,
#	sources = ["crow/pubsub/pubsub.cpp"]
#)

#licant.module("crow.protocol.pubsub", "stub",
#	defines = ["WITHOUT_CROW_PUBSUB=1"]
#)

#####################################################################################

licant.module("crow.crowker.service_node", sources=["crow/brocker/service.cpp"])
