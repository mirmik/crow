#!/usr/bin/env python3

import pycrow
import time
import signal

def _exit():
	sys.exit(0)

signal.signal(signal.SIGINT, _exit)

pycrow.start_spin()
pycrow.diagnostic(True)

ugate = pycrow.udpgate(10010).bind(12)
addr = pycrow.address(".12.127.0.0.1:10010")
addrself = pycrow.address("")
#pycrow.send(addr, "HelloWorld", 0, 0, 100)

mb0 = pycrow.msgbox().bind(13)
mb1 = pycrow.msgbox().bind(14)

mb0.send(addrself, 14, "hello", 0, 100)
pack = mb1.receive()
print(pack.rawdata())
mb1.send(addr, 13, "rrrr", 0, 100)

pack = mb0.receive()
print(pack.rawdata())

time.sleep(0.01)
pycrow.waitall()
pycrow.finish_spin()