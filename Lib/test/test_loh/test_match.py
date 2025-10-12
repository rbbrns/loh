import unittest


class Tests(unittest.TestCase):
    def test_match(self):
        x = 1
        match x:
            case 1:
                case_ran = 1
            case 2:
                ^^^ AssertionError
            case _:
                ^^^ AssertionError
        assert case_ran == 1

        ?== x:
            1:
                case_ran = 1
            2:
                ^^^ AssertionError
            _:
                ^^^ AssertionError
        assert case_ran == 1

        x = -1
        ?== x:
            1:
                ^^^ AssertionError
            2:
                ^^^ AssertionError
            _:
                guard_ran = True
        assert guard_ran

        x = 'str'
        ?== x:
            str() => s:
                str_ran = s
        assert str_ran == 'str'

        x = 2
        match y := x:
            case 2:
                case_ran = y
            case _:
                ^^^ AssertionError

        
        assert case_ran == 2
                