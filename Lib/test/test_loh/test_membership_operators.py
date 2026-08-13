import unittest

class TestMembershipOperators(unittest.TestCase):
    def test_basic_membership(self):
        code = """
res1 = 'a' <~ 'abc'
res2 = 'x' !<~ 'abc'
res3 = 1 <~ [1, 2, 3]
res4 = 4 !<~ [1, 2, 3]
res5 = "key" <~ {"key": "val"}
"""
        scope = {}
        exec(code, scope)
        self.assertTrue(scope["res1"])
        self.assertTrue(scope["res2"])
        self.assertTrue(scope["res3"])
        self.assertTrue(scope["res4"])
        self.assertTrue(scope["res5"])

    def test_membership_combined_operators(self):
        code = """
b1 = ('a' <~ 'abc') && ('x' !<~ 'abc')
b2 = ('a' <~ 'abc') ?? False
b3 = "yes" ? ('a' <~ 'abc') ?? "no"
b4 = ('a' <~ 'abc') == True
b5 = !('a' <~ 'abc')
b6 = ({"a"} <= {"a", "b"}) && ('a' <~ {"a", "b"})
"""
        scope = {}
        exec(code, scope)
        self.assertTrue(scope["b1"])
        self.assertTrue(scope["b2"])
        self.assertEqual(scope["b3"], "yes")
        self.assertTrue(scope["b4"])
        self.assertFalse(scope["b5"])
        self.assertTrue(scope["b6"])

    def test_loop_and_comprehension_variations(self):
        code = """
# Explicit single variable
acc1 = []
$ x := [1, 2, 3]:
    acc1.append(x)

# Implicit sigil ($)
acc2 = []
$ := [10, 20, 30]:
    acc2.append($)

# Tuple unpacking
acc3 = []
$ a, b := [(1, 10), (2, 20)]:
    acc3.append(a + b)

# Multi-arity sigil unpacking
acc4 = []
$$ := {"x": 100, "y": 200}.items():
    acc4.append(($, $$))

# Filtered loop (0..6 -> range(0, 6) -> [0, 1, 2, 3, 4, 5])
acc5 = []
$ x := 0..6 ? x % 2 == 0:
    acc5.append(x)

# Comprehensions
comp1 = [x * 2 $ x := 1..4]
comp2 = [$ * 2 := 1..4]
comp3 = {$$: $ := {"a": 1, "b": 2}.items()}
comp4 = {x % 3 $ x := 0..6}
gen_sum = sum($ * 2 := 1..4)
"""
        scope = {}
        exec(code, scope)
        self.assertEqual(scope["acc1"], [1, 2, 3])
        self.assertEqual(scope["acc2"], [10, 20, 30])
        self.assertEqual(scope["acc3"], [11, 22])
        self.assertEqual(scope["acc4"], [("x", 100), ("y", 200)])
        self.assertEqual(scope["acc5"], [0, 2, 4])
        self.assertEqual(scope["comp1"], [2, 4, 6])
        self.assertEqual(scope["comp2"], [2, 4, 6])
        self.assertEqual(scope["comp3"], {1: "a", 2: "b"})
        self.assertEqual(scope["comp4"], {0, 1, 2})
        self.assertEqual(scope["gen_sum"], 12)


if __name__ == "__main__":
    unittest.main()
