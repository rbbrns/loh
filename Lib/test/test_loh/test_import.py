import unittest

from itertools import *

class Tests(unittest.TestCase):
    def test_import(self):
        /math
        assert math.sqrt(4) == 2

    def test_from(self):
        /math/sqrt
        assert sqrt(4) == 2

    def test_as(self):
        /math => m
        assert m.sqrt(4) == 2

        /math/sqrt => s
        assert s(4) == 2
    
    def test_import_all(self):
        #intertools imported above at module level
        assert isinstance(chain(), chain)

    def test_import_multiple(self):
        / math / sqrt, floor
        assert sqrt(4) == 2
        assert floor(4.5) == 4

        / math, datetime
        assert sqrt(4) == 2 
        assert datetime.datetime.now()

        / os / (path, listdir)
        assert path
        assert listdir

    def test_import_multiple_as(self):
        / math => m, datetime => d
        assert m.sqrt(4) == 2 
        assert d.datetime.now()

    def test_import_submodule(self):
        / xml.etree.ElementTree
        assert xml.etree.ElementTree

        / xml.etree / ElementTree
        assert ElementTree

        / xml.etree / ElementTree => ET
        assert ET

    def test_relative_import(self):
        pass
    
    