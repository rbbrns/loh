import unittest

class TestFunctionAliases(unittest.TestCase):
    def test_basic_aliases(self):
        # Define a function with one alias
        foo | bar(x, y):
            -> x + y

        self.assertEqual(foo(3, 4), 7)
        self.assertEqual(bar(3, 4), 7)
        self.assertEqual(foo.__name__, "foo")

        # Define a function with multiple aliases
        a | b | c(x):
            -> x * 10

        self.assertEqual(a(2), 20)
        self.assertEqual(b(2), 20)
        self.assertEqual(c(2), 20)
        self.assertEqual(a.__name__, "a")

    def test_method_aliases_symmetric(self):
        C::
            .add | .sum(x, y):
                -> x + y

        c = C()
        self.assertEqual(c.add(10, 20), 30)
        self.assertEqual(c.sum(10, 20), 30)

    def test_method_aliases_asymmetric(self):
        C::
            .multiply | prod(x, y):
                -> x * y

        c = C()
        self.assertEqual(c.multiply(2, 3), 6)
        self.assertEqual(c.prod(2, 3), 6)

    def test_asymmetric_validation(self):
        # Mismatched dot prefixes must raise a SyntaxError
        with self.assertRaises(SyntaxError):
            compile("foo | .bar(x):\n    pass", "<string>", "exec")

    def test_async_aliases(self):
        import asyncio
        async a_foo | a_bar(x):
            -> x + 5

        self.assertEqual(asyncio.run(a_foo(10)), 15)
        self.assertEqual(asyncio.run(a_bar(10)), 15)

    def test_decorated_aliases(self):
        def dec(f):
            def wrapper(*args, **kwargs):
                return f(*args, **kwargs) * 2
            return wrapper

        @dec
        foo | bar(x):
            -> x

        self.assertEqual(foo(5), 10)
        self.assertEqual(bar(5), 10)

if __name__ == "__main__":
    unittest.main()
