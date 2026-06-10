import unittest
import asyncio

class Tests(unittest.TestCase):
    def test_global_function_aliasing(self):
        # Global-style function definition with two names
        foo | bar(x, y):
            -> x + y
        
        self.assertEqual(foo(1, 2), 3)
        self.assertEqual(bar(1, 2), 3)

    def test_multiple_aliases(self):
        # Defining three aliases
        f1 | f2 | f3(x):
            -> x * 2
        
        self.assertEqual(f1(5), 10)
        self.assertEqual(f2(5), 10)
        self.assertEqual(f3(5), 10)

    def test_local_nested_aliasing(self):
        # Nesting inside another function scope
        def outer():
            a | b(val):
                -> val + 100
            -> a(1), b(2)
        
        self.assertEqual(outer(), (101, 102))

    def test_class_method_aliasing(self):
        # Class methods with dot prefixes (.foo | .bar)
        class MyClass:
            .foo | .bar(val):
                -> val * 10
        
        instance = MyClass()
        self.assertEqual(instance.foo(5), 50)
        self.assertEqual(instance.bar(5), 50)

    def test_decorator_propagation(self):
        # Decorators should only apply to the primary function,
        # but the alias references the decorated function object.
        def multiply_by_two(func):
            def wrapper(*args, **kwargs):
                return func(*args, **kwargs) * 2
            return wrapper

        @multiply_by_two
        val_foo | val_bar(x):
            -> x

        self.assertEqual(val_foo(10), 20)
        self.assertEqual(val_bar(10), 20)

    def test_class_method_with_decorators(self):
        def add_five(func):
            def wrapper(*args, **kwargs):
                return func(*args, **kwargs) + 5
            return wrapper

        class DecClass:
            @add_five
            .m_foo | .m_bar(val):
                -> val
        
        inst = DecClass()
        self.assertEqual(inst.m_foo(1), 6)
        self.assertEqual(inst.m_bar(1), 6)

    def test_async_function_aliasing(self):
        # Async definitions
        async async_foo | async_bar(x):
            -> x + 5

        loop = asyncio.new_event_loop()
        try:
            self.assertEqual(loop.run_until_complete(async_foo(1)), 6)
            self.assertEqual(loop.run_until_complete(async_bar(1)), 6)
        finally:
            loop.close()

    def test_prefix_symmetry_checks(self):
        # Mismatched prefixes must raise a compile-time SyntaxError
        invalid_cases = [
            "def foo | .bar(x): pass",
            "foo | .bar(x): pass",
        ]
        
        for code in invalid_cases:
            with self.assertRaises(SyntaxError):
                compile(code, "?", "exec")
