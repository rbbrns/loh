import unittest
from dataclasses import dataclass
from typing import Annotated
from abc import ABC


class Tests(unittest.TestCase):
    def test_class(self):
        A::...
        B::...
        C::...
        assert A()
        assert B()
        assert C()

        C:(ABC):
            pass
        self.assertIsSubclass(C, ABC)

        C:(A, B):
            pass
        self.assertIsSubclass(C, A)
        self.assertIsSubclass(C, B)

        C:A:
            pass
        self.assertIsSubclass(C, A)

        C:A,B:
            pass
        self.assertIsSubclass(C, A)
        self.assertIsSubclass(C, B)

    def test_methods(self):
        C::
            def foo(self):
                return 1
        assert C().foo() == 1

    def test_init(self):
        C::
            def __init__(self, v):
                self.v = v
        assert C(1).v == 1

    def test_metaclass(self):
        class Meta(type):
            pass
        C:metaclass=Meta:
            pass
        assert isinstance(C, Meta)

        C:(metaclass=Meta):
            pass
        assert isinstance(C, Meta)

    def test_annotated(self):
        C::
            v:Annotated[int, 'v'] = 1
        assert C().v == 1


if __name__ == "__main__":
    unittest.main()
