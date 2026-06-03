import unittest
from typing import Generator
import datetime

class TestReadmeExamples(unittest.TestCase):
    def test_logic_and_comparisons(self):
        x = True
        y = False
        z = None
        val = 1
        my_list = [1, 2, 3]

        ? (x === + && y !== -) || z === ~:
            <> val
        
        # Test not in
        x = 4
        ? x !<~ my_list:
            pass

    def test_conditionals(self):
        score = 85
        ? score > 90:
            grade = 'A'
        ?? score > 80:
            grade = 'B'
        ??:
            grade = 'C'
        self.assertEqual(grade, 'B')

        # Ternary expressions
        cond = True
        self.assertEqual(1 ? cond ?? 2, 1)
        self.assertEqual(1 ?! cond ?? 2, 2)
        self.assertEqual((1 ? cond), 1)
        self.assertEqual((1 ?! cond), None)

    def test_loops(self):
        total = 0
        $ i in range(10):
            ? i == 5:
                $<<
            ? i == 8:
                $>>
            total += i
        ?!$>>:
            self.fail("Loop should break and not execute else")

        # Alternative loops
        $ i := range(3):
            pass

        # Comprehensions
        evens = [i $ i <~ range(10) ? i % 2 == 0]
        self.assertEqual(evens, [0, 2, 4, 6, 8])

        data = [(1, 2, 3), (4, 5, 6, 7)]
        firsts = [first $ first, *rest <~ data]
        self.assertEqual(firsts, [1, 4])

    def test_functions_and_lambdas(self):
        # We define functions at local/method scope to keep it tidy
        calculate_total(price: float, tax: float) -> float:
            -> price * (1 + tax)
        self.assertEqual(calculate_total(100.0, 0.05), 105.0)

        countdown(n) -> Generator:
            $? n > 0:
                ~> n
                n -= 1
        self.assertEqual(list(countdown(3)), [3, 2, 1])

        # Lambda functions
        doubles = list(map((x) -> x * 2, [1, 2, 3]))
        self.assertEqual(doubles, [2, 4, 6])

        add = (x, y) -> x + y
        self.assertEqual(add(1, 2), 3)

        # Duplicate kwargs and collector
        def setup_config(name, **):
            **['name'] = name
            -> **
        
        self.assertEqual(setup_config("test", a=1, **{"a": 2}), {"name": "test", "a": 2})

    def test_classes(self):
        BaseAccount::
            pass

        Account:BaseAccount:
            .(.owner, .balance):
                pass

            .deposit(amount):
                .balance += amount
                -> .balance

        acc = Account("Alice", 100.0)
        self.assertEqual(acc.owner, "Alice")
        self.assertEqual(acc.deposit(50.0), 150.0)

    def test_exceptions_and_assertions(self):
        ran = False
        ^:
            result = 10 / 0
        ^? ZeroDivisionError => e:
            ran = True
        ^??:
            self.fail("Should have raised ZeroDivisionError")
        ^*:
            pass
        self.assertTrue(ran)

        # Legacy operator syntax (Option 2)
        ran = False
        ~^:
            result = 10 / 0
        ?^ ZeroDivisionError => e:
            ran = True
        ?!^:
            self.fail("Should have raised ZeroDivisionError")
        ?*:
            pass
        self.assertTrue(ran)

        # Raising exceptions
        with self.assertRaises(ValueError):
            ^^^ ValueError("Invalid code")

        with self.assertRaisesRegex(Exception, "Something went wrong"):
            ^^^ "Something went wrong"

        # Raising from
        error = ValueError()
        with self.assertRaises(Exception) as cm:
            ^^^ "Failed" from error
        self.assertIs(cm.exception.__cause__, error)

        # Assertions
        x = 15
        ^?! x > 10, "x is too small"

        # Assert Not
        x = False
        ^? x, "x must be False or None"

    def test_imports(self):
        # We test that the imports compile and run.
        /math
        self.assertTrue(math)

        /math => m
        self.assertTrue(m)

        /math/sqrt
        self.assertTrue(sqrt)

        /math/sqrt => s
        self.assertTrue(s)

        /math/sqrt, floor
        self.assertTrue(floor)

        /math/(sqrt, floor)

        /math, datetime
        self.assertTrue(datetime)

        # Relative imports are tested separately in test_relative_imports.py.
        # We verify syntax by testing compilation of the syntax string
        compile("/ . / helper", "<string>", "exec")
        compile("/ .. / helper", "<string>", "exec")

        /os/(path, listdir)
        self.assertTrue(path)
        self.assertTrue(listdir)

    def test_implicit_none(self):
        x = ~
        self.assertIsNone(x)

        y: float = ~
        self.assertIsNone(y)

        # Function with default None
        foo(a~, b~):
            -> a, b
        self.assertEqual(foo(), (None, None))

        # None comparisons
        x = None
        ran = False
        ? x is ~:
            ran = True
        self.assertTrue(ran)

        y = 1
        ran = False
        ? y !== ~:
            ran = True
        self.assertTrue(ran)

        z = None
        ran = False
        ? z == ~:
            ran = True
        self.assertTrue(ran)

        my_dict = {'a': ~, 'b': ~}
        self.assertEqual(my_dict, {'a': None, 'b': None})

    def test_implicit_assignments(self):
        # Argument shorthand
        foo(name, config):
            -> name, config

        name = "test"
        config = "cfg"
        self.assertEqual(foo(=name, =config), ("test", "cfg"))

        class Obj:
            value = 42
        obj = Obj()
        foo2(value):
            -> value
        self.assertEqual(foo2(=obj.value), 42)

        # Definition default from outer scope
        a = 100
        foo3(=a):
            -> a
        self.assertEqual(foo3(), 100)

        # Attribute statements
        class Config:
            verbose = True
        config = Config()
        =config.verbose
        self.assertTrue(verbose)

        # Bool defaults & statements
        foo4(a+, b-):
            -> a, b
        self.assertEqual(foo4(), (True, False))

        x+
        y-
        self.assertTrue(x)
        self.assertFalse(y)

    def test_dict_literals(self):
        my_dict = {x=10, y=20, z=~}
        self.assertEqual(my_dict, {'x': 10, 'y': 20, 'z': None})

    def test_pipe_operator(self):
        result = 5 |> (x) -> x + 1 |> (x) -> x * 2
        self.assertEqual(result, 12)

        processed = (
            "   my data string   "
            |> str.strip
            |> str.upper
            |> (s) -> s.replace(" ", "_")
        )
        self.assertEqual(processed, "MY_DATA_STRING")

    def test_none_operators(self):
        class User:
            def __init__(self, name):
                self.name = name
        user = User("Alice")
        none_user = None
        self.assertEqual(user~.name, "Alice")
        self.assertIsNone(none_user~.name)

        self.assertEqual(None ~~ "default", "default")
        self.assertEqual("value" ~~ "default", "value")

        data = {"items": [10, 20]}
        none_data = None
        self.assertEqual(data~["items"]~[1], 20)
        self.assertIsNone(none_data~["items"])

    def test_advanced_compiler_features(self):
        : IntOrFloat = int | float
        from typing import TypeAliasType
        self.assertIsInstance(IntOrFloat, TypeAliasType)

        # We test compilation of empty_none_str
        code = """from __future__ import empty_none_str\nstr_val = str(None)"""
        namespace = {}
        exec(code, namespace)
        self.assertEqual(namespace["str_val"], "")

    def test_range_literals(self):
        # Loop 0 to 9
        res = []
        $ i <~ 0..10:
            res.append(i)
        self.assertEqual(res, list(range(10)))

        # Check if value in range
        x = 50
        in_bounds = -
        ? x <~ 1..100:
            in_bounds = +
        self.assertTrue(in_bounds)

    def test_initializer_block(self):
        class RestaurantManager:
            .(.name):
                .items = []
                .active = -
            .add_menu_item(item_id, name, price, category):
                .items.append((item_id, name, price, category))
        
        manager = RestaurantManager("The Loh Bistro") {
            .add_menu_item(101, "Truffle Fries", 12.50, "appetizer");
            .active = +
        }
        self.assertEqual(manager.name, "The Loh Bistro")
        self.assertEqual(manager.active, True)
        self.assertEqual(manager.items, [(101, "Truffle Fries", 12.50, "appetizer")])

        class User:
            .(.name):
                .status = "inactive"
            .update_status(status):
                .status = status

        user_sent = None
        def send_email(u):
            nonlocal user_sent
            user_sent = u

        user = User("Bob")
        send_email(user {
            .name = "Alice";
            .update_status("active")
        })
        self.assertTrue(user_sent is user)
        self.assertEqual(user.name, "Alice")
        self.assertEqual(user.status, "active")

    def test_runtime_versioning(self):
        import sys
        self.assertEqual(sys.loh_version, "0.2.0")
        self.assertEqual(sys.loh_version_info, (0, 2, 0))

        if sys.loh_version_info >= (0, 2, 0):
            active = +
        else:
            active = True
        self.assertTrue(active)

    def test_auto_fstrings(self):
        code = r"""from __loh__ import auto_fstrings
name = "Loh"
version = "0.2.0"
message = "Welcome to {name} version {version}!"
regex = r"^\d{3}-\d{4}$"
normal = n"Normal string with {braces} and \n escape"
css = "div {{ color: red; }}"
"""
        scope = {}
        exec(code, {}, scope)
        self.assertEqual(scope["message"], "Welcome to Loh version 0.2.0!")
        self.assertEqual(scope["regex"], r"^\d{3}-\d{4}$")
        self.assertEqual(scope["normal"], "Normal string with {braces} and \n escape")
        self.assertEqual(scope["css"], "div { color: red; }")

if __name__ == "__main__":
    unittest.main()
