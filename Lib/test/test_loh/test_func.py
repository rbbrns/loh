import sys
import unittest
from test.support import import_helper

class Tests(unittest.TestCase):
    def test_return(self):
        def test():
            -> 1
        assert test() == 1

        def test2():
            -> 2
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
            
    def test_lambda(self):
        test1 = (x)->x
        assert test1(1) == 1

        test2 = (x, y)->x + y
        assert test2(1, 2) == 3

        test3 = ()->1
        assert test3() == 1

        def call_lambda(l):
            return l(1)
        assert call_lambda(l=(x) -> x + 1) == 2
        assert ((x) -> x + 1)(1) == 2

    def test_lambda_capture(self):
        x = 1
        test1 = (x)->x
        assert test1(2) == 2
        assert x == 1

        def multiplier(n):
            return lambda x: x * n

        assert multiplier(2)(3) == 6
        assert multiplier(3)(4) == 12
        
    def test_duplicate_kwargs(self):
        def test1(a, b, c):
            -> a, b, c
        assert test1(1, 2, 3) == (1, 2, 3)
        assert test1(a=1, b=2, c=3) == (1, 2, 3)
        assert test1(1, b=2, c=3, a=10) == (10, 2, 3)
        assert test1(a=1, b=2, c=3, **{'a': 10, 'b': 20, 'c': 30}) == (10, 20, 30)
        assert test1(**{'a': 10, 'b': 20, 'c': 30}, b=-2, **{'a': 1}) == (1, -2, 30)

    def test_implicit_kwargs(self):
        def test1(a, b=2, **):
            c = 3
            -> a,b,**,c
        
        assert test1(1) == (1, 2, {}, 3)
        assert test1(1, x=10) == (1, 2, {'x': 10}, 3)
        
        def test2(**):
            -> (**)
        assert test2() == {}
        assert test2(a=1) == {'a': 1}
        assert test2(a=1, b=2) == {'a': 1, 'b': 2}
        
        def test3(**):
            **['x'] = 10
            -> **
        assert test3(x=1) == {'x': 10}
        
        def test4(**):
            def inner(**):
                -> (**)
            -> inner(**)
        assert test4(a=5, b=6) == {'a': 5, 'b': 6}