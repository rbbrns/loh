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
| `x != None` | `x !~` | Value inequality comparison with None (presence postfix) |
| `x is not None` | `x !~~` | Identity inequality comparison with None (presence postfix) |
| None-safe member | `~.` | None-safe attribute access / navigation |
| None-coalesce | `~~` | None-coalescing operator (fallback value) |
| None-safe index | `~[]` | None-safe subscripting / indexing |
| `and` | `&&` | Logical AND |
| `or` | `\|\|` | Logical OR |
| `x = x or y` | `x \|\|= y` | Truthy-coalescing assignment |
| `x = x and y` | `x &&= y` | Logical-AND assignment |
| `x = x if x == True else y` | `x ++= y` | Boolean-strict True fallback assignment |
| `x = x if x == False else y` | `x --= y` | Boolean-strict False fallback assignment |
| `not` | `!` | Logical NOT |
| `is` | `===` | Identity comparison |
| `is not` | `!==` | Negated identity comparison |
| `in` | `<==` | Membership check |
| `not in` | `!<==` *(or `not <==`)*| Negated membership check |
| `range(start, stop)` | `start..stop` | Range / sequence literal |
| `del` | `<>` | Delete statement |
| `if` | `?` | Conditional branch |
| `elif` | `??` | Else-if branch |
| `else` | `??` | Else branch |
| `a if a else b` | `a ?? b` | Truthy-coalescing expression |
| `for` | `$` | For loop |
| Implicit Loop | `$ := items:` | Implicit loop targeting `$` (e.g. `print($)`) |
| Filtered Loop | `$ t := ex ? cond:` | Loop with inline filter check (e.g. `? item > 2:`) |
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
| `async` / `create_task` | `%` | Async function declaration / non-blocking task spawn |
| `await` / `gather` | `%%` | Await completion / multi-task gather (`%% [t1, t2]`) |
| `lambda` | `(args) -> expr` | Lambda function (arrow syntax) |

| `class` | `Name::` *(or `Name:Parent:`)*| Class declaration |
| `def` | *(omit)* | Function definition |
| Function Aliasing | `foo \| bar(x):` | Function and method aliasing |
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
| Lazy evaluation | `` `expr` `` | Late-bound expressions and lazy evaluation variables |
| Extension key separators | `+:`, `|:`, `?:`, `: <>` | Dict extension key separators for appending, merging, fallback defaults, and deletion |
| Deep-copy container unpacking | `[***x]` / `{***x}` / `{***z}` | Recursive deep-copy unpacking inside container literals and function arguments |
| Implicit dict unpacking | `{ "a": 1, {"b": 2} }` | Bare dictionary and dict comprehension unpacking inside dict literals without `**` |
| Set & dict relational comparisons | `set <= iterable` / `dict <= dict` | General set containment checks and dict sub-mapping matching |





---

## **Core Features & Syntax Guide**

### **1. Logic & Comparisons**

> **Motivation:** Symbols like `&&`, `||`, and strict identity `===` align Python with standard mathematical logic and modern programming conventions. Operators like `<==` (membership evaluation) and `<>` (removal/deletion) use visual cues that match the developer's mental model of collection scanning and variable cleanup.

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
? x !<== my_list:
    ...
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
$ i := 0..10:
    ? i == 5:
        $<
    ? i == 8:
        $>
    total += i
?!$>:
    print("Loop finished")
```

#### **Implicit Loops & Loop Filters**

Loh supports short-form implicit loops and inline header filtering to reduce boilerplate nesting:

1. **Implicit Loops (`$ :=`)**: Omitting the target variable name binds the loop sigil `$` to the current element inside the loop body.
2. **Inline Loop Filters (`? condition`)**: Allows placing a query filter directly in the loop signature row, avoiding an indented `if` check.

##### **Python**
```python
# 1. Standard loop filter requires nesting
for user in users:
    if not user.is_active:
        continue
    send_email(user)
```

##### **Loh**
```python
# Implicit target, inline filter, and accessing current item as '$'
$ := users ? $.is_active:
    send_email($)
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
$ i := 0..10:
    ...
