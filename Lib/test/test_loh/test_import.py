import unittest

class Tests(unittest.TestCase):
    def test_import(self):
        import math
        assert math.sqrt(4) == 2
        
        <> math
        <. math
        assert math.sqrt(4) == 2

    def test_from(self):
        from math import sqrt
        assert sqrt(4) == 2

        .> math <. sqrt
        assert sqrt(4) == 2
        
    