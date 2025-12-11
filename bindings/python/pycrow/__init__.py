try:
    import pycrow.libcrow
    from pycrow.libcrow import *
except Exception as ex:
    print("PyCrow: Import libs error. ex:", ex)
    exit(0)


# Global state for default tower and executor
_default_tower = None
_default_executor = None
_default_udpgate = None


def get_default_tower():
    """Get or create the default Tower instance."""
    global _default_tower
    if _default_tower is None:
        _default_tower = pycrow.libcrow.Tower()
    return _default_tower


def start_client(port=0):
    """
    Start crow client with UDP gate.
    @port - udp port. 0 for dynamic
    """
    global _default_tower, _default_executor, _default_udpgate

    _default_tower = pycrow.libcrow.Tower()
    _default_udpgate = pycrow.libcrow.udpgate(port)
    _default_udpgate.bind_to_tower(_default_tower, 12)

    _default_executor = pycrow.libcrow.TowerThreadExecutor(_default_tower)
    _default_executor.start()


def stop_client():
    """Stop crow client."""
    global _default_executor, _default_udpgate
    if _default_executor is not None:
        _default_executor.stop(True)
        _default_executor = None
    if _default_udpgate is not None:
        _default_udpgate.close()
        _default_udpgate = None
