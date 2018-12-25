#!/usr/bin/env python3
#coding: utf-8

import sys
import pycrow

pycrow.set_crowker(".12.109.173.108.206:10009")

@pycrow.subscribe_handler("mirmik")
def subscribe_handler(packet):
	thm = packet.theme()
	print(thm)

pycrow.subscribe("mirmik")

pycrow.create_udpgate(12, 10009)
pycrow.spin()