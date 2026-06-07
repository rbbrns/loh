# **Loh: A Near-Superset of Python**

**Loh** is a syntax extension and near-superset of Python designed to make coding more concise, intuitive, and expressive. By replacing verbose keywords with symbol-based operators and introducing powerful syntactic sugar, Loh reduces boilerplate while keeping the full power of the Python ecosystem.

While Loh aims for maximum compatibility, it is **not a strict superset**. It intentionally breaks or repurposes a small number of redundant or obsolete Python syntax patterns (such as double bitwise NOT `~~x` or double logical NOT `!!x`) for its own symbol-based features, and adjusts certain parser edge cases (such as keyword-argument order restrictions). Any standard Python code that avoids these redundant patterns will compile and run identically under Loh.

---

## **Getting Started**

### **Building Loh from Source**
To compile the Loh interpreter:
```bash
./configure
make -j8 python.exe
make regen-all PYTHON_FOR_REGEN=./python.exe
make -j8

# Recommended: Create a symlink to use `loh` command
ln -s ./python.exe ./loh
```

### **Syntax Highlighting**
- **GitHub**: Highlighting is enabled automatically for `*.loh` files using standard Python/Linguist grammar mapping configured in `.gitattributes`.
- **VS Code**: Support is located in [Tools/vscode-loh-syntax](file:///Users/robbarnes/Development/loh/Tools/vscode-loh-syntax). To install:
  1. Open VS Code.
  2. Run the command `Extensions: Install from Location...` from the command palette.
  3. Select the `Tools/vscode-loh-syntax` directory.

---

## **Keyword & Symbol Mapping Cheat Sheet**

Loh maps Python's verbose keywords and structures to elegant, symbol-based alternatives:

| **Python** | **Loh** | **Description** |
| :--- | :--- | :--- |
| `True` | `+` | Boolean True constant |
| `False` | `-` | Boolean False constant |
| `None` | `~` | None constant |
| `x = True` | `x+` | Shorthand Boolean assignment / default parameter |
| `x = False` | `x-` | Shorthand Boolean assignment / default parameter |
| `x = None` | `x~` | Shorthand None assignment / default parameter |
| `x == True` | `x++` | Value equality comparison with True |
| `x === True` | `x+++` | Identity equality comparison with True |
| `x == False` | `x--` | Value equality comparison with False |
| `x === False` | `x---` | Identity equality comparison with False |
| `x == None` | `x~~` | Value equality comparison with None |
| `x === None` | `x~~~` | Identity equality comparison with None |
| None-safe member | `~.` | None-safe attribute access / navigation |
| None-coalesce | `~~` | None-coalescing operator (fallback value) |
| None-safe index | `~[]` | None-safe subscripting / indexing |
| `and` | `&&` | Logical AND |
| `or` | `\|\|` | Logical OR |
| `not` | `!` | Logical NOT |
| `is` | `===` | Identity comparison |
| `is not` | `!==` | Negated identity comparison |
| `in` | `<~` | Membership check |
| `not in` | `!<~` *(or `not <~`)*| Negated membership check |
| `range(start, stop)` | `start..stop` | Range / sequence literal |
| `del` | `<>` | Delete statement |
| `if` | `?` | Conditional branch |
| `elif` | `??` | Else-if branch |
| `else` | `??` | Else branch |
| `for` | `$` | For loop |
| `while` | `$?` | While loop |
| `break` | `$>` | Break statement |
| `continue` | `$<` | Continue statement |
| `try` | `^:` | Try block |
| `except` | `?^` | Except handler |
| `except*` | `?^*` | Except-star handler |
| `else` *(try)* | `?!^:` | Try-else block |
| `finally` | `*:` | Finally block |
| `as` | `=>` *(or `as`)* | Alias binding operator |
| `raise` | `^^^` | Raise exception |
| `assert` | `^?!` | Assert statement |
| `assert not` | `^?` | Negated assert statement |
| Inline exception rescue | `expr ?^ fallback` / `expr ?^ exc -> fallback` | Inline exception rescue expression |
| `with` | `&` | Context manager |
| `import` / `from` | `/` | Import symbol |
| `return` | `->` | Return statement |
| `yield` | `~>` | Yield statement |
| `async` / `await` | `%` | Asynchronous operations |
| `lambda` | `(args) -> expr` | Lambda function (arrow syntax) |
| `class` | `Name::` *(or `Name:Parent:`)*| Class declaration |
| `def` | *(omit)* | Function definition |
| `def __init__(self, ...)` | `.(...)` | Constructor shorthand |
| `self.param = param` | `.param` in signature | Parameter property binding |
| Parameter Keyword-Only Alias | `primary \| alias` | Keyword-only parameter alias (e.g. `limit \| l = 100`) |
| `super()` | `..` | Parent class reference shorthand |
| `type` | `:` | Type alias declaration |
| `match` | `?==` | Structural pattern matching subject |
| `case` | *(omit)* | Pattern case declaration |
| Inline initializer | `obj { .prop = val }` | Scope initializer block |
| Implicit f-strings | `from __loh__ import auto_fstrings` | Future import defaulting strings with braces to f-strings |
| Implicit returns | `from __loh__ import implicit_returns` | Future import returning the final expression of a function |
| None-safe assignment | `obj~.prop = value` | None-safe attribute/subscript assignment |
| Infinite loop | `$:` | Infinite loop shorthand (while True) |
| Multi-key slicing & unpacking | `*obj` / `**obj` / `*obj[keys]` / `**obj[keys]` | Multi-key subscript slicing, unpacking, and assignments |

---

## **Core Features & Syntax Guide**

### **1. Logic & Comparisons**

> **Motivation:** Symbols like `&&`, `||`, and strict identity `===` align Python with standard mathematical logic and modern programming conventions. Operators like `<~` (pointing into a collection) and `<>` (removal/deletion) use visual cues that match the developer's mental model of collection scanning and variable cleanup.

Loh provides sleek, compact symbols for logic, identity, and membership testing.

#### **Python**
```python
if (x is True and y is not False) or z is None:
    del val
if x not in my_list:
    pass
```

#### **Loh**
```python
? (x+++ && !y---) || z~~~:
    <> val
? x !<~ my_list:
    pass
```

---

### **2. Conditionals & If-Expressions**

> **Motivation:** The question mark `?` is the universal symbol for querying a state. Using `?` and `??` maps conditionals directly to decision-making flow. Ternary `x ? cond ?? y` aligns with the industry-standard conditional expression structure, while `x ? cond` provides a natural way to assign values that dynamically default to `None` if the condition isn't met.

Loh simplifies conditional logic by utilizing `?` for `if` and `??` for both `elif` and `else`.

#### **Python**
```python
if score > 90:
    grade = 'A'
elif score > 80:
    grade = 'B'
else:
    grade = 'C'

# Ternary expressions
x = a if cond else b
y = a if not cond else b
z = a if cond else None
```

#### **Loh**
```python
? score > 90:
    grade = 'A'
?? score > 80:
    grade = 'B'
??:
    grade = 'C'

# Ternary expressions
x = a ? cond ?? b
y = a ?! cond ?? b
z = a ? cond
```

---

### **3. Loops & Control Flow**

> **Motivation:** The iteration symbol `$` represents traversing a collection. Using `$>` (pointing forward/out) and `$<` (pointing backward/loop-start) matches the physical flow of breaking out of or looping back in control structures. The loop-else replacement `?!$>:` (literally "if not break") explicitly documents the execution path, resolving a common point of confusion in Python's standard `else:` loop syntax.

Loh loop grammar uses `$` for `for` loops, `$?` for `while` loops, `$>` for `break`, and `$<` for `continue`.

#### **Python**
```python
total = 0
for i in range(10):
    if i == 5:
        continue
    if i == 8:
        break
    total += i
else:
    print("Loop finished")
```

#### **Loh**
```python
total = 0
$ i in range(10):
    ? i == 5:
        $<
    ? i == 8:
        $>
    total += i
?!$>:
    print("Loop finished")
```


---

### **4. Comprehensions**

> **Motivation:** Comprehensions are pipelines that transform and filter collections. By utilizing the iteration symbol `$` and the filter query `?` within list/set brackets, comprehensions read like a structured query, separating the collection source from the filter condition.

#### **Python**
```python
# Alternative loop syntax and comprehensions
for i in range(10):
    pass
evens = [i for i in range(10) if i % 2 == 0]
firsts = [first for first, *rest in data]
```

#### **Loh**
```python
# Alternative loop syntax and comprehensions
$ i := range(10):
    pass
evens = [i $ i <~ range(10) ? i % 2 == 0]
firsts = [first $ first, *rest <~ data]
```

---

### **5. Function Definitions**

> **Motivation:** A function is defined by its signature and parameters, making the keyword `def` redundant. Using arrows for output flow—`->` for returning a final value and `~>` for yielding a stream of values—directly matches the mental model of data flowing out of a function block.

#### **Python**
```python
def calculate_total(price: float, tax: float) -> float:
    return price * (1 + tax)

def countdown(n):
    while n > 0:
        yield n
        n -= 1
```

#### **Loh**
```python
calculate_total(price: float, tax: float) -> float:
    -> price * (1 + tax)

countdown(n) -> Generator:
    $? n > 0:
        ~> n
        n -= 1
```

#### **Parameter Keyword-Only Multi-Name Aliases (`limit | l = 100`)**

> **Motivation:** APIs frequently evolve to require clearer parameter names, or benefit from short, convenient keyboard shortcuts (e.g. `l` for `limit`). By allowing parameter alias syntax `primary | alias`, Loh automatically handles the routing: both names are validated (raising a conflict `TypeError` if both are supplied), the value is resolved to the primary name, and the alias name is removed from the local scope so it does not clutter the function's variables.

##### **Python**
```python
def query(limit=100, **kwargs):
    # Manually resolve aliases in python
    if 'l' in kwargs:
        if 'limit' in kwargs or limit != 100:
            raise TypeError("got multiple values for alias parameter 'limit'/'l'")
        limit = kwargs.pop('l')
    return limit
```

##### **Loh**
```python
query(limit | l = 100):
    # Loh automatically desugars and resolves 'l' to 'limit', then deletes 'l'
    -> limit
```

---

### **6. Arrow Lambdas**

> **Motivation:** Lambda functions represent a direct mapping of inputs to an output. The arrow notation `(args) -> expr` aligns Python's anonymous functions with standard mathematical functions and modern arrow conventions, making inline callbacks much more intuitive to read.

#### **Python**
```python
map(lambda x: x * 2, [1, 2, 3])
add = lambda x, y: x + y
```

#### **Loh**
```python
map((x) -> x * 2, [1, 2, 3])
add = (x, y) -> x + y
```

---

### **7. Keyword Arguments & Kwarg Collection (`**`)**

> **Motivation:** A double asterisk `**` is Python's native symbol for dictionary unpacking. Binding the kwarg collector directly to `**` inside a function body allows you to work with the kwargs dictionary using the symbol that defined it, removing the need to invent and write arbitrary variable names like `kwargs`.

#### **Python**
```python
# Standard Python raises TypeError for duplicate keys
# and requires naming the kwargs collector (e.g. **kwargs)
def setup_config(name, **kwargs):
    kwargs['name'] = name
    return kwargs

setup_config("test", **{"a": 1, "b": 2})
```

#### **Loh**
```python
# Loh allows runtime overriding and binds local collector to `**`
setup_config(name, **):
    **['name'] = name
    -> **

setup_config("test", a=1, **{"a": 2})  # overrides 'a' to 2
```

---

### **8. Classes, Constructors, & Parameter Properties**

> **Motivation:** Method and attribute declarations inside a class are inherently bound to the instance namespace. Using a prefix dot (`.method` or `.attribute`) matches the intuition of accessing member properties of the current object. Furthermore, defining class constructors using `.(...)` and prefixing parameter properties with a dot (`.param`) automates attribute assignment and eliminates the repetitive typing of `self` and constructor boilerplates.

#### **Python**
```python
class Account(BaseAccount):
    def __init__(self, owner, balance):
        self.owner = owner
        self.balance = balance

    def deposit(self, amount):
        self.balance += amount
        return self.balance
```

#### **Loh**
```python
Account:BaseAccount:
    # Constructor shorthand with parameter properties
    .(.owner, .balance):
        pass

    .deposit(amount):
        .balance += amount
        -> .balance
```
*Note: A class with no base classes can be declared using `MyClass::`.*

#### **Parent Class Shorthand (`..`)**

Loh provides `..` as a prefix/standalone shorthand for referencing the parent class context (`super()`):
- `..` (standalone) $\rightarrow$ `super()`
- `..field` $\rightarrow$ `super().field`
- `..method(args)` $\rightarrow$ `super().method(args)`
- `..(args)` $\rightarrow$ `super().__init__(args)` (parent constructor call)

##### **Python**
```python
class SavingsAccount(Account):
    def __init__(self, owner, balance, interest_rate):
        super().__init__(owner, balance)
        self.interest_rate = interest_rate

    def get_details(self):
        base_details = super().get_details()
        return f"{base_details}, Rate: {self.interest_rate}"
```

##### **Loh**
```python
SavingsAccount:Account:
    .(owner, balance, .interest_rate):
        ..(owner, balance)  # calls parent constructor

    .get_details():
        base_details = ..get_details()  # calls parent method
        -> f"{base_details}, Rate: {.interest_rate}"
```

---

### **9. Type Aliases**

> **Motivation:** The colon `:` is the universal symbol for type annotations in Python. Using `:` to declare a type alias (e.g. `: MyType = int`) maintains semantic consistency with standard type annotation patterns, making type alias statements immediately recognizable.

#### **Python**
```python
type IntOrFloat = int | float
```

#### **Loh**
```python
: IntOrFloat = int | float
```

---

### **10. Exceptions & Try-Except-Finally**

> **Motivation:** Exception handling is a control flow structure designed to catch errors. The caret symbol `^` represents entering a guarded try block, while `?^` queries for matching exceptions. `?!^:` acts as try-else block (if no exception occurred), and `*:` represents finally block (wildcard cleanup). Raising string literals directly (`^^^ "error"`) simplifies throwing standard exceptions, removing the boilerplate of instantiating exception classes for simple error messages.

#### **Python**
```python
try:
    result = 10 / 0
except ZeroDivisionError as e:
    print(f"Error: {e}")
else:
    print("Success")
finally:
    print("Cleanup")

# Raising Exceptions
raise ValueError("Invalid code")
raise Exception("Something went wrong")
raise Exception("Failed") from error
```

#### **Loh**
```python
^:
    result = 10 / 0
?^ ZeroDivisionError => e:
    print(f"Error: {e}")
?!^:
    print("Success")
*:
    print("Cleanup")

# Raising Exceptions
^^^ ValueError("Invalid code")
^^^ "Something went wrong"
^^^ "Failed" from error
```

#### **Inline Exception Rescue Expressions**

Loh provides a concise, inline syntax for exception handling. This allows developers to catch standard or specific exceptions and supply a fallback value directly in expression contexts:

##### **Python**
```python
# Rescue any exception (standard except Exception)
try:
    value = int(x)
except Exception:
    value = 0

# Rescue specific exceptions
try:
    value = int(x)
except ValueError:
    value = -1

# Rescue with exception binding
try:
    value = int(x)
except ValueError as e:
    value = len(e.args)
```

##### **Loh**
```python
# Rescue any exception (defaults to catching Exception)
value = int(x) ?^ 0

# Rescue specific exception(s)
value = int(x) ?^ ValueError -> -1
value = int(x) ?^ (ValueError | TypeError) -> -1

# Rescue with variable binding
value = int(x) ?^ (ValueError => e) -> len(e.args)
```

---

### **11. Assertions & Assert Not**

> **Motivation:** Assertions act as guard rails at the entry or exit of code paths. The caret symbol `^?!` (assert) and its negated counterpart `^?` (assert not) act as visual pointers that guard the execution flow, making validation checks concise and easily distinguishable from standard logic.

#### **Python**
```python
assert x > 10, "x is too small"
assert not x, "x must be False or None"
```

#### **Loh**
```python
^?! x > 10, "x is too small"
^? x, "x must be False or None"
```

---

### **12. Module Imports & Aliasing**

> **Motivation:** Python modules are physically stored in a nested directory layout. The forward slash `/` syntax maps imports directly to file paths (e.g., `/math/sqrt`), making module discovery and relative imports (`/ . / helper`) align with standard directory navigation conventions.

Loh converts all import syntax to use forward slashes (`/`), mimicking filesystem paths. Aliasing uses `=>`.

#### **Python**
```python
import math
import math as m
from math import sqrt
from math import sqrt as s
from math import sqrt, floor
from . import helper
from .. import helper
```

#### **Loh**
```python
/math
/math => m
/math/sqrt
/math/sqrt => s
/math/sqrt, floor
/ . / helper
/ .. / helper
```

#### **Import Reference Mapping**

| **Python** | **Loh** |
| :--- | :--- |
| `import math` | `/math` |
| `import math as m` | `/math => m` |
| `from math import sqrt` | `/math/sqrt` |
| `from math import sqrt as s` | `/math/sqrt => s` |
| `from math import sqrt, floor` | `/math/sqrt, floor` *(or `/math/(sqrt, floor)`)*|
| `import math, datetime` | `/math, datetime` |
| `from . import helper` | `/ . / helper` |
| `from .. import helper` | `/ .. / helper` |

---

### **13. None Constants & Shorthands**

> **Motivation:** Using verbose keywords like `None` clutters declarations. Loh introduces `~` as a concise constant for `None`, along with postfix `~~` and `~~~` comparisons and shorthand parameter/assignment formats to streamline handling of missing values.

#### **Python**
```python
x = None
y: float = None

def foo(a=None, b=None):
    pass

if x is None:
    pass
if y is not None:
    pass
if z == None:
    pass

my_dict = {'a': None, 'b': None}
```

#### **Loh**
```python
x = ~         # Or: x~
y: float = ~  # Or: y: float = ~

foo(a~, b~):
    ...

? x~~~:      # Or: ? x === ~:
    pass
? !y~~~:     # Or: ? y !== ~:
    pass
? z~~:       # Or: ? z == ~:
    pass

my_dict = {'a': ~, 'b': ~}
```

---

### **14. Implicit Parameter & Argument Mapping**

> **Motivation:** When passing variables to functions, the parameter name and the variable name are frequently identical (e.g. `foo(config=config)`). The `=name` syntax explicitly captures this intent by binding the parameter to the local variable of the same name, removing redundant declarations.

#### **Python**
```python
# Function Definition Default
def foo(a=a, b=b):
    pass

# Function Call Shorthand
foo(name=name, config=config)
foo(value=obj.value)
```

#### **Loh**
```python
# Function Definition Default
foo(=a, =b):
    ...

# Function Call Shorthand
foo(=name, =config)
foo(=obj.value)
```

---

### **15. Implicit Attribute Binding**

> **Motivation:** Extracting an object's attribute into a local variable of the same name (e.g. `verbose = config.verbose`) is a repetitive task. The prefix assignment `=config.verbose` automates this mapping, declaring a local variable named after the attribute.

#### **Python**
```python
verbose = config.verbose
```

#### **Loh**
```python
=config.verbose
```

---

### **16. Boolean Shorthands & Postfix Comparisons**

> **Motivation:** Declaring and checking boolean state is one of the most common tasks. In Loh, standard boolean constants are represented by `+` (`True`) and `-` (`False`). Using postfix shorthands (`x+` and `y-`) allows assigning boolean values or setting parameter defaults extremely compactly. Postfix operators `++`/`+++` and `--`/`---` allow quick check for value or identity equality.

#### **Python**
```python
def foo(a=True, b=False):
    pass

x = True
y = False

if x is True:
    pass
if y == False:
    pass
```

#### **Loh**
```python
foo(a+, b-):
    ...

x+
y-

? x+++:
    pass
? y--:
    pass
```

---

### **17. Dict Literals Keyword-Style Syntax**

> **Motivation:** Dictionary literals function as collections of named attributes. Using keyword-style assignments (`{x=10}`) maps key-value definition to standard function call conventions, treating dictionary keys as named parameters and eliminating quote clutter.

Instead of standard string mapping, dictionary literals can accept keyword-style assignments:

#### **Python**
```python
my_dict = {'x': 10, 'y': 20, 'z': None}
```

#### **Loh**
```python
my_dict = {x=10, y=20, z=}
```

---

### **18. The Pipe Operator (`|>`)**

> **Motivation:** Data processing is a sequential flow of transformations. The pipe operator `|>` allows you to chain function calls in the order they occur (from left to right), matching the developer's mental model of passing data through a pipeline rather than reading nested functions inside-out.

Loh features a pipe operator to feed expressions into callable objects. `x |> f` evaluates to `f(x)`. It has lower precedence than standard arithmetic and chains from left to right.

#### **Python**
```python
# Nested execution
result = double(add_one(5))

# Sequential execution
processed = str.upper(str.strip("   my data string   ")).replace(" ", "_")
```

#### **Loh**
```python
# Nested execution
result = 5 |> (x) -> x + 1 |> (x) -> x * 2

# Sequential execution
processed = (
    "   my data string   "
    |> str.strip
    |> str.upper
    |> (s) -> s.replace(" ", "_")
)
```

---

### **19. The `empty_none_str` Future Import**

> **Motivation:** When formatting user-facing text, empty values should naturally display as blank spaces rather than the word `"None"`. The `empty_none_str` future import ensures that string representation of missing values behaves intuitively, defaulting to an empty string `""` without requiring manual fallback checks.

Importing the future flag `empty_none_str` modifies standard Python behavior so that `str(None)` returns an empty string `""` instead of `"None"`:

#### **Python**
```python
print(str(None))  # Outputs: "None"
```

#### **Loh**
```python
from __future__ import empty_none_str
print(str(None))  # Outputs: ""
```

---

### **20. None-Safe Operators (`~.`, `~~`, `~[]`)**

> **Motivation:** Accessing attributes or indexing nested data structures that might contain `None` often requires verbose inline checks or deep conditional branching. Loh introduces three None-safe operators to streamline safe navigation, indexing, and default-value fallback handling without repeated evaluations or boilerplate.

Loh supports:
1. **Safe Navigation (`~.`)**: `obj~.attr` accesses `attr` if `obj` is not `None`, evaluating to `None` otherwise.
2. **None-Coalescing (`~~`)**: `left ~~ right` evaluates to `left` if it is not `None`, evaluating to `right` otherwise.
3. **Safe Subscripting (`~[]`)**: `obj~[index]` accesses the element or key at `index` if `obj` is not `None`, evaluating to `None` otherwise.

All three operators evaluate their left-hand side expression exactly once.

#### **Python**
```python
# Safe navigation
user_name = user.name if user is not None else None

# None-coalescing
display_name = user_name if user_name is not None else "Guest"

# Safe subscripting
first_item = data['items'][0] if data is not None else None
```

#### **Loh**
```python
# Safe navigation
user_name = user~.name

# None-coalescing
display_name = user_name ~~ "Guest"

# Safe subscripting
first_item = data~['items']~[0]
```

---

### **21. Range / Slice Literals (`..`)**

> **Motivation:** Standard Python relies on `range(start, stop)` for iteration and sequences. In mathematical notations and languages like Rust or Ruby, range/interval literals (`1..10`) are used to represent sequences cleanly. Adding this to Loh makes loop variables and slice checks extremely compact.

Loh introduces range literals using the `..` operator, which translates directly to standard `range()` calls.

#### **Python**
```python
# Loop 0 to 9
for i in range(0, 10):
    print(i)

# Check if value in range
if x in range(1, 100):
    print("In bounds")
```

#### **Loh**
```python
# Loop 0 to 9
$ i <~ 0..10:
    print(i)

# Check if value in range
? x <~ 1..100:
    print("In bounds")
```

---

### **22. Inline Scope Initializer Block (`obj { ... }`)**

> **Motivation:** Configuring or modifying objects upon construction or inline inside pipelines typically requires separate statement-level assignments. An inline scope initializer block allows properties to be assigned and methods to be invoked using Loh's leading-dot receiver context, returning the configured receiver object.

Loh supports appending `{ ... }` blocks to primary expressions (constructor calls, variables, function returns), which desugar to evaluate the receiver exactly once, execute the nested statements in-place, and return the modified object.

#### **Python**
```python
# Initialization on construction
manager = RestaurantManager("The Loh Bistro")
manager.add_menu_item(101, "Truffle Fries", 12.50, "appetizer")
manager.active = True

# In-place configuration passed directly to a function
user.name = "Alice"
user.update_status("active")
send_email(user)
```

#### **Loh**
```python
# Initialization on construction
manager = RestaurantManager("The Loh Bistro") {
    .add_menu_item(101, "Truffle Fries", 12.50, "appetizer");
    .active = +
}

# In-place configuration passed directly to a function
send_email(user {
    .name = "Alice";
    .update_status("active")
})
```

---

### **23. Runtime Versioning (`sys.loh_version`, `sys.loh_version_info`)**

> **Motivation:** As Loh evolves and introduces breaking changes, downstream tooling, compilers, and consumers need a reliable way to query the language version at runtime to adjust behavior or assert compatibility.

Loh exposes version metadata attributes via the standard `sys` module, allowing dynamic version checking and feature detection:

- **`sys.loh_version`**: A string representing the semantic version of Loh (e.g., `"0.2.0"`).
- **`sys.loh_version_info`**: A tuple of `(major, minor, micro)` integers representing the Loh version components (e.g., `(0, 2, 0)`).

#### **Example**
```python
import sys

if sys.loh_version_info >= (0, 2, 0):
    # Use unified +, -, ~ syntax
    active = +
else:
    # Use legacy ++ / -- syntax
    active = True
```

---

### **24. The `auto_fstrings` Future Import**

> **Motivation:** Writing `f"..."` for every string interpolation is verbose and can lead to bugs when developers forget the `f` prefix. The `auto_fstrings` future import in Loh allows standard string literals containing braces `{}` to automatically compile as f-strings.

By importing `auto_fstrings` from the `__loh__` (or standard `__future__`) module, standard string literals containing any braces `{` or `}` are processed as f-strings at compile-time.

- **Explicit raw string bypass**: Standard raw string literals (`r"..."` / `R"..."`) containing braces remain literal plain strings and are never converted.
- **Explicit normal string prefix**: Standard normal string literals (`n"..."` / `N"..."`) bypass automatic f-string conversion while preserving standard backslash escape processing.
- **Docstring protection**: The first string literal of a module, class, or function body is protected and remains a static string constant.
- **Brace escaping**: Doubled braces `{{` and `}}` are correctly recognized as escaped braces and evaluate to single literal braces `{` and `}`.

#### **Example**
```python
from __loh__ import auto_fstrings

name = "Loh"
version = "0.2.0"

# Automatically treated as an f-string:
message = "Welcome to {name} version {version}!"
print(message)  # Outputs: "Welcome to Loh version 0.2.0!"

# Raw strings are bypassed and remain plain:
regex = r"^\d{3}-\d{4}$"

# Normal strings bypass conversion but preserve escapes:
normal = n"Normal string with {braces} and \n escape"

# Escaped braces evaluate to literal braces:
css = "div {{ color: red; }}"  # Evaluates to: "div { color: red; }"
```

---

### **25. None-Safe Attribute Assignment**

> **Motivation:** Safe navigation `user~.profile~.address` protects against attribute reads crashing on `None` values. However, trying to assign to a nested property where a parent might be `None` still results in a traceback. Applying safe navigation to assignment allows writing values safely to nested properties, silently short-circuiting and doing nothing if any parent object in the chain is `None`.

Loh extends the safe navigation operators `~.` and `~[]` to target assignment. When used on the left-hand side of an assignment or augmented assignment statement, it will evaluate the chain and execute the assignment only if all intermediate objects are not `None`.

#### **Example**
```python
# Silently does nothing if user or profile is None
user~.profile~.address = "NYC"

# Safe subscript assignment
arr~[0] = 42

# Safe augmented assignment
user~.score += 10
```

---

### **26. Infinite Loops Shorthand**

> **Motivation:** Python lacks a dedicated infinite loop construct, requiring `while True:`. Loh uses the loop sigil `$` alone followed by `:` to represent an infinite loop.

Loh compiles `$` followed directly by `:` into a standard `while True:` loop.

#### **Example**
```python
$:
    # Loop runs forever until broken
    print("Processing...")
    if should_stop:
        break
```

---

### **27. Multi-Key Subscript Slicing and Unpacking (`*` / `**`)**

> **Motivation:** Extracting subsets of dictionary keys, converting lists to index-value mappings, and retrieving values/keys as lists are common but verbose tasks. By using the `*` (sequence/list) and `**` (mapping/dictionary) prefix operators in combination with subscripts or standalone targets, Loh provides a clean and powerful syntax that is 100% backward compatible with standard Python's tuple-subscript conflicts.

Loh defines the following behaviors for `*` and `**` on the RHS (evaluating) and LHS (assigning), both with subscripts (`d[keys]`) and without brackets:

#### **Dictionary Targets**
* **Bracketless / Standalone**:
  - `*d` (RHS) $\rightarrow$ Returns a list of all dictionary keys (equivalent to `list(d.keys())`).
  - `*d = [10, 20]` (LHS) $\rightarrow$ Assigns sequence values to existing keys positionally in-place.
  - `**d` (RHS) $\rightarrow$ Returns a shallow copy of the dictionary (equivalent to `dict(d)`).
  - `**d = {"a": 10, "b": 20}` (LHS) $\rightarrow$ Merges/updates the dictionary in-place (equivalent to `d.update(...)`).
* **Subscripted (`d[keys]`)**:
  - `*d['a', 'b']` (RHS) $\rightarrow$ Extracts specified values into a list: `[d['a'], d['b']]`.
  - `*d['a', 'b'] = [10, 20]` (LHS) $\rightarrow$ Assigns values to specified keys positionally in-place: `d['a'] = 10; d['b'] = 20`.
  - `**d['a', 'b']` (RHS) $\rightarrow$ Extracts specified key-value pairs: `{"a": d['a'], "b": d['b']}`.
  - `**d['a', 'b'] = {"a": 10, "b": 20}` (LHS) $\rightarrow$ Copies matching key-value pairs in-place: `d['a'] = 10; d['b'] = 20`.
* **Empty Subscript (`d[]`)**:
  - `d[]` (RHS) $\rightarrow$ Returns the dynamic values view: `d.values()`.
  - `*d[]` (RHS) $\rightarrow$ Extracts all values into a list: `list(d.values())`.

#### **List / Tuple Targets**
* **Bracketless / Standalone**:
  - `*lst` (RHS) $\rightarrow$ Returns a shallow copy of the list (equivalent to `list(lst)`).
  - `*lst = [10, 20]` (LHS) $\rightarrow$ Overwrites the entire list content in-place (equivalent to `lst[:] = [10, 20]`).
  - `**lst` (RHS) $\rightarrow$ Converts the list to an index-to-value dictionary: `{i: v for i, v in enumerate(lst)}`.
  - `**lst = {0: 10, 2: 30}` (LHS) $\rightarrow$ Updates elements at specified index keys in-place: `lst[0] = 10; lst[2] = 30`.
* **Subscripted (`lst[indices]` / `lst[slice]`)**:
  - `*lst[0, 1]` (RHS) $\rightarrow$ Extracts values at specified indices: `[lst[0], lst[1]]`.
  - `*lst[0, 1] = [10, 20]` (LHS) $\rightarrow$ Assigns values to specified indices positionally: `lst[0] = 10; lst[1] = 20`.
  - `**lst[0, 1]` (RHS) $\rightarrow$ Extracts indices and values: `{0: lst[0], 1: lst[1]}`.
  - `**lst[0, 1] = {0: 10, 1: 20}` (LHS) $\rightarrow$ Assigns values to specified indices: `lst[0] = 10; lst[2] = 20`.
  - `**lst[0:2]` (RHS) $\rightarrow$ Converts a list slice to an index-to-value dictionary.
* **Empty Subscript (`lst[]`)**:
  - `lst[]` (RHS) $\rightarrow$ Returns a shallow copy of the list (equivalent to `lst[:]`).
  - `*lst[]` (RHS) $\rightarrow$ Returns a shallow copy of the list.

#### **Example**
```python
d = {x=1, y=2}

# Standalone RHS unpacking
keys = *d        # ['x', 'y']
copy = **d       # {'x': 1, 'y': 2}

# Subscripted RHS unpacking
values = *d['x', 'y']  # [1, 2]
subdict = **d['x']      # {'x': 1}

# LHS assignments
*d['x', 'y'] = [10, 20]  # d becomes {'x': 10, 'y': 20}
```

---

### **28. The `implicit_returns` Future Import**

> **Motivation:** Writing explicit return statements (`return` or `->` in Loh) in short helper functions or lambda-like standard methods adds visual noise. Expression-oriented languages like Rust and Ruby automatically return the last evaluated expression in a function body. Enabling this behavior via a future import makes Loh functions extremely clean and concise.

By importing `implicit_returns` from the `__loh__` (or standard `__future__`) module, the compiler automatically desugars the final statement of a function body (both synchronous and asynchronous) into a `Return` statement if it is an expression statement (`Expr` node).

* **Docstring protection**: A function containing only a docstring (a single string literal constant) is protected and will not wrap it in a return statement, preserving correct docstring semantics.
* **Control flow and assignments**: Control flow keywords (like `pass`, `break`, `continue`, `raise`, etc.) and variable assignments (`x = y`) are not expressions and thus remain unaffected.

#### **Example**
```python
from __loh__ import implicit_returns

# Automatically returns base * (1 + tax)
calculate_total(base, tax):
    rate = 1 + tax
    base * rate

# Async functions are also supported
async fetch_data(url):
    await request(url)
```





