try:
	import pycrow.libcrow
	from pycrow.libcrow import *
except Exception as ex:
	print("PyCrow: Import libs error. ex:", ex)
	exit(0)

import reactivex
import reactivex.subject

def start_client(port = 0):
	"""
		@port - udp port. 0 for dynamic choise
	"""
	pycrow.libcrow.create_udpgate(12, port)
	pycrow.libcrow.start_spin()

def stop_client():
	pycrow.libcrow.stop_spin()

PUBLISHERS = {}
SUBSCRIBERS = {}

def get_publisher(theme):
	if theme in PUBLISHERS:
		return PUBLISHERS[theme]
	crowker = pycrow.libcrow.crowker_address()
	PUBLISHERS[theme] = pycrow.libcrow.publisher(crowker, theme)
	return PUBLISHERS[theme]

def get_subscriber(theme, incoming):
	crowker = pycrow.libcrow.crowker_address()
	SUBSCRIBERS[theme] = pycrow.libcrow.subscriber(incoming)
	SUBSCRIBERS[theme].subscribe(crowker, theme)
	return SUBSCRIBERS[theme]

def publish(theme, bindata):
	pub = get_publisher(theme)
	pub.publish(bindata)

def subscribe(theme, incoming, keepalive=2000):
	sub = get_subscriber(theme, incoming)
	sub.install_keepalive(keepalive, False)

def rxsubscribe(theme, keepalive=2000):
	subject = reactivex.subject.Subject()	
	subscribe(theme, lambda x: subject.on_next(x), keepalive)
	return subject

