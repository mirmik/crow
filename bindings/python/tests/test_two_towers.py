#!/usr/bin/env python3
# coding: utf-8

import pytest
import pycrow
import time


class TestTwoTowers:
    """Tests for multiple independent Tower instances."""

    def test_create_two_towers(self):
        """Test that two independent towers can be created."""
        tower1 = pycrow.Tower()
        tower2 = pycrow.Tower()
        assert tower1 is not None
        assert tower2 is not None
        assert tower1 is not tower2

    def test_udpgate_bind_to_tower(self):
        """Test binding udpgate to a specific tower."""
        tower = pycrow.Tower()
        gate = pycrow.udpgate()
        gate.open(19101)
        gate.bind_to_tower(tower, 12)
        gate.close()

    def test_two_towers_with_separate_gates(self):
        """Test two towers with separate UDP gates."""
        tower1 = pycrow.Tower()
        tower2 = pycrow.Tower()

        gate1 = pycrow.udpgate()
        gate2 = pycrow.udpgate()

        gate1.open(19201)
        gate2.open(19202)

        gate1.bind_to_tower(tower1, 12)
        gate2.bind_to_tower(tower2, 12)

        gate1.close()
        gate2.close()

    def test_tower_send_qos0(self):
        """Test sending message between two towers with QoS 0."""
        tower1 = pycrow.Tower()
        tower2 = pycrow.Tower()

        gate1 = pycrow.udpgate()
        gate2 = pycrow.udpgate()

        gate1.open(19301)
        gate2.open(19302)

        gate1.bind_to_tower(tower1, 12)
        gate2.bind_to_tower(tower2, 12)

        # Send from tower1 to tower2
        addr = pycrow.address(".12.127.0.0.1:19302")
        tower1.send(addr, b"Hello from tower1", type=0, qos=0)

        # Process - need to manually call read_handler for synchronous testing
        for _ in range(10):
            tower1.onestep()
            gate2.read_handler(0)  # Manually trigger UDP read
            tower2.onestep()
            if tower2.get_total_travelled() > 0:
                break
            time.sleep(0.01)

        # Verify tower2 received
        assert tower2.get_total_travelled() == 1

        gate1.close()
        gate2.close()

    def test_tower_send_qos2(self):
        """Test sending message between two towers with QoS 2 (reliable)."""
        tower1 = pycrow.Tower()
        tower2 = pycrow.Tower()

        gate1 = pycrow.udpgate()
        gate2 = pycrow.udpgate()

        gate1.open(19401)
        gate2.open(19402)

        gate1.bind_to_tower(tower1, 12)
        gate2.bind_to_tower(tower2, 12)

        # Send from tower1 to tower2 with QoS 2
        addr = pycrow.address(".12.127.0.0.1:19402")
        tower1.send(addr, b"Reliable message", type=0, qos=2, ackquant=50)

        # Process multiple times for reliable delivery with manual read_handler
        for _ in range(20):
            tower1.onestep()
            gate2.read_handler(0)  # Receive packet on gate2
            tower2.onestep()
            gate1.read_handler(0)  # Receive ACK on gate1

            if tower2.get_total_travelled() > 0 and not tower1.has_untravelled():
                break
            time.sleep(0.01)

        # Verify tower2 received and tower1 got ack
        # QoS 2 includes ACKs, so count may be > 1
        assert tower2.get_total_travelled() >= 1
        assert not tower1.has_untravelled()

        gate1.close()
        gate2.close()

    def test_bidirectional_communication(self):
        """Test bidirectional communication between two towers."""
        tower1 = pycrow.Tower()
        tower2 = pycrow.Tower()

        gate1 = pycrow.udpgate()
        gate2 = pycrow.udpgate()

        gate1.open(19501)
        gate2.open(19502)

        gate1.bind_to_tower(tower1, 12)
        gate2.bind_to_tower(tower2, 12)

        # Send from tower1 to tower2
        addr2 = pycrow.address(".12.127.0.0.1:19502")
        tower1.send(addr2, b"Message to tower2", qos=0)

        # Send from tower2 to tower1
        addr1 = pycrow.address(".12.127.0.0.1:19501")
        tower2.send(addr1, b"Message to tower1", qos=0)

        # Process with manual read_handler calls
        for _ in range(10):
            tower1.onestep()
            tower2.onestep()
            gate1.read_handler(0)
            gate2.read_handler(0)

            if tower1.get_total_travelled() > 0 and tower2.get_total_travelled() > 0:
                break
            time.sleep(0.01)

        # Both should have received (count includes sent + received)
        assert tower1.get_total_travelled() >= 1
        assert tower2.get_total_travelled() >= 1

        gate1.close()
        gate2.close()

    def test_independent_towers(self):
        """Test that towers are truly independent."""
        tower1 = pycrow.Tower()
        tower2 = pycrow.Tower()

        # Initially both should have 0 travelled
        assert tower1.get_total_travelled() == 0
        assert tower2.get_total_travelled() == 0

        # Set different retransling settings
        tower1.set_retransling_allowed(True)
        tower2.set_retransling_allowed(False)

        assert tower1.get_retransling_allowed() == True
        assert tower2.get_retransling_allowed() == False