evens = [i $ i := 0..10 ? i % 2 == 0]
firsts = [first $ first, *rest := data]
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
        ...

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

#### **File Resolution**

When importing a module, Loh searches for files with the `.loh` extension (e.g. `module.loh`) in the module search path (`sys.path`) alongside traditional `.py` files.

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
    ...
? !y~~~:     # Or: ? y !== ~:
    ...
? z~~:       # Or: ? z == ~:
    ...

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
    ...
? y--:
    ...
```

---


### **17. The Pipe Operator (`|>`)**

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

### **18. The `empty_none_str` Future Import**

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

### **19. None-Safe Operators (`~.`, `~~`, `~[]`)**

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

### **20. Range / Slice Literals (`..`)**

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
$ i := 0..10:
    print(i)

# Check if value in range
? x <== 1..100:
    print("In bounds")
```

---

### **21. Inline Scope Initializer Block (`obj { ... }`)**

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

### **22. Runtime Versioning (`sys.loh_version`, `sys.loh_version_info`)**

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

### **23. The `auto_fstrings` Future Import**

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

### **24. None-Safe Attribute Assignment**

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

### **25. Infinite Loops Shorthand**

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

### **26. Multi-Key Subscript Slicing and Unpacking (`*` / `**`)**

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
d = {'x': 1, 'y': 2}

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

### **27. The `implicit_returns` Future Import**

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

---

### **28. Truthy-Coalescing and Logical-AND Assignments (`||=`, `&&=`)**

> **Motivation:** Python lacks short-circuiting logical compound assignment operators. Instead of writing `x = x or y` or `x = x and y`, Loh supports `x ||= y` and `x &&= y` which perform these operations and assign the result to the target back in-place.

* **Truthy-Coalescing Assignment (`||=`)**: `x ||= y` evaluates to `x = x or y` (Python `x = x or y`).
* **Logical-AND Assignment (`&&=`)**: `x &&= y` evaluates to `x = x and y` (Python `x = x and y`).

Both operators support full short-circuiting: the right-hand side is only evaluated if the left-hand side's value requires it.

#### **Example**
```python
x = 0
x ||= 42  # x becomes 42 (since 0 is falsy)

y = 100
y &&= 200 # y becomes 200 (since 100 is truthy)
```

---

### **29. Boolean-Strict Fallback Assignments (`++=`, `--=`)**

> **Motivation:** It is common to want a fallback value if a variable is not strictly equal to `True` or `False`. While `||=` and `&&=` operate on truthiness/falsiness, `++=` and `--=` perform strict equality checks with `True` and `False` respectively.

* **Boolean-Strict True Fallback (`++=`)**: `x ++= y` desugars to `x = x if x == True else y`.
* **Boolean-Strict False Fallback (`--=`)**: `x --= y` desugars to `x = x if x == False else y`.

#### **Example**
```python
x = 1
x ++= "fallback"  # x remains 1 (since 1 == True is True in Python)

y = "hello"
y ++= "fallback"  # y becomes "fallback" (since "hello" == True is False)

z = 0
z --= "fallback"  # z remains 0 (since 0 == False is True in Python)
```

---

### **30. Presence Postfix Operators (`!~`, `!~~`)**

> **Motivation:** Postfix checks for value and identity inequality with `None` allow for cleaner presence guards and query filters without nesting or leading `not` calls.

* **Value Presence Postfix (`!~`)**: `x !~` checks value inequality with `None` (equivalent to `x != None`).
* **Identity Presence Postfix (`!~~`)**: `x !~~` checks identity inequality with `None` (equivalent to `x is not None`).

#### **Example**
```python
x = 42
if x!~~:
    print("x is present")

# A class overriding __eq__ for None
class EqualNone:
    __eq__(self, other):
        -> other === ~

obj = EqualNone()
obj!~   # False (since obj == None is True)
obj!~~  # True (since obj is not None)
```

---

### **31. Truthy-Coalescing Operator (`??`)**

