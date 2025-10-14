import sys
import unittest
from test.support import import_helper

class Tests(unittest.TestCase):
    def test_default_call_args(self):
        Foo::
            a = 1
            b = 2

        foo(a=3, b=4):
                return a, b

        a = 5
        b = 6

        assert foo() == (3,4)
        assert foo(=a, =b) == (5, 6)
        assert foo(=Foo.a, =Foo.b) == (1, 2)
        assert foo(a=7,b=8) == (7,8)
        assert foo(a=, b=) == (None, None)

    def test_default_params(self):
        a = 1
        b = 2
        Foo::
            a = 3
            b = 4

        foo(=a, =b):
            return a, b

        a = 5
        b = 6
        
        assert foo() == (1,2)
        assert foo(=a, =b) == (5, 6)
        assert foo(=Foo.a, =Foo.b) == (3, 4)
        assert foo(a=7,b=8) == (7,8)
        assert foo(a=, b=) == (None, None)

        foo(=Foo.a, =Foo.b):
            return a, b

        assert foo() == (3,4)
        assert foo(=a, =b) == (5, 6)
        assert foo(=Foo.a, =Foo.b) == (3, 4)
        assert foo(a=7,b=8) == (7,8)
        assert foo(a=, b=) == (None, None)


    def test_default_assign(self):
        Foo::
            a = 1
            b = 2

        a = 0
        b = 0
        
        =a
        =b

        assert a == 0
        assert b == 0

        =Foo.a
        =Foo.b

        assert a == 1
        assert b == 2







