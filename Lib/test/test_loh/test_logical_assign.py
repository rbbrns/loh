import unittest

class Tests(unittest.TestCase):
    def test_or_assign_basic(self):
        # Falsy left hand side, should assign right hand side
        x = 0
        x ||= 42
        self.assertEqual(x, 42)

        x = None
        x ||= "default"
        self.assertEqual(x, "default")

        x = ""
        x ||= "fallback"
        self.assertEqual(x, "fallback")

        # Truthy left hand side, should keep it
        x = 100
        x ||= 42
        self.assertEqual(x, 100)

        x = "hello"
        x ||= "fallback"
        self.assertEqual(x, "hello")

    def test_and_assign_basic(self):
        # Truthy left hand side, should assign right hand side
        x = 100
        x &&= 42
        self.assertEqual(x, 42)

        x = "hello"
        x &&= "fallback"
        self.assertEqual(x, "fallback")

        # Falsy left hand side, should keep it
        x = 0
        x &&= 42
        self.assertEqual(x, 0)

        x = None
        x &&= "default"
        self.assertIsNone(x)

    def test_short_circuit_evaluation(self):
        # For ||=, if LHS is truthy, RHS is not evaluated
        x = True
        evaluated = False
        def get_rhs():
            nonlocal evaluated
            evaluated = True
            return 42
        x ||= get_rhs()
        self.assertEqual(x, True)
        self.assertFalse(evaluated)

        # For &&=, if LHS is falsy, RHS is not evaluated
        x = False
        evaluated = False
        x &&= get_rhs()
        self.assertEqual(x, False)
        self.assertFalse(evaluated)

    def test_subscript_and_attribute(self):
        lst = [0, 100]
        lst[0] ||= 42
        self.assertEqual(lst[0], 42)
        lst[1] &&= 200
        self.assertEqual(lst[1], 200)

        class Holder:
            def __init__(self):
                self.x = 0
                self.y = 100

        h = Holder()
        h.x ||= 42
        self.assertEqual(h.x, 42)
        h.y &&= 200
        self.assertEqual(h.y, 200)