> **Motivation:** Loh supports None-coalescing with `~~`, but often developers want a more general coalescing operator that falls back if the left-hand side is *falsy* (rather than strictly `None`). The truthy-coalescing operator `??` provides this by desugaring to a logical `or` expression, while maintaining full syntactic compatibility with the ternary conditional operator `a ? b ?? c`.

* **Truthy-Coalescing (`??`)**: `a ?? b` evaluates to `a or b` (equivalent to `a if a else b`).

Truthy-coalescing is fully short-circuiting: the right-hand side is only evaluated if the left-hand side is falsy.

#### **Example**
```python
x = 0
fallback = x ?? 42          # fallback is 42

name = "Loh"
result = name ?? "default"  # result is "Loh"

# Does not conflict with standard ternary: true_val ? condition ?? else_val
x = 10 ? True ?? 20         # x is 10
```

---

### **32. Function Aliasing (`foo | bar`)**

> **Motivation:** Functions and methods often benefit from alias names to support command-like interfaces, offer short keyboard shortcuts, or retain backward compatibility during API changes. Using the pipe `|` separator directly on the signature line allows defining multiple aliases cleanly without writing redundant boilerplate wrapper functions.

* **Function Aliasing (`|`)**: Defines a function or class method under multiple names.

#### **Example**
```python
foo | bar(x, y):
    -> x + y

# Both names are valid callable targets
result_foo = foo(10, 20)  # 30
result_bar = bar(10, 20)  # 30
```

Inside classes, asymmetric method aliasing is fully supported:
```python
MyClass::
    .add | sum(x, y):
        -> x + y
```
This desugars cleanly to:
```python
class MyClass:
    def add(self, x, y):
        return x + y
    sum = add
```

---

### **33. Lazy Evaluation & Late-Bound Expressions (`\`expr\``)**

> **Motivation:** Standard Python evaluates default parameter arguments once at function definition time. This causes the infamous mutable-default trap (e.g., `x=[]`) and prevents default values from referencing other parameters or instance receivers (`self` / `.`). Delayed execution of expensive computations also requires verbose boilerplate like wrapping in helper functions. Lazy evaluation backticks (`` `expr` ``) solve this by desugaring code-quote expressions into a highly-performant C-level cached proxy (`_LohLazy`) that resolves on-demand.

* **Lazy Variables**: Creating a lazy object `` lazy_val = `expr` `` defers the evaluation of `expr` until the value is first accessed. The result is then cached for all future accesses.
* **Late-Bound Signature Defaults**: Parameter defaults wrapped in backticks (e.g., `x = `expr``) are dynamically resolved within the function body's local scope at call-time. This allows defaults to reference preceding arguments or instance receivers (`self` / `.`).

#### **Example**
```python
x = 10
lazy_val = `x + 5`

# Evaluates on first use and resolves to 15
print(lazy_val) # 15

# Since the result is cached, subsequent outer changes do not affect it
x = 20
print(lazy_val) # 15
```

#### **Late-Bound Parameters Example**
```python
def calculate(width, height = `width * 2`, depth = `height * 3`):
    -> width + height + depth

print(calculate(10)) # 90 (10 + 20 + 60)
print(calculate(10, 5)) # 30 (10 + 5 + 15)
```

Inside class methods, receiver attribute lookup is fully supported via late-bound defaults:
```python
Circle::
    .(.radius):
        ...
    .diameter(d = `.radius * 2`):
        -> d
```

---

### **34. Dict & Object Extension Key Separators (`|:`, `+:`, `?:`, `: <>`)**

> **Motivation:** When merging or extending configuration dictionaries (`base | { ... }`), standard assignment (`key: val`) replaces parent keys completely. Adding modifier sigils to the dictionary key-value separator `:` provides fine-grained control over list appending, deep merging, default fallbacks, and key deletion.

Loh introduces four extension key separators:

