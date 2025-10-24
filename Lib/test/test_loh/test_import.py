import unittest

class Tests(unittest.TestCase):
    def test_import(self):
        /math
        assert math.sqrt(4) == 2

    def test_from(self):
        /math/sqrt
        assert sqrt(4) == 2
    
    
    