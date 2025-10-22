import sys
import unittest
from test.support import import_helper

class Tests(unittest.TestCase):
    def test_true(self):
        assert ++
        assert ++ === True
        assert True === ++


    def test_false(self):
        assert not --
        assert -- === False
        assert False === --
        assert False == --
        assert -- == False

    def test_is_not(self):
        x = [1,2]
        y = [1,2]
        assert x == y
        assert x !== y
        assert x is not y

    def test_none(self):
        assert not ~
        assert ~ is None
        assert ~ == None
        assert None == ~
        assert None is ~

    def test_and(self):
        assert True && True

    def test_or(self):
        assert True || False
        assert False || True

    def test_not(self):
        assert !! True
        assert ! False
        assert !False
        assert! False
        assert!False

        ^?! !! ++
        ^?! ! --
        ^?!!--

        if_ran--
        if ! False:
            if_ran++
        assert if_ran

        if_ran--
        ? ! --:
            if_ran++
        assert if_ran

        assert True ? ! False
        assert False ? ! True ?! ! False
        
        assert ! False

    def test_is(self):
        assert True === True
        assert False === False
        assert True === not False
        assert True === !False
        assert ++ === !--

    def test_del(self):
        x = 1
        del x
        with self.assertRaises(NameError):
            print(x)

        x = 1
        <> x
        with self.assertRaises(NameError):
            print(x)

    
    def test_in(self):
        assert 'a' <~ 'abc'
        assert 'a' not <~ 'def'
        assert 'a' !<~ 'def'        

    def test_just_if_expression(self):
        assert (True if True) is True
        assert (True if False) is None

        assert (True ? True) is True
        assert (False ? False) is None

if __name__ == "__main__":
    unittest.main()
