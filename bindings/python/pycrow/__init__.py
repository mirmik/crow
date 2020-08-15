try:
	import pycrow.libcrow
	from pycrow.libcrow import *
except Exception as ex:
	print("PyCrow: Import libs error. ex:", ex)
	exit(0)

import time
import threading

THREAD = None
crowker = None
spin_cancel = False

def spin():
	while 1:
		if spin_cancel: return
		pycrow.libcrow.onestep()
		time.sleep(0.0001)

def start_spin():
	global THREAD
	THREAD = threading.Thread(target = spin, args=())
	THREAD.start()

def stop_spin():
	global spin_cancel
	spin_cancel = True


def join_spin():
	THREAD.join()

def waitall():
	while not pycrow.fully_empty():
		time.sleep(0.0001) 

def diagnostic(pack=False, live=False):
	if pack:
		pycrow.libcrow.diagnostic()

	if live:
		pycrow.libcrow.live_diagnostic()

def incoming_handler(func):
	pycrow.libcrow.set_incoming_handler(func)

subs = dict()
subs_init = False

def subscribe_handler_impl(packet):
	packet = pycrow.packet_pubsub_ptr(packet)
	thm = packet.theme().decode('utf-8')
	subs[thm](packet)

def subscribe(theme, handler, qos=0, ackquant=200, rqos=0, rackquant=200):
	subs[theme] = handler

	if not subs_init:
		pycrow.libcrow.set_subscribe_handler(subscribe_handler_impl)

	return pycrow.libcrow.subscribe(crowker, theme, qos, ackquant, rqos, rackquant)


def use_environment_crowker():
	crowker = pycrow.libcrow.environment_crowker()

def set_crowker(cker):
	global crowker
	crowker = pycrow.libcrow.compile_address(cker)
