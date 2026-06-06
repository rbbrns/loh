import unittest

class Base:
    base_class_val = 100  # class attribute

    def __init__(self, name):
        self.name = name

    def get_val(self):
        return self.base_class_val

    def get_greet(self, prefix):
        return f"{prefix} {self.name}"

class Sub(Base):
    def __init__(self, name, age):
        ..(name)  # calls parent constructor
        self.age = age

    def get_val(self):
        return ..get_val() + 10  # calls parent method

    def get_greet(self, prefix):
        return ..get_greet(prefix) + "!"  # calls parent method with args

    def get_base_class_val(self):
        return ..base_class_val  # accesses parent class attribute

    def try_set_base_val(self, val):
        ..base_class_val = val  # tries to set parent attribute

    def try_delete_base_val(self):
        del ..base_class_val  # tries to delete parent attribute

    def get_super_class(self):
        return ..  # standalone reference to super()

class Tests(unittest.TestCase):
    def test_super_constructor(self):
        s = Sub("Rob", 30)
        self.assertEqual(s.name, "Rob")
        self.assertEqual(s.age, 30)

    def test_super_method_calls(self):
        s = Sub("Rob", 30)
        self.assertEqual(s.get_val(), 110)
        self.assertEqual(s.get_greet("Hello"), "Hello Rob!")

    def test_super_class_attribute_access(self):
        s = Sub("Rob", 30)
        self.assertEqual(s.get_base_class_val(), 100)

    def test_super_instance_attribute_access_raises(self):
        s = Sub("Rob", 30)
        # super() doesn't look up instance attributes in standard Python
        with self.assertRaises(AttributeError):
            _ = s.get_super_class().name

    def test_super_attribute_assignment_raises(self):
        s = Sub("Rob", 30)
        with self.assertRaises(AttributeError):
            s.try_set_base_val(200)

    def test_super_attribute_deletion_raises(self):
        s = Sub("Rob", 30)
        with self.assertRaises(AttributeError):
            s.try_delete_base_val()

    def test_standalone_super(self):
        s = Sub("Rob", 30)
        sup = s.get_super_class()
        self.assertIsInstance(sup, super)
        self.assertEqual(sup.get_val(), 100)

if __name__ == "__main__":
    unittest.main()
