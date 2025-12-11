import pycrow
import pycrow.libcrow
from pycrow.libcrow import *


PUBLISHERS = {}
SUBSCRIBERS = {}


def get_publisher(theme):
    if theme in PUBLISHERS:
        return PUBLISHERS[theme]
    crowker = pycrow.libcrow.crowker_address()
    tower = pycrow.get_default_tower()
    pub = pycrow.libcrow.publisher(crowker, theme)
    pub.bind_to_tower(tower)
    PUBLISHERS[theme] = pub
    return PUBLISHERS[theme]


def get_subscriber(theme, incoming):
    crowker = pycrow.libcrow.crowker_address()
    tower = pycrow.get_default_tower()
    sub = pycrow.libcrow.subscriber(incoming)
    sub.bind_to_tower(tower)
    sub.subscribe(crowker, theme)
    SUBSCRIBERS[theme] = sub
    return SUBSCRIBERS[theme]


def publish(theme, bindata):
    pub = get_publisher(theme)
    pub.publish(bindata)


def subscribe(theme, incoming, keepalive=2000):
    sub = get_subscriber(theme, incoming)
    sub.install_keepalive(keepalive, False)
