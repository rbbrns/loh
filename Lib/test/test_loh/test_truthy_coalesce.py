import unittest

class Tests(unittest.TestCase):
    def test_truthy_coalesce_basic(self):
        # Falsy values fallback to RHS
        self.assertEqual(0 ?? 42, 42)
        self.assertEqual("" ?? "fallback", "fallback")
        self.assertEqual(None ?? "fallback", "fallback")
        self.assertEqual(False ?? "fallback", "fallback")
        self.assertEqual([] ?? [1], [1])
        
        # Truthy values are preserved
        self.assertEqual(100 ?? 42, 100)
        self.assertEqual("hello" ?? "fallback", "hello")
        self.assertEqual(True ?? "fallback", True)
        self.assertEqual([1, 2] ?? [3], [1, 2])

    def test_short_circuit(self):
        evaluated = False
        def get_rhs():
            nonlocal evaluated
            evaluated = True
            return 42

        # LHS is truthy, RHS should not be evaluated
        x = 10 ?? get_rhs()
        self.assertEqual(x, 10)
        self.assertFalse(evaluated)

        # LHS is falsy, RHS should be evaluated
        x = 0 ?? get_rhs()
        self.assertEqual(x, 42)
        self.assertTrue(evaluated)

    def test_ternary_conditional_compatibility(self):
        # Verify standard ternary: true_val ? condition ?? else_val
        self.assertEqual(100 ? True ?? 200, 100)
        self.assertEqual(100 ? False ?? 200, 200)

    def test_variables_attributes_subscripts(self):
        # Variables
        a = 0
        b = 10
        self.assertEqual(a ?? b, 10)

        # Attributes
        class Holder:
            def __init__(self, val):
                self.val = val
        h = Holder(None)
        self.assertEqual(h.val ?? 42, 42)
        h.val = 100
        self.assertEqual(h.val ?? 42, 100)

        # Subscripts
        lst = [0, "val"]
        self.assertEqual(lst[0] ?? 42, 42)
        self.assertEqual(lst[1] ?? 42, "val")
