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

Loh simplifies conditional logic by utilizing `?` for `if` and `??` for both `elif` and `else`.

#### **Side-by-Side Comparison**

```python
# Python
if score > 90:
    grade = 'A'
elif score > 80:
    grade = 'B'
else:
    grade = 'C'
```
```python
# Loh
? score > 90:
    grade = 'A'
?? score > 80:
    grade = 'B'
??:
    grade = 'C'
```

#### **Single Line If-Expressions (Ternary)**
Loh supports Python's ternary expressions without requiring an `else` branch (evaluating to `None` if the condition is false), as well as standard three-operand expressions:

| **Python Equivalent** | **Loh Syntax** |
| :--- | :--- |
| `x if cond else y` | `x ? cond ?? y` |
| `x if not cond else y` | `x ?! cond ?? y` *(using `!` for not)* |
| `x if cond else None` | `x ? cond` |
| `x if not cond else None` | `x ?! cond` |

---

### **3. Loops & Control Flow**

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

> [!NOTE]
> The loop `else:` block can be written as `?!$>>:` or `?! break:` (literally translating to "if not break"), which clarifies when the block will execute. Standard `else:` is also supported.

#### **Alternative Loop Syntax & Comprehensions**
- You can substitute `in` with `:=` or `<~` in loop headers:
  ```python
  $ i := range(10):  # Equivalent to: for i in range(10):
  ```
- **Comprehensions** feel natural with Loh syntax:
  ```python
  # Python: evens = [i for i in range(10) if i % 2 == 0]
  evens = [i $ i <~ range(10) ? i % 2 == 0]
  
  # Python: firsts = [first for first, *rest in data]
  firsts = [first $ first, *rest <~ data]
  ```

---

### **4. Functions, Lambdas, & Duplicate Kwargs**

#### **Function Definitions**
The `def` keyword is omitted in Loh. Standard signatures start directly with the function name and parameters. Return statements use `->` and yield statements use `~>`.

```python
# Function with type annotations and return
calculate_total(price: float, tax: float) -> float:
    -> price * (1 + tax)

# Generator yielding values
countdown(n) -> Generator:
    $? n > 0:
        ~> n
        n -= 1
```

#### **Lambda Functions**
Loh introduces clean arrow function lambdas. Parentheses around arguments are required.

```python
# Python: map(lambda x: x * 2, [1, 2, 3])
map((x) -> x * 2, [1, 2, 3])

# Multi-argument lambda
add = (x, y) -> x + y
```

#### **Duplicate Keyword Arguments & Kwarg Collection (`**`)**
Loh allows overriding duplicate kwargs inside function calls. Additionally, the `**` token functions as a special local variable mapping to the kwargs dict:

- **Duplicate Kwargs**: Passing duplicate keyword keys at runtime (e.g. `foo(1, a=2)` or `foo(a=1, **{"a": 2})`) will override the earlier values instead of raising a `TypeError`. Note that literal duplicate keywords like `foo(a=1, a=2)` are caught by the parser at compile-time and will raise a `SyntaxError: keyword argument repeated`.
- **Kwarg Variable (`**`)**: Declaring `**` at the end of a parameter list binds it as a local variable. You can manipulate, pass, or return it directly.

```python
def setup_config(name, **):
    **['name'] = name
    -> **  # returns the collected kwargs dict
```

---

### **5. Classes & Object Properties**

Loh class syntax uses a double colon (`::`) to denote class definition. Method declarations omit `def`. If a method name starts with a dot (`.`), standard `self` is automatically injected as the first parameter, and attributes can be referenced directly using `.attribute` (which resolves to `self.attribute`).

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

### **6. Exceptions & Assertions**

Exception handling blocks are mapped to symbols, and assertion keywords are significantly shortened.

#### **Try-Except-Finally Block**

```python
# Python
try:
    result = 10 / 0
except ZeroDivisionError as e:
    print(f"Error: {e}")
else:
    print("Success")
finally:
    print("Cleanup")
```
```python
# Loh
~^:
    result = 10 / 0
?^ ZeroDivisionError => e:
    print(f"Error: {e}")
?!^:
    print("Success")
?*:
    print("Cleanup")
```

