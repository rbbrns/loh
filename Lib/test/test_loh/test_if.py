import sys
import unittest
from test.support import import_helper

class Tests(unittest.TestCase):
    def test_if_true(self):
        ran = 0
        ? !!:
            ran = 1
        assert ran == 1
        
        ?True:
            ran = 2
        assert ran == 2

        ? !!:
            ran = 3
        assert ran == 3
    
    def test_if_false(self):
        ran = False
        ? !!!:
            ran = True
        assert not ran

        ran = False
        ? False:
            ran = True
        assert ~~ran

        ran = False
        ?False:
            ran = True
        assert not ran

        ran = False
        ? !!!:
            ran = True
        assert not ran
        
        ran = False
        ? ~~!!:
            ran = True
        assert not ran

    def test_elif(self):
        if_ran = False
        elif_ran = False
        ? False:
            if_ran = True
        ?!? True:
            elif_ran = True
        assert not if_ran
        assert elif_ran

        if_ran = False
        elif_ran = False
        ? False:
            if_ran = True
        ?!? False:
            elif_ran = True
        assert not if_ran
        assert not elif_ran

    def test_else(self):
        if_ran = False
        else_ran = False
        ? False:
            if_ran = True
        ?!:
            else_ran = True
        assert not if_ran
        assert else_ran

        if_ran = False
        else_ran = False
        ? True:
            if_ran = True
        ?!:
            else_ran = True
        assert if_ran
        assert not else_ran

       

if __name__ == "__main__":
    unittest.main()
