import unittest

class Tests(unittest.TestCase):
    def test_double_plus_equal_variable(self):
        # When target is True or equals True (like 1), keep it
        x = True
        x ++= "fallback"
        self.assertEqual(x, True)

        x = 1
        x ++= "fallback"
        self.assertEqual(x, 1)

        # When target does not equal True, assign fallback
        x = False
        x ++= "fallback"
        self.assertEqual(x, "fallback")

        x = None
        x ++= "fallback"
        self.assertEqual(x, "fallback")

        x = "hello"
        x ++= "fallback"
        self.assertEqual(x, "fallback")

    def test_double_minus_equal_variable(self):
        # When target is False or equals False (like 0), keep it
        x = False
        x --= "fallback"
        self.assertEqual(x, False)

        x = 0
        x --= "fallback"
        self.assertEqual(x, 0)

        # When target does not equal False, assign fallback
        x = True
        x --= "fallback"
        self.assertEqual(x, "fallback")

        x = None
        x --= "fallback"
        self.assertEqual(x, "fallback")

        x = "hello"
        x --= "fallback"
        self.assertEqual(x, "fallback")

    def test_subscript_target(self):
        lst = [True, False, 1, 0, "hello", "world"]
        
        # Test ++= on subscript
        lst[0] ++= "fallback"
        self.assertEqual(lst[0], True)
        
        lst[4] ++= "fallback"
        self.assertEqual(lst[4], "fallback")

        # Test --= on subscript
        lst[1] --= "fallback"
        self.assertEqual(lst[1], False)
        
        lst[5] --= "fallback"
        self.assertEqual(lst[5], "fallback")

    def test_attribute_target(self):
        class Holder:
            def __init__(self):
                self.val = True

        h = Holder()
        h.val ++= "fallback"
        self.assertEqual(h.val, True)

        h.val = "not_true"
        h.val ++= "fallback"
        self.assertEqual(h.val, "fallback")
