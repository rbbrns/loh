import unittest

class TestRangeLiteral(unittest.TestCase):
    def test_range_literal_basic(self):
        # Basic iteration and generation
        r = 0..5
        self.assertEqual(list(r), [0, 1, 2, 3, 4])
        self.assertIsInstance(r, range)

        # Iteration loops
        result = []
        $ i := 0..5:
            result.append(i)
        self.assertEqual(result, [0, 1, 2, 3, 4])

    def test_range_literal_membership(self):
        # Membership checking with <== (in) and !<== (not in)
        self.assertTrue(3 <== 0..10)
        self.assertFalse(10 <== 0..10)
        self.assertTrue(11 !<== 0..10)

    def test_range_literal_variables(self):
        # Using variables as start and stop bounds
        start = 2
        stop = 6
        self.assertEqual(list(start..stop), [2, 3, 4, 5])

    def test_range_literal_expressions(self):
        # Expressions as bounds and operator precedence
        # Arithmetic should have higher precedence than range literal
        self.assertEqual(list(1 + 2 .. 2 * 4), [3, 4, 5, 6, 7])

    def test_range_literal_negative(self):
        # Unary bounds
        self.assertEqual(list(-5..-1), [-5, -4, -3, -2])

if __name__ == "__main__":
    unittest.main()
