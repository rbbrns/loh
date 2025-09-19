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
        assert ~~ ~~ True
        assert ~~~~ True
        assert ~~ False

        assert !!! False

        if !!! False:
            if_ran = True
        assert if_ran

        if !!!!!! True:
            if_ran2 = False
        assert not if_ran2

    def test_is(self):
        assert True === True
        assert False === False
        assert True === not False
        assert True === ~~ False
        assert ++===!!!--

    def test_del(self):
        x = 1
        del x
        with self.assertRaises(NameError):
            print(x)

        x = 1
        =>x
        with self.assertRaises(NameError):
            print(x)
        



if __name__ == "__main__":
    unittest.main()