| Separator | Name | Semantics | Example |
| :--- | :--- | :--- | :--- |
| **`key: val`** | **Overwrite** | Standard assignment. Replaces parent value completely. | `"port": 8443` |
| **`key +: val`** | **Append / Concat** | Appends or concatenates to existing value (`list + list`, `str + str`, `num + num`). | `"tags" +: ["v2"]` |
| **`key |: val`** | **Union / Merge** | Deep-merges dictionaries (`dict | dict`) or unions sets (`set | set`). | `"headers" |: {"Auth": "Bearer"}` |
| **`key ?: val`** | **Fallback Default** | Sets `key` to `val` **if and only if** `key` is missing or `None` in target. | `"timeout" ?: 60` |
| **`key: <>`** | **Delete Key** | Removes `key` entirely from the target dictionary using Loh's `<>` delete symbol. | `"legacy_mode": <>` |

#### **Example**
```python
base_config = {
    "host": "localhost",
    "port": 8000,
    "tags": ["v1"],
    "headers": {"Accept": "application/json"},
    "timeout": 30,
    "legacy_mode": True
}

# Extending base_config:
prod_config = base_config | {
    "port": 8443,                              # Overwrite
    "tags" +: ["v2"],                          # Append -> ["v1", "v2"]
    "headers" |: {"Authorization": "Bearer"},  # Deep dict merge
    "timeout" ?: 60,                           # Keeps 30 (exists and non-None)
    "legacy_mode": <>                          # Deletes 'legacy_mode' from result
}
```

Bare dictionary literals containing extension key separators construct patch dictionaries wrapping `_LohOp` marker objects. These patch dictionaries support `pickle.dumps()` / `pickle.loads()`, `dict.copy()`, and `repr()`.

---

### **35. Deep-Copy Container Unpacking (`[***x]`, `{***x}`, `{***z}`)**

> **Motivation:** Python supports 1-level shallow unpacking inside container literals using `[*y]` (lists), `{**x}` (dicts), and `{*z}` (sets). To provide a conflict-free, complementary syntax for deep copies without colliding with function argument unpacking (`func(*args)` or `func(**kwargs)`), Loh introduces `***` (Triple-Star) as a recursive deep-unpacker inside container literals and function arguments.

Loh defines `***` unpacking across all container types and call arguments:

| Unpacking Type | List Literal | Dict Literal | Set Literal |
| :--- | :--- | :--- | :--- |
| **Shallow Copy (1-Level)** | `[*y]` | `{**x}` | `{*z}` |
| **Deep Copy (Recursive)** | **`[***y]`** | **`{***x}`** | **`{***z}`** |

#### **Example**
```python
original_dict = {"items": [1, 2, 3], "meta": {"owner": "Alice"}}

# Deep-copies original_dict recursively into a new dictionary literal:
deep_dict = {***original_dict}

# Deep-copies nested elements into a list:
deep_list = [***original_dict["items"]]

# Deep-copies sequence elements into function arguments:
foo(***original_dict["items"])
```

#### **Compile-Time Desugaring**
At parse-time, the PEG parser compiles `***x` inside container literals and call arguments by desugaring `x` into a call to `_loh_deepcopy(x)` prior to unpacking.

---

### **36. Implicit Dictionary Unpacking inside Dict Literals (`{}`)**

> **Motivation:** In standard Python, placing an un-keyed dictionary expression or dict comprehension (`{ k: v for ... }`) directly inside another dictionary literal without `**` (e.g. `{ "a": 1, {"b": 2} }`) is a syntax error. In Loh, writing a bare dictionary or dict comprehension as a standalone entry inside a dictionary literal (`{}`) automatically unpacks (`**`) its key-value pairs into the enclosing dictionary.

#### **Example**
```python
app_config = {
    "env": "prod",
    "port": 8080,

    # Bare dict comprehension inside {} -> AUTOMATICALLY UNPACKED!
    { f"node_{i}": f"http://10.0.0.{i}" for i in range(1, 4) },

    # Combined with field extension
    "tags" +: ["cluster_a"]
}

# Evaluates to:
# {
#     "env": "prod",
#     "port": 8080,
#     "node_1": "http://10.0.0.1",
#     "node_2": "http://10.0.0.2",
#     "node_3": "http://10.0.0.3",
#     "tags": ["cluster_a"]
# }
```

