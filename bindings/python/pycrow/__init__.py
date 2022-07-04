try:
	import pycrow.libcrow
	from pycrow.libcrow import *
except Exception as ex:
	print("PyCrow: Import libs error. ex:", ex)
	exit(0)

def start_client(port = 0):
	"""
		@port - udp port. 0 for dynamic choise
	"""
	pycrow.libcrow.create_udpgate(12, port)
	pycrow.libcrow.start_spin()

def stop_client():
	pycrow.libcrow.stop_spin()

PUBLISHERS = {}

def get_publisher(theme):
	global PUBLISHERS
	if theme in PUBLISHERS:
		return PUBLISHERS[theme]
	crowker = pycrow.libcrow.crowker_address()
	PUBLISHERS[theme] = pycrow.libcrow.publisher(crowker, theme)
	return PUBLISHERS[theme]

def publish(theme, bindata):
	pub = get_publisher(theme)
	pub.publish(bindata)