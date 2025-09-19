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

        def test_none():
            ->
        assert test_none() is None

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

    def test_def(self):
        test1():
            -> 1
        assert test1() == 1

        test2() -> Int:
            -> 2
        assert test2() == 2

        test3(a, b, c):
            -> a + b + c
        assert test3(1, 2, 3) == 6

        test4(a, b=1, c=2) -> Int:
            -> a + b + c
        assert test4(1) == 4

        test5(a: Int, b: Int=2, c: Int=3) -> Int:
            -> a + b + c
        assert test5(1) == 6
            
        
        