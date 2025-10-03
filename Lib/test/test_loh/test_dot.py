import sys
import unittest
from test.support import import_helper

class Tests(unittest.TestCase):
    def test_dot_set(self):
        . = 1
        assert . == 1

    def test_dot_arg(self):
        def foo(.):
            return .
        assert foo(1) == 1

        foo(.):
            return .
        assert foo(2) == 2

    def test_dot_method(self):
        C::
            .__init__(x):
                .x = x

            .get():
                return .x

            .set(v):
                .x = v  
        
        c = C(10)
        assert c.get() == 10
        c.set(20)
        assert c.get() == 20
    
    def test_int_dot(self):
        . = 1 + 3j
        assert . == 1 + 3j
        assert .imag == 3
        assert .real == 1

    def test_nested_dot(self):
        C::
            .__init__(v):
                .x = v

            .get():
                return .x

            .set(v):
                .x = v
        
        . = C(10)
        .get()
        assert .get() == 10
        .set(20)
        assert .get() == 20

        .x = C(30)
        assert .x.get() == 30
        .x.set(40)
        assert .x.get() == 40
        
        .x.x = 100
        assert .x.x == 100