#### **Raising Exceptions & Raising Strings**
- To raise exceptions, use the `^^^` operator:
  ```python
  ^^^ ValueError("Invalid code")
  ```
- **Raising Strings**: Loh compiles direct string raising into standard Exception instances:
  ```python
  ^^^ "Something went wrong"             # Raises Exception("Something went wrong")
  ^^^ "Failed" from error                # Raises Exception("Failed") from error
  ```

#### **Assert & Assert Not**
- **Assert**: `^?!` is mapped to `assert`:
  ```python
  ^?! x > 10, "x is too small"
  ```
- **Assert Not**: `^?` is mapped to `assert not`:
  ```python
  ^? x, "x must be False or None"        # Compiles to: assert not x, "x must be False or None"
  ```

---

### **7. Module Imports & Aliasing**

Loh converts all import syntax to use forward slashes (`/`), mimicking filesystem paths. Aliasing uses `=>`.

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

### **8. Implicit None ("The Empty Space")**

Loh treats empty syntax spaces as implicit `None` values, simplifying default assignments and None-checks.

- **Empty Assignment**:
  ```python
  x =        # Resolves to: x = None
  y:float =  # Resolves to: y:float = None
  ```
- **Default Arguments**:
  ```python
  foo(a=, b=):  # Resolves to: def foo(a=None, b=None):
      ...
  ```
- **None Comparisons**: Omitting the right-hand operand of comparison symbols evaluates the comparison against `None`:
  ```python
  ? x is:      # Resolves to: if x is None:
      ...
  ? y !==:     # Resolves to: if y is not None:
      ...
  ? z ==:      # Resolves to: if z == None:
      ...
  ```
- **Dictionary Shorthand**:
  ```python
  my_dict = {'a':, 'b':}  # Resolves to: {'a': None, 'b': None}
  ```

---

### **9. Implicit Parameter & Argument Assignments**

Loh contains syntaxes to quickly map variables into method calls and default assignments:

- **Argument Shorthand (`=name`)**: Pass variables as keyword arguments of the same name:
  ```python
  foo(=name, =config)   # Resolves to: foo(name=name, config=config)
  foo(=obj.value)       # Resolves to: foo(value=obj.value)
  ```
- **Definition Defaults (`=name`)**: Bind parameter defaults directly from outer scopes:
  ```python
  a = 1
  foo(=a):              # Resolves to: def foo(a=a): (where default is 1)
      ...
  ```
- **Attribute Statements (`=obj.attr`)**: Easily assign attributes to local variables:
  ```python
  =config.verbose       # Resolves to: verbose = config.verbose
  ```
- **Bool Defaults & Statements (`++` / `--`)**: Assign or default boolean variables implicitly:
  ```python
  foo(a++, b--):        # Resolves to: def foo(a=True, b=False):
      ...
  x++                   # Resolves to: x = True
  y--                   # Resolves to: y = False
  ```

---

### **10. Dict Literals Keyword-Style Syntax**

Instead of standard string mapping, dictionary literals can accept keyword-style assignments:

```python
# Python: my_dict = {'x': 10, 'y': 20, 'z': None}
my_dict = {x=10, y=20, z=}
```

---

### **11. The Pipe Operator (`|>`)**

Loh features a pipe operator to feed expressions into callable objects. `x |> f` evaluates to `f(x)`. It has lower precedence than standard arithmetic and chains from left to right.

```python
# Pipe chains evaluate left-to-right
result = 5 |> (x) -> x + 1 |> (x) -> x * 2  # Evaluates to: ((5 + 1) * 2) = 12

# Processing a clean data pipeline
processed = (
    "   my data string   "
    |> str.strip
    |> str.upper
    |> (s) -> s.replace(" ", "_")
)
# processed == "MY_DATA_STRING"
```

---

### **12. Advanced Compiler Features**

- **Type Aliasing (`:`)**:
  ```python
  : IntOrFloat = int | float  # Resolves to: type IntOrFloat = int | float
  ```
- **Empty None String future import**:
  Importing the future flag `empty_none_str` forces `str(None)` to output an empty string `""` instead of `"None"`:
  ```python
  from __future__ import empty_none_str
  print(str(None))  # Outputs: ""
  ```
