import unittest

class Tests(unittest.TestCase):
    def test_lazy_variable_caching_and_scoping(self):
        x = 10
        lazy_val = `x + 5`
        
        # Evaluates first time and resolves to 15
        self.assertEqual(lazy_val, 15)
        
        # Cached, so subsequent changes to outer variables do not affect it
        x = 20
        self.assertEqual(lazy_val, 15)

    def test_late_bound_parameter_defaults_basic(self):
        def add_ten(a, b = `a + 10`):
            return a + b
            
        self.assertEqual(add_ten(5), 20)
        self.assertEqual(add_ten(5, 2), 7)

    def test_late_bound_defaults_referencing_each_other(self):
        def calculate(width, height = `width * 2`, depth = `height * 3`):
            return width + height + depth
            
        self.assertEqual(calculate(10), 90)  # 10 + 20 + 60
        self.assertEqual(calculate(10, 5), 30)  # 10 + 5 + 15
        self.assertEqual(calculate(10, 5, 2), 17)  # 10 + 5 + 2

    def test_preserves_explicit_none_and_false(self):
        def configure(timeout = `30`, debug = `True`):
            return timeout, debug
            
        self.assertEqual(configure(None, False), (None, False))

    def test_method_receiver_dots(self):
        class Circle:
            def __init__(., radius):
                .radius = radius
            def diameter(., d = `.radius * 2`):
                return d
                
        c = Circle(5)
        self.assertEqual(c.diameter(), 10)
        self.assertEqual(c.diameter(12), 12)

    def test_proxy_attributes(self):
        class Person:
            def __init__(self, name):
                self.name = name
                
        lazy_person = `Person("Alice")`
        self.assertEqual(lazy_person.name, "Alice")
        
        lazy_person.name = "Bob"
        self.assertEqual(lazy_person.name, "Bob")

    def test_proxy_list_indexing_and_len(self):
        lazy_list = `[1, 2, 3]`
        self.assertEqual(len(lazy_list), 3)
        self.assertEqual(lazy_list[0], 1)
        self.assertEqual(lazy_list[1:3], [2, 3])
        
        lazy_list[0] = 10
        self.assertEqual(lazy_list[0], 10)
        
        del lazy_list[1]
        self.assertEqual(lazy_list, [10, 3])

    def test_proxy_iteration(self):
        lazy_list = `[10, 20, 30]`
        result = []
        for x in lazy_list:
            result.append(x)
        self.assertEqual(result, [10, 20, 30])

    def test_proxy_calling(self):
        lazy_func = `lambda x: x + 10`
        self.assertEqual(lazy_func(5), 15)

    def test_proxy_rich_comparisons(self):
        a = `5`
        b = `10`
        self.assertTrue(a < b)
        self.assertTrue(b > a)
        self.assertEqual(a, 5)
        self.assertNotEqual(a, b)
