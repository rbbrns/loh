import unittest

class Tests(unittest.TestCase):
    def test_basic_property_assignment(self):
        class Person:
            pass

        p = Person() {
            .name = "Bob";
            .age = 42
        }
        self.assertEqual(p.name, "Bob")
        self.assertEqual(p.age, 42)

    def test_method_calls_and_mutations(self):
        class Counter:
            def __init__(self):
                self.count = 0
            def inc(self, n=1):
                self.count += n

        c = Counter() {
            .inc();
            .inc(5);
            .count += 2
        }
        self.assertEqual(c.count, 8)

    def test_returns_modified_receiver_object(self):
        class Item:
            pass

        x = Item()
        y = x {
            .a = 1
        }
        self.assertTrue(x is y)
        self.assertEqual(x.a, 1)

    def test_evaluated_exactly_once(self):
        class Helper:
            def __init__(self):
                self.val = 0

        eval_count = 0
        def get_helper():
            nonlocal eval_count
            eval_count += 1
            return Helper()

        h = get_helper() {
            .val = 100
        }
        self.assertEqual(eval_count, 1)
        self.assertEqual(h.val, 100)

    def test_delete_properties(self):
        class Data:
            def __init__(self):
                self.temp = 42

        d = Data() {
            <> .temp
        }
        self.assertFalse(hasattr(d, "temp"))

    def test_nested_initializer_blocks(self):
        class Inner:
            pass
        class Outer:
            pass

        o = Outer() {
            .inner = Inner() {
                .name = "Alice"
            };
            .other = .inner
        }
        self.assertEqual(o.inner.name, "Alice")
        self.assertEqual(o.other.name, "Alice")
        self.assertTrue(o.inner is o.other)

    def test_outer_scope_resolutions(self):
        class Person:
            pass

        name = "Alice"
        p = Person() {
            .name = name
        }
        self.assertEqual(p.name, "Alice")
