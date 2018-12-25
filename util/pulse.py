#!/usr/bin/env python3
#coding: utf-8

import pycrow
import time

pycrow.use_environment_crowker()
pycrow.create_udpgate(12, 0)

i = 0
while 1:
	pycrow.publish("pulse2", str(i))
	pycrow.onestep()
	
	i += 1
	
	time.sleep(1)