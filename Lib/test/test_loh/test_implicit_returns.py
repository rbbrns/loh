import unittest
import asyncio

class TestImplicitReturns(unittest.TestCase):
    def test_implicit_returns_basic(self):
        code = """from __loh__ import implicit_returns

def add(a, b):
    a + b

def multiline(a, b):
    x = a * 2
    x * b
"""
        scope = {}
        exec(code, {}, scope)
        self.assertEqual(scope["add"](3, 4), 7)
        self.assertEqual(scope["multiline"](3, 4), 24)

    def test_implicit_returns_future_module_name(self):
        code = """from __future__ import implicit_returns

def square(x):
    x * x
"""
        scope = {}
        exec(code, {}, scope)
        self.assertEqual(scope["square"](5), 25)

    def test_implicit_returns_disabled(self):
        code = """
def add(a, b):
    a + b
"""
        scope = {}
        exec(code, {}, scope)
        self.assertIsNone(scope["add"](3, 4))

    def test_implicit_returns_docstrings(self):
        code = """from __loh__ import implicit_returns

def only_doc():
    \"\"\"This is docstring\"\"\"

def doc_and_expr():
    \"\"\"Docstring here\"\"\"
    42
"""
        scope = {}
        exec(code, {}, scope)
        self.assertIsNone(scope["only_doc"]())
        self.assertEqual(scope["only_doc"].__doc__, "This is docstring")
        self.assertEqual(scope["doc_and_expr"](), 42)
        self.assertEqual(scope["doc_and_expr"].__doc__, "Docstring here")

    def test_implicit_returns_nested_functions(self):
        code = """from __loh__ import implicit_returns

def outer(x):
    def inner(y):
        y * 2
    inner(x) + 5
"""
        scope = {}
        exec(code, {}, scope)
        self.assertEqual(scope["outer"](10), 25)

    def test_implicit_returns_non_expr_statements(self):
        code = """from __loh__ import implicit_returns

def test_pass():
    pass

def test_assign():
    x = 42

def test_if(x):
    if x > 0:
        "positive"
    else:
        "non-positive"
"""
        scope = {}
        exec(code, {}, scope)
        self.assertIsNone(scope["test_pass"]())
        self.assertIsNone(scope["test_assign"]())
        self.assertIsNone(scope["test_if"](5))

    def test_implicit_returns_async(self):
        code = """from __loh__ import implicit_returns
import asyncio

async def async_add(a, b):
    a + b
"""
        scope = {}
        exec(code, {}, scope)
        
        async def run():
            return await scope["async_add"](10, 20)
            
        result = asyncio.run(run())
        self.assertEqual(result, 30)

    def test_implicit_returns_generator(self):
        code = """from __loh__ import implicit_returns

def gen():
    yield 1
    yield 2
"""
        scope = {}
        exec(code, {}, scope)
        g = scope["gen"]()
        self.assertEqual(next(g), 1)
        try:
            next(g)
        except StopIteration as e:
            # yield 2 was wrapped as return (yield 2)
            # which returns whatever yield 2 evaluates to (None when next() resumes it)
            self.assertIsNone(e.value)

if __name__ == "__main__":
    unittest.main()
