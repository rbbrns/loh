import sys
import unittest
from test.support import import_helper

class Tests(unittest.TestCase):
    def test_loh_version(self):
        self.assertEqual(sys.loh_version, "0.2.0")
        self.assertEqual(sys.loh_version_info, (0, 2, 0))

    def test_none_assign(self):
        a = ~
        assert a is None
        
        b = ~
        assert b is None

        c = ~;
        assert c is None

        d: float = ~
        assert d is None

        e : int = ~
        assert e is None

        # Check shorthands
        x~
        y~
        z~
        assert x is None
        assert y is None
        assert z is None

    def test_none_params(self):
        def foo(a, b~, c~, d~):
            assert a == None
            assert b === None
            assert c == None
            assert d === None

        foo(None, b=None)

    def test_none_if_comparison(self):
        a = None
        if_ran = False
        if a~~~:
            if_ran = True
        assert if_ran

        if_ran = False
        if a~~:
            if_ran = True
        assert if_ran

        if_ran = False
        if !a~~~:
            if_ran = True
        assert not if_ran

    def test_none_comparison(self):
        a = None
        b = 1
        assert a~~
        assert a~~~

        assert !b~~~
        assert b !== ~

    def test_none_in_dict(self):
        assert type({}) == dict
        assert {'a': ~} == {'a': None}
        assert {'a': ~, 'b': ~} == {'a': None, 'b': None}