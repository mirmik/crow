#!/usr/bin/env python3

import pycrow

pycrow.start_client()

pycrow.publish("lalala", "enc".encode("utf-8"))

pycrow.stop_client()