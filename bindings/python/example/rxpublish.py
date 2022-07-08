#!/usr/bin/env python3

import struct

import pycrow
from pycrow.rxcrow import rxpublish, rxsubscribe

from rxsignal import rxinterval, rxprint

pycrow.start_client()
pycrow.diagnostic_setup(True)

t = rxinterval(1)
rxprint(t)

rxpublish("pulse", t.map(lambda t: struct.pack("!i", t)))

while True:
    pass
