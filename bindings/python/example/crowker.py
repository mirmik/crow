#!/usr/bin/env python3

import pycrow

crowker = pycrow.crowker.instance()
crowker.set_info_mode(True)

print(crowker)
crowker.publish("theme", "data")