#### **Zero Set Ambiguity**
Python distinguishes sets from dicts by the presence of colons (`:`). Because dictionaries are unhashable and can never be elements of sets (`{ {"a": 1} }` raises `TypeError`), placing a bare dictionary expression `{ k: v ... }` inside an outer dictionary `{}` is 100% grammatically unambiguous: it can only mean unpacking its key-value pairs (`**`) into the outer dictionary.

---

### **37. Set & Dict Relational Comparisons (`set <= iterable`, `dict <= dict`)**

> **Motivation:** In standard Python, comparing a `set` with a non-set iterable (like a `list` or `dict`) or comparing two `dict` objects using relational operators (`<`, `<=`, `>`, `>=`) raises a `TypeError`. In Loh, set and dictionary relational comparisons are dynamically extended to perform containment and sub-mapping checks at C-level speed.

#### **Set Relational Comparisons (`set <= iterable`)**
When comparing a `set` with a non-set sequence or dictionary using relational operators, the non-set operand is dynamically converted to a set at runtime:

```python
# Check if all elements in a set are present in a list:
? {"a", "b"} <= ["a", "b", "c"]:
    print("All elements present!")

# Check if keys exist in a dictionary:
? {"env", "port"} <= config_dict:
    print("Required keys are configured!")
```

#### **Dictionary Sub-Mapping Containment (`dict <= dict`)**
Comparing two dictionary objects with relational operators checks key-value sub-mapping containment:

```python
# Check if key "port" has value 8080:
? {"port": 8080} <= config:
    print("Port is 8080")

# Check if multiple key-value pairs match:
? {"env": "prod", "port": 8443} <= config:
    print("Production environment configuration matched!")

---

### **38. Explicit & Implicit Walrus Loops & Arity Sigils (`:=`, `$`, `$$`, `$$$`, `$?:`)**

> **Motivation:** Loh unifies statement-level loops and comprehensions around the walrus operator (`:=`), arity sigils (`$`, `$$`, `$$$`), and conditional `while` loops (`$?`).

#### **Comprehensions with Implicit Arity Sigils**
Use `:=` to introduce loop clauses in list, dict, set, and generator comprehensions. Arity sigils `$` (1st item), `$$` (2nd item), and `$$$` (3rd item) automatically set the loop unpacking target at parse-time:

```python
# List comprehension (single item):
doubled = [$ * 2 := [1, 2, 3]]  # [2, 4, 6]

# List comprehension (pair zip):
totals = [$ * $$ := zip([10, 20], [2, 3])]  # [20, 60]

# List comprehension (enumerate):
labels = [f"#{$ + 1}: {$$}" := enumerate(["Alice", "Bob"])]  # ["#1: Alice", "#2: Bob"]

# Dict comprehension (invert keys & values):
swapped = {$$: $ := {"a": 1, "b": 2}.items()}  # {1: "a", 2: "b"}

# Generator call:
total = sum($.price := items)
```

#### **Statement Loops & Arity Sigils**
```python
# Implicit 2-item loop (pair unpacking):
$$ := {"x": 100, "y": 200}.items():
    print($, "=>", $$)

# Implicit 3-item loop (triple unpacking):
$$$ := zip(names, ages, cities):
    print($, $$, $$$)

# Conditional while loop with walrus:
$? := file.readline():
    print($)
```

---

### **39. Async Task Spawning (`%`) & Completion Await / Gathering (`%%`)**

> **Motivation:** Loh unifies asynchronous operations around the `%` sigil with a symmetric rule: single `%` means non-blocking background execution or declaration, and double `%%` means awaiting completion or gathering concurrent tasks.

#### **Async Function & Background Spawning (`%`)**
```python
# Declare async function:
% fetch_data(url):
    -> api.get(url)

# Non-blocking background task spawn (asyncio.create_task):
task1 = % fetch_data("https://api.a.com")
task2 = % fetch_data("https://api.b.com")
```

#### **Awaiting & Concurrent Gathering (`%%`)**
```python
# Await single task:
data1 = %% task1

# Concurrently gather multiple tasks (asyncio.gather):
res1, res2 = %% [task1, task2]

# Starred task list gathering:
results = %% *task_list
```


```






