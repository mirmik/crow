import pycrow
import pycrow.libcrow

from pycrow.libcrow import *

def spin():
	while 1:
		pycrow.libcrow.onestep()

def incoming_handler(func):
	pycrow.libcrow.set_incoming_handler(func)

subs = dict()
subs_init = False

def subscribe_handler_impl(packet):
	thm = packet.theme()
	subs[thm](packet)

def subscribe_handler(theme):
	def decorator(func):
		subs[theme] = func

	if not subs_init:
		pycrow.libcrow.set_subscribe_handler(subscribe_handler_impl)

	return decorator