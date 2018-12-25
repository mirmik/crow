#!/usr/bin/env python3
#coding: utf-8

import sys
import pycrow

pycrow.set_crowker(".12.109.173.108.206:10009")
pycrow.subscribe("pulse", ack=2, handler=lambda packet: print(packet.data().decode("utf-8")))

pycrow.create_udpgate(12, 10009)
pycrow.spin()