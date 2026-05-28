# **Loh: A Superset of Python**

**Loh** is a syntax extension and superset of Python designed to make coding more concise, intuitive, and expressive. By replacing verbose keywords with symbol-based operators and introducing powerful syntactic sugar, Loh reduces boilerplate while keeping the full power of the Python ecosystem.

Since Loh is a strict superset, **any valid Python code is also valid Loh code**.

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
| `True` | `++` | Boolean True constant |
| `False` | `--` | Boolean False constant |
| `None` | `~` *(or omit)* | None constant (empty space represents None) |
| `and` | `&&` | Logical AND |
| `or` | `\|\|` | Logical OR |
| `not` | `!` | Logical NOT |
| `is` | `===` | Identity comparison |
| `is not` | `!==` | Negated identity comparison |
| `in` | `<~` | Membership check |
| `not in` | `!<~` *(or `not <~`)*| Negated membership check |
| `del` | `<>` | Delete statement |
| `if` | `?` | Conditional branch |
| `elif` | `??` | Else-if branch |
| `else` | `??` | Else branch |
| `for` | `$` | For loop |
| `while` | `$?` | While loop |
| `break` | `$>>` | Break statement |
| `continue` | `$<<` | Continue statement |
| `try` | `~^` | Try block |
| `except` | `?^` | Except handler |
| `except*` | `?^*` | Except-star handler |
| `else` *(try)* | `?!^` | Try-else block |
| `finally` | `?*` | Finally block |
| `as` | `=>` *(or `as`)* | Alias binding operator |
| `raise` | `^^^` | Raise exception |
| `assert` | `^?!` | Assert statement |
| `assert not` | `^?` | Negated assert statement |
| `with` | `&` | Context manager |
| `import` / `from` | `/` | Import symbol |
| `return` | `->` | Return statement |
| `yield` | `~>` | Yield statement |
| `async` / `await` | `%` | Asynchronous operations |
| `lambda` | `(args) -> expr` | Lambda function (arrow syntax) |
| `class` | `Name::` *(or `Name:Parent:`)*| Class declaration |
| `def` | *(omit)* | Function definition |
| `type` | `:` | Type alias declaration |
| `match` | `?==` | Structural pattern matching subject |
| `case` | *(omit)* | Pattern case declaration |

---

## **Core Features & Syntax Guide**

### **1. Logic & Comparisons**

> **Motivation:** English-based logical keywords (`and`, `or`, `not`, `is`, `in`, `del`) disrupt visual uniformity and limit code compactness. Replacing them with mathematical/logical operators (similar to C, C++, and JavaScript) makes code more concise and aligns Python with globally understood developer syntax.

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
? (x === ++ && y !== --) || z === ~:
    <> val
? x !<~ my_list:
    pass
```

---

### **2. Conditionals & If-Expressions**

> **Motivation:** Standard conditional keywords create substantial vertical and horizontal indentation clutter. The ternary sigils `?` and `??` unify simple conditions under a single theme. Introducing single-line `if` expressions without an `else` branch allows clean conditional assignments that default to `None` without requiring boilerplate fallback declarations.

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

> **Motivation:** Loops and jump statements (`for`, `while`, `break`, `continue`) are extremely frequent, and standardizing them with single-character sigils increases syntax density. Additionally, standard Python loop-else blocks (`else:`) are notoriously counterintuitive because they execute only when a loop does *not* break; naming the block `?!$>>:` (literally "if not break") explicitly documents the execution path.

Loh loop grammar uses `$` for `for` loops, `$?` for `while` loops, `$>>` for `break`, and `$<<` for `continue`.

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
        $<<
    ? i == 8:
        $>>
    total += i
?!$>>:
    print("Loop finished")
```

> **Note:** The loop `else:` block can be written as `?!$>>:` or `?! break:` (literally translating to "if not break"), which clarifies when the block will execute. Standard `else:` is also supported.

---

### **4. Comprehensions**

> **Motivation:** Standard list, set, and dictionary comprehensions are wordy. Combining the loop sigil `$` and condition sigil `?` creates highly compact collections.

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

> **Motivation:** Function definitions often contain redundant keywords like `def` and `return`. Since parameter lists and colons already denote function declarations, the `def` keyword can be safely omitted. Using arrows for return and yield aligns with modern programming syntax.

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

---

### **6. Arrow Lambdas**

> **Motivation:** The standard anonymous function keyword `lambda` is verbose. Arrow lambdas `(args) -> expr` align with modern anonymous functions.

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

> **Motivation:** Overriding duplicate keyword arguments at runtime enables cleaner configuration patterns. Naming the keyword collector variable `**` removes the boilerplate of naming and unpacking `**kwargs` manually.

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

### **8. Classes & Object Properties**

> **Motivation:** Standard Python classes suffer from a heavy "self-clutter" tax. By auto-injecting the instance parameter (normally `self`) when methods start with a dot (`.`) and mapping `.attribute` directly to `self.attribute`, Loh retains Python's explicit instance model while removing the repetitive manual typing of `self`.

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
    .__init__(owner, balance):
        .owner = owner
        .balance = balance

    .deposit(amount):
        .balance += amount
        -> .balance
```
*Note: A class with no base classes can be declared using `MyClass::`.*

---

### **9. Type Aliases**

> **Motivation:** Type statements are common in modern type-annotated codebases. Mapping them to a simple colon (`:`) keeps type definitions neat and visual.

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

> **Motivation:** Error-handling flow is highly visual and fits symbolic mapping perfectly (`~^` represents the boundary entry, `?^` catches issues). Providing string raising (`^^^ "message"`) eliminates constructor boilerplate for basic exceptions.

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
~^:
    result = 10 / 0
?^ ZeroDivisionError => e:
    print(f"Error: {e}")
?!^:
    print("Success")
?*:
    print("Cleanup")

# Raising Exceptions
^^^ ValueError("Invalid code")
^^^ "Something went wrong"
^^^ "Failed" from error
```

---

### **11. Assertions & Assert Not**

> **Motivation:** Defensive programming requires clean, readable validation checks. Caret assertions (`^?!` and `^?`) compress validation checks down to a single line.

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

> **Motivation:** Imports in Python represent modules stored in a hierarchical directory layout. Using `/` matches filesystem paths, making import structures and relative imports (`/ . / helper`) immediately intuitive and visually distinct from standard logical code.

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

### **13. Implicit None ("The Empty Space")**

> **Motivation:** In Python, the absence of a value is traditionally represented by the keyword `None`. Loh codifies the "empty space" as an implicit representation of `None` in assignments, comparison operands, dictionary pairs, and parameter defaults, eliminating the repetition of typing `None`.

Loh treats empty syntax spaces as implicit `None` values, simplifying default assignments and None-checks.

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
x =
y: float =

foo(a=, b=):
    ...

? x is:
    pass
? y !==:
    pass
? z ==:
    pass

my_dict = {'a':, 'b':}
```

---

### **14. Implicit Parameter & Argument Mapping**

> **Motivation:** Mapping variables directly to matching parameter names (`name=name` or `x=x`) is a massive source of boilerplate in standard Python. The `=name` shorthand resolves this repetition.

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

> **Motivation:** Assigning object attributes to local variables of the same name is a highly repetitive pattern. The attribute assignment statement automates variable declaration from attributes.

#### **Python**
```python
verbose = config.verbose
```

#### **Loh**
```python
=config.verbose
```

---

### **16. Implicit Boolean Flags**

> **Motivation:** Setting boolean variables or parameters is frequent; increment/decrement-style operators (`++`/`--`) provide a clean assignment shortcut.

#### **Python**
```python
def foo(a=True, b=False):
    pass

x = True
y = False
```

#### **Loh**
```python
foo(a++, b--):
    ...

x++
y--
```

---

### **17. Dict Literals Keyword-Style Syntax**

> **Motivation:** Dictionary keys in Python are frequently string literals. Requiring quotes (`{'x': 10}`) adds noise, whereas keyword-style dict assignments (`{x=10}`) align dict construction with keyword function arguments, removing clutter.

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

> **Motivation:** Nested function execution (like `h(g(f(x)))`) reads right-to-left and is hard to scan. The pipe operator (`|>`) establishes sequential, left-to-right pipelines (resembling Elixir, F#, or Unix terminal piping), improving readability for data transformation flows.

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

> **Motivation:** During string interpolation or file output generation, representing missing values as literal `"None"` strings often ruins formatting or requires explicit `val or ""` wrappers. Importing `empty_none_str` configures the runtime to output `""` instead, simplifying formatting templates.

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
