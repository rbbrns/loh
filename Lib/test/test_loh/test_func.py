import sys
import unittest
from test.support import import_helper

class Tests(unittest.TestCase):
    def test_return(self):
        def test():
            -> 1
        assert test() == 1

        def test2():
            <- 2
        assert test2() == 2

    def test_yield(self):
        def test():
            ~> 1
            ~> 2
            ~> 3
        assert list(test()) == [1, 2, 3]

        def test2():
            <~ 1
            <~ 2
            <~ 3
        assert list(test2()) == [1, 2, 3]
        