import unittest

class Foo:
    def __init__(self):
        self.prop = 10
        self.nested = None

class TestSafeAssign(unittest.TestCase):
    def test_safe_attribute_assignment_none(self):
        obj = None
        # Should do nothing and not raise exception
        obj~.prop = 42
        self.assertIsNone(obj)

    def test_safe_attribute_assignment_not_none(self):
        obj = Foo()
        obj~.prop = 42
        self.assertEqual(obj.prop, 42)

    def test_safe_subscript_assignment_none(self):
        arr = None
        # Should do nothing and not raise exception
        arr~[0] = 42
        self.assertIsNone(arr)

    def test_safe_subscript_assignment_not_none(self):
        arr = [10, 20]
        arr~[0] = 42
        self.assertEqual(arr[0], 42)
        arr~[1] = 99
        self.assertEqual(arr[1], 99)

    def test_nested_safe_assignment_none_root(self):
        user = None
        user~.profile~.address = "NYC"
        self.assertIsNone(user)

    def test_nested_safe_assignment_none_intermediate(self):
        user = Foo()
        user.profile = None
        user~.profile~.address = "NYC"
        self.assertIsNone(user.profile)

    def test_nested_safe_assignment_success(self):
        class Profile:
            def __init__(self):
                self.address = "Old"
        user = Foo()
        user.profile = Profile()
        user~.profile~.address = "NYC"
        self.assertEqual(user.profile.address, "NYC")

    def test_safe_augmented_assignment_none(self):
        obj = None
        obj~.prop += 5
        self.assertIsNone(obj)

    def test_safe_augmented_assignment_not_none(self):
        obj = Foo()
        obj~.prop += 5
        self.assertEqual(obj.prop, 15)

    def test_multiple_targets_single_evaluation(self):
        eval_count = 0
        def get_value():
            nonlocal eval_count
            eval_count += 1
            return 42

        x = 0
        y = Foo()
        
        # Multiple assignment with one safe target
        x = y~.prop = get_value()
        
        self.assertEqual(x, 42)
        self.assertEqual(y.prop, 42)
        self.assertEqual(eval_count, 1)

if __name__ == "__main__":
    unittest.main()
