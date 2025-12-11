#!/usr/bin/env python3
# coding: utf-8

import pytest
import pycrow


def test_import():
    """Test that pycrow module imports correctly."""
    assert hasattr(pycrow, 'udpgate')
    assert hasattr(pycrow, 'subscriber')
    assert hasattr(pycrow, 'publisher')
    assert hasattr(pycrow, 'address')


def test_tower_class():
    """Test that Tower class is available."""
    assert hasattr(pycrow, 'Tower')
    assert hasattr(pycrow, 'default_tower')


def test_create_tower():
    """Test creating a Tower instance."""
    tower = pycrow.Tower()
    assert tower is not None


def test_default_tower():
    """Test default_tower() returns a Tower."""
    tower = pycrow.default_tower()
    assert tower is not None


def test_tower_methods():
    """Test Tower has expected methods."""
    tower = pycrow.Tower()
    assert hasattr(tower, 'send')
    assert hasattr(tower, 'onestep')
    assert hasattr(tower, 'has_untravelled')
    assert hasattr(tower, 'get_total_travelled')


def test_address():
    """Test address function."""
    addr = pycrow.address(".12.127.0.0.1:10009")
    assert addr is not None


def test_udpgate_create():
    """Test creating udpgate."""
    gate = pycrow.udpgate()
    assert gate is not None


def test_udpgate_with_port():
    """Test creating udpgate with port."""
    gate = pycrow.udpgate(19001)
    assert gate is not None


def test_hostaddr():
    """Test hostaddr class."""
    addr = pycrow.hostaddr(".12.127.0.0.1:10009")
    assert addr is not None
