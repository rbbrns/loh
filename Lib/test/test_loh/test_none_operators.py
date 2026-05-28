import unittest

class Tests(unittest.TestCase):
    def test_safe_navigation_basic(self):
        class Foo:
            def __init__(self, x):
                self.x = x

        f = Foo(42)
        n = None

        # When object is not None
        self.assertEqual(f~.x, 42)
        
        # When object is None
        self.assertIsNone(n~.x)

    def test_safe_navigation_single_evaluation(self):
        class Counter:
            def __init__(self):
                self.count = 0
                self.val = Foo(100)

        class Foo:
            def __init__(self, x):
                self.x = x

        c = Counter()

        def get_foo():
            c.count += 1
            return c.val

        # Member access should evaluate get_foo() exactly once
        self.assertEqual(get_foo()~.x, 100)
        self.assertEqual(c.count, 1)

        # None case
        c2 = Counter()
        def get_none():
            c2.count += 1
            return None

        self.assertIsNone(get_none()~.x)
        self.assertEqual(c2.count, 1)

    def test_safe_navigation_chaining(self):
        class Node:
            def __init__(self, val, next_node=None):
                self.val = val
                self.next = next_node

        n1 = Node(1, Node(2, Node(3)))
        n2 = Node(1, Node(2, None))

        # Chaining when no None in between
        self.assertEqual(n1~.next~.next~.val, 3)
        self.assertEqual(n1~.next~.val, 2)

        # Chaining when there is None in between
        self.assertIsNone(n1~.next~.next~.next~.val)
        self.assertIsNone(n2~.next~.next~.val)

    def test_none_coalescing_basic(self):
        # basic checks
        self.assertEqual(42 ~~ 100, 42)
        self.assertEqual(None ~~ 100, 100)
        self.assertEqual(False ~~ 100, False)
        self.assertEqual("" ~~ 100, "")

    def test_none_coalescing_single_evaluation(self):
        count = 0
        def get_val():
            nonlocal count
            count += 1
            return 42

        # Left hand side evaluated exactly once when not None
        self.assertEqual(get_val() ~~ 100, 42)
        self.assertEqual(count, 1)

        count = 0
        def get_none():
            nonlocal count
            count += 1
            return None

        # Left hand side evaluated exactly once when None
        self.assertEqual(get_none() ~~ 100, 100)
        self.assertEqual(count, 1)

    def test_none_coalescing_precedence_and_associativity(self):
        # Chains to expression (right-associative)
        # None ~~ None ~~ 42 => None ~~ (None ~~ 42) => 42
        self.assertEqual(None ~~ None ~~ 42, 42)
        self.assertEqual(1 ~~ 2 ~~ 3, 1)

        # Lower precedence than comparisons / disjunction
        # (5 > 3) ~~ 100 => True ~~ 100 => True
        self.assertEqual(5 > 3 ~~ 100, True)
        
        # None ~~ 5 + 5 => None ~~ 10 => 10
        self.assertEqual(None ~~ 5 + 5, 10)
