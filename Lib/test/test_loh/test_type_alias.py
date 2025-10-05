from types import UnionType, GenericAlias
from typing import TypeAliasType, Union
import unittest
from test.support import import_helper

class Tests(unittest.TestCase):
    def test_type_alias(self):
        : IntOrFloat = UnionType[int, float]
        assert isinstance(IntOrFloat, TypeAliasType)
        assert IntOrFloat.__value__ == UnionType[int, float]

        : FloatType = float
        assert isinstance(FloatType, TypeAliasType)
        assert FloatType.__value__ == float

        : IntOrFloat = int | float
        assert isinstance(IntOrFloat, TypeAliasType)
        assert IntOrFloat.__value__ == UnionType[int, float]


    def test_old_type_alias(self):
        IntOrFloat = UnionType[int, float]
        assert isinstance(IntOrFloat, Union)
        assert IntOrFloat.__args__ == (int, float)

        FloatType = float
        assert isinstance(FloatType, type)
        
        IntOrFloat = int | float
        assert isinstance(IntOrFloat, Union)
        assert IntOrFloat.__args__ == (int, float)
        
        
        
        