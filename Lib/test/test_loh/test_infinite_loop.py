import unittest

class TestInfiniteLoop(unittest.TestCase):
    def test_basic_infinite_loop(self):
        x = 0
        # In Loh, '?:' is if, '$>>' is break, and '$:' is infinite loop
        code = """
x = 0
$:
    x += 1
    if x == 5:
        break
"""
        scope = {}
        exec(code, {}, scope)
        self.assertEqual(scope["x"], 5)

    def test_infinite_loop_with_else_no_break(self):
        # A loop with 'else' block that doesn't break? But an infinite loop always runs forever unless broken.
        # If it is broken, the else block doesn't run.
        code = """
x = 0
$:
    x += 1
    if x == 5:
        break
else:
    x = 100
"""
        scope = {}
        exec(code, {}, scope)
        self.assertEqual(scope["x"], 5)

if __name__ == "__main__":
    unittest.main()
