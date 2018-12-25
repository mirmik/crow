#!/usr/bin/env python3
#coding: utf-8

import sys
import pycrow

pycrow.diagnostic(pack=True, live=True)

@pycrow.subscribe_handler("mirmik")
def subscribe_handler(packet):
	print("ttt", packet.data())

@pycrow.subscribe_handler("log")
def subscribe_handler(packet):
	print("log", packet.data())

pycrow.set_crowker(".12.109.173.108.206:10009")
pycrow.subscribe("mirmik", ack=2, rack=2)
pycrow.subscribe("log")

pycrow.create_udpgate(12, 10009)
pycrow.spin()