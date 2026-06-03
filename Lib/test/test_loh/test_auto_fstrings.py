import unittest

class TestAutoFstrings(unittest.TestCase):
    def test_auto_fstrings_basic(self):
        # We'll use compile/exec to test files with the future import
        code = """from __loh__ import auto_fstrings
x = 42
s = "value: {x}"
s2 = "expr: {x + 8}"
s3 = "format: {x:.2f}"
"""
        scope = {}
        exec(code, {}, scope)
        self.assertEqual(scope["s"], "value: 42")
        self.assertEqual(scope["s2"], "expr: 50")
        self.assertEqual(scope["s3"], "format: 42.00")

    def test_auto_fstrings_future_module_name(self):
        # Test that standard __future__ module also works
        code = """from __future__ import auto_fstrings
x = 100
s = "value: {x}"
"""
        scope = {}
        exec(code, {}, scope)
        self.assertEqual(scope["s"], "value: 100")

    def test_auto_fstrings_doubled_braces(self):
        code = """from __loh__ import auto_fstrings
s = "literal {{braces}}"
s2 = "mix {10} with {{braces}}"
"""
        scope = {}
        exec(code, {}, scope)
        self.assertEqual(scope["s"], "literal {braces}")
        self.assertEqual(scope["s2"], "mix 10 with {braces}")

    def test_auto_fstrings_raw_strings(self):
        code = """from __loh__ import auto_fstrings
x = 42
s = r"raw string {x}"
s2 = R"another {x}"
"""
        scope = {}
        exec(code, {}, scope)
        self.assertEqual(scope["s"], "raw string {x}")
        self.assertEqual(scope["s2"], "another {x}")

    def test_auto_fstrings_docstrings(self):
        code = """from __loh__ import auto_fstrings
x = 42
def my_func():
    \"\"\"This is doc {x}\"\"\"
    return 123

class MyClass:
    "class doc {x}"
    def method(self):
        '''method doc {x}'''
        pass

s = "regular {x}"
"""
        scope = {}
        exec(code, {}, scope)
        self.assertEqual(scope["my_func"].__doc__, "This is doc {x}")
        self.assertEqual(scope["MyClass"].__doc__, "class doc {x}")
        self.assertEqual(scope["MyClass"].method.__doc__, "method doc {x}")
        self.assertEqual(scope["s"], "regular 42")

    def test_auto_fstrings_syntax_error(self):
        code = """from __loh__ import auto_fstrings
s = "invalid {x + }"
"""
        with self.assertRaises(SyntaxError):
            exec(code, {}, {})

    def test_auto_fstrings_no_braces(self):
        code = """from __loh__ import auto_fstrings
s = "no braces here"
"""
        scope = {}
        exec(code, {}, scope)
        self.assertEqual(scope["s"], "no braces here")

    def test_auto_fstrings_normal_strings(self):
        code = r"""from __loh__ import auto_fstrings
x = 42
s = n"normal string {x}\nnewline"
s2 = N"another {x}"
"""
        scope = {}
        exec(code, {}, scope)
        self.assertEqual(scope["s"], "normal string {x}\nnewline")
        self.assertEqual(scope["s2"], "another {x}")

    def test_auto_fstrings_normal_string_error(self):
        # n-prefix cannot be combined with other prefixes
        with self.assertRaises(SyntaxError):
            exec('s = nr"hello"', {}, {})
        with self.assertRaises(SyntaxError):
            exec('s = rn"hello"', {}, {})

if __name__ == "__main__":
    unittest.main()
