import unittest

class Tests(unittest.TestCase):
    def test_parameter_alias_with_default(self):
        def f(limit | l = 100):
            return limit

        # Positional passing to primary name
        self.assertEqual(f(42), 42)

        # Keyword passing to primary name
        self.assertEqual(f(limit=50), 50)

        # Keyword passing to alias name
        self.assertEqual(f(l=60), 60)

        # Neither passed (falls back to default)
        self.assertEqual(f(), 100)

        # Both passed (raises TypeError)
        with self.assertRaises(TypeError) as ctx:
            f(42, l=24)
        self.assertIn("got multiple values for alias parameter 'limit'/'l'", str(ctx.exception))

        with self.assertRaises(TypeError) as ctx:
            f(limit=42, l=24)
        self.assertIn("got multiple values for alias parameter 'limit'/'l'", str(ctx.exception))

    def test_parameter_alias_no_default(self):
        def f(x | y):
            return x

        # Positional passing
        self.assertEqual(f(5), 5)

        # Keyword passing to primary
        self.assertEqual(f(x=10), 10)

        # Keyword passing to alias
        self.assertEqual(f(y=20), 20)

        # Neither passed (raises TypeError)
        with self.assertRaises(TypeError) as ctx:
            f()
        self.assertIn("missing 1 required argument: 'x' or 'y'", str(ctx.exception))

        # Both passed (raises TypeError)
        with self.assertRaises(TypeError) as ctx:
            f(5, y=10)
        self.assertIn("got multiple values for alias parameter 'x'/'y'", str(ctx.exception))

    def test_alias_scope_deletion(self):
        def f(limit | l = 100):
            # The alias parameter `l` should be deleted and not visible in the function body
            is_l_in_locals = 'l' in locals()
            
            # Trying to reference `l` should raise a NameError
            try:
                val = l
            except NameError:
                raised_name_error = True
            else:
                raised_name_error = False
                
            return limit, is_l_in_locals, raised_name_error

        limit_val, is_l_in_locals, raised_name_error = f(l=42)
        self.assertEqual(limit_val, 42)
        self.assertFalse(is_l_in_locals)
        self.assertTrue(raised_name_error)

    def test_multiple_aliases(self):
        def f(a | x = 1, b | y = 2):
            return a, b

        self.assertEqual(f(), (1, 2))
        self.assertEqual(f(10), (10, 2))
        self.assertEqual(f(x=10), (10, 2))
        self.assertEqual(f(y=20), (1, 20))
        self.assertEqual(f(10, 20), (10, 20))
        self.assertEqual(f(10, y=20), (10, 20))
        self.assertEqual(f(x=10, y=20), (10, 20))

        with self.assertRaises(TypeError):
            f(10, x=20)
        with self.assertRaises(TypeError):
            f(10, 20, y=30)

    def test_keyword_only_alias(self):
        # Test in keyword-only arguments section of signature (after *)
        def f(*, limit | l = 100):
            return limit

        self.assertEqual(f(), 100)
        self.assertEqual(f(limit=5), 5)
        self.assertEqual(f(l=10), 10)
        
        with self.assertRaises(TypeError):
            f(5)  # cannot pass positionally since it is after *
            
        with self.assertRaises(TypeError):
            f(limit=5, l=10)
