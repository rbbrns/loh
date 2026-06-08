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
                $<
            ? i == 8:
                $>
            total += i
        ?!$>:
            self.fail("Loop should break and not execute else")

        # Alternative loops
        $ i := range(3):
            pass

        # Implicit loops
        items = [1, 2, 3]
        res = []
        $ <~ items:
            res.append($)
        self.assertEqual(res, [1, 2, 3])

        # Loop filters
        res2 = []
        $ item <~ items ? item > 1:
            res2.append(item)
        self.assertEqual(res2, [2, 3])

        # Combined implicit and filter
        res3 = []
        $ <~ items ? $ > 2:
            res3.append($)
        self.assertEqual(res3, [3])

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

        # Parameter aliases
        my_func(limit | l = 100):
            -> limit
        self.assertEqual(my_func(42), 42)
        self.assertEqual(my_func(limit=50), 50)
        self.assertEqual(my_func(l=60), 60)
        self.assertEqual(my_func(), 100)


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

        SavingsAccount:Account:
            .(owner, balance, .interest_rate):
                ..(owner, balance)  # calls parent constructor

            .deposit(amount):
                -> ..deposit(amount)  # calls parent method

            .get_details():
                -> f"{.owner}, Rate: {.interest_rate}"

        sav = SavingsAccount("Bob", 200.0, 0.05)
        self.assertEqual(sav.owner, "Bob")
        self.assertEqual(sav.balance, 200.0)
        self.assertEqual(sav.interest_rate, 0.05)
        self.assertEqual(sav.deposit(50.0), 250.0)
        self.assertEqual(sav.get_details(), "Bob, Rate: 0.05")

    def test_exceptions_and_assertions(self):
        ran = False
        ^:
            result = 10 / 0
        ?^ ZeroDivisionError => e:
            ran = True
        ?!^:
            self.fail("Should have raised ZeroDivisionError")
        *:
            pass
        self.assertTrue(ran)

        # Legacy operator syntax
        ran = False
        ^:
            result = 10 / 0
        ?^ ZeroDivisionError => e:
            ran = True
        ?!^:
            self.fail("Should have raised ZeroDivisionError")
        *:
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

    def test_dict_literals_removed_syntax_error(self):
        with self.assertRaises(SyntaxError):
            compile("{x=10}", "<string>", "eval")

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

    def test_safe_attribute_assignment(self):
        code = """
class Profile:
    def __init__(self):
        self.address = "Old"
class User:
    def __init__(self):
        self.profile = Profile()
        self.score = 0

user = User()
user~.profile~.address = "NYC"
user~.score += 10

arr = [10]
arr~[0] = 42

none_user = None
none_user~.profile~.address = "LA"
"""
        scope = {}
        exec(code, scope)
        self.assertEqual(scope["user"].profile.address, "NYC")
        self.assertEqual(scope["user"].score, 10)
        self.assertEqual(scope["arr"][0], 42)
        self.assertIsNone(scope["none_user"])

    def test_infinite_loop_shorthand(self):
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

    def test_multi_key_subscript_slicing_and_unpacking(self):
        code = """
d = {'x': 1, 'y': 2}

# Standalone RHS unpacking
keys = *d        # ['x', 'y']
copy = **d       # {'x': 1, 'y': 2}

# Subscripted RHS unpacking
values = *d['x', 'y']  # [1, 2]
subdict = **d['x']      # {'x': 1}

# LHS assignments
*d['x', 'y'] = [10, 20]  # d becomes {'x': 10, 'y': 20}
"""
        scope = {}
        exec(code, {}, scope)
        self.assertEqual(scope["keys"], ['x', 'y'])
        self.assertEqual(scope["copy"], {'x': 1, 'y': 2})
        self.assertEqual(scope["values"], [1, 2])
        self.assertEqual(scope["subdict"], {'x': 1})
        self.assertEqual(scope["d"], {'x': 10, 'y': 20})

    def test_implicit_returns(self):
        code = """from __loh__ import implicit_returns
 
calculate_total(base, tax):
    rate = 1 + tax
    base * rate
"""
        scope = {}
        exec(code, {}, scope)
        self.assertEqual(scope["calculate_total"](100, 0.05), 105.0)

    def test_logical_assignments(self):
        code = """
x = 0
x ||= 42  # x becomes 42 (since 0 is falsy)

y = 100
y &&= 200 # y becomes 200 (since 100 is truthy)
"""
        scope = {}
        exec(code, {}, scope)
        self.assertEqual(scope["x"], 42)
        self.assertEqual(scope["y"], 200)

    def test_boolean_strict_fallback_assignments(self):
        code = """
x = 1
x ++= "fallback"  # x remains 1 (since 1 == True is True in Python)

y = "hello"
y ++= "fallback"  # y becomes "fallback" (since "hello" == True is False)

z = 0
z --= "fallback"  # z remains 0 (since 0 == False is True in Python)
"""
        scope = {}
        exec(code, {}, scope)
        self.assertEqual(scope["x"], 1)
        self.assertEqual(scope["y"], "fallback")
        self.assertEqual(scope["z"], 0)

    def test_presence_postfix_operators(self):
        code = """
x = 42
present = x!~~

class EqualNone:
    __eq__(self, other):
        -> other === ~

obj = EqualNone()
val_present = obj!~
id_present = obj!~~
"""
        scope = {}
        exec(code, {}, scope)
        self.assertTrue(scope["present"])
        self.assertFalse(scope["val_present"])
        self.assertTrue(scope["id_present"])

    def test_truthy_coalescing_operator(self):
        code = """
x = 0
fallback = x ?? 42          # fallback is 42

name = "Loh"
result = name ?? "default"  # result is "Loh"

# Does not conflict with standard ternary: true_val ? condition ?? else_val
x = 10 ? True ?? 20         # x is 10
"""
        scope = {}
        exec(code, {}, scope)
        self.assertEqual(scope["fallback"], 42)
        self.assertEqual(scope["result"], "Loh")
        self.assertEqual(scope["x"], 10)

if __name__ == "__main__":
    unittest.main()


