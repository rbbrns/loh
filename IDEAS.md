# Loh Language Enhancement Ideas

This document tracks potential future language enhancements and syntax upgrades for **Loh**.

---

## 1. Loop Auto-Enumeration with `$, item <~ collection` (Featured)

### Motivation
Tracking iteration indices is a common pattern that currently requires wrapping collections in `enumerate(collection)`. This adds visual nesting and boilerplate. We want a lightweight, unambiguous way to capture the loop index directly using the loop sigil `$`.

### Proposed Syntax
By placing a comma `,` immediately after the loop keyword `$`, the loop symbol itself doubles as the index variable name. Inside the loop body, `$` evaluates to the current iteration index:

```python
# Loop with index
$, item <~ items:
    print($, item)  # '$' is the index
```

### Compile-Time Desugaring
At parse-time, the PEG parser translates the `$` index variable to a compiler-safe variable name (e.g., `_dollar_idx`) and wraps the collection in a call to `enumerate()`:

1. **Header Translation**:
   ```python
   $, item <~ items:
   # becomes:
   for _dollar_idx, item in enumerate(items):
   ```
2. **Body Translation**:
   Any standalone `$` inside the loop body is parsed as `_dollar_idx`:
   ```python
   print($, item)
   # becomes:
   print(_dollar_idx, item)
   ```

### Technical Implementation Path
This is implemented entirely in the parser, with zero changes required to the bytecode compiler, runtime, or VM:

1. **Grammar Expression Rule** (in [python.gram](file:///Users/robbarnes/Development/loh/Grammar/python.gram) under `atom`):
   ```peg
   atom[expr_ty]:
       | '$' { _PyAST_Name(CHECK(PyObject*, _PyPegen_new_identifier(p, "_dollar_idx")), Load, EXTRA) }
   ```
2. **Grammar Statement Rule** (in [python.gram](file:///Users/robbarnes/Development/loh/Grammar/python.gram) under `for_stmt`):
   ```peg
   for_stmt[stmt_ty]:
       | '$' ',' t=star_targets 'in' ~ ex=star_expressions ':' tc=[TYPE_COMMENT] b=block el=[loop_else_block] {
           _PyAST_For(
               _PyAST_Tuple(CHECK(asdl_expr_seq*, _PyPegen_seq_insert_in_front(p, 
                   _PyAST_Name(CHECK(PyObject*, _PyPegen_new_identifier(p, "_dollar_idx")), Store, EXTRA),
                   _PyPegen_singleton_seq(p, t)
               )), Store, EXTRA),
               _PyAST_Call(
                   _PyAST_Name(CHECK(PyObject*, _PyPegen_new_identifier(p, "enumerate")), Load, EXTRA),
                   _PyPegen_singleton_seq(p, ex),
                   NULL, NULL, NULL, EXTRA
               ),
               b, el, NEW_TYPE_COMMENT(p, tc), EXTRA
           )
       }
   ```

---

## 2. Unified Expression Placeholders & Partial Application (`$`, `$$`, `$$$`)

### Motivation
In standard Python, writing inline transformation callbacks (`lambda x: x * 2`) or partial function applications (`functools.partial(json.dumps, indent=4)`) requires verbose syntax or extra library imports. Using `_` as a placeholder collides with standard Python internationalization (`gettext` `_("text")`), REPL history, and discard variable conventions (`for _ in items`).

Loh unifies expression placeholders and partial application around its arity sigils (`$`, `$$`, `$$$`). Because walrus loop and comprehension bindings explicitly use `:=` (e.g. `[$ * $$ := zip(a, b)]`), expressions *without* `:=` that contain arity sigils (`$`, `$$`, `$$$`) automatically desugar into anonymous lambdas at compile-time.

### Proposed Syntax
```python
# Unary expression placeholder & piping
evens = [1..10] |> filter($ % 2 == 0) |> map($ * 2)

# Partial function application
add10 = add(10, $)
formatted = data |> json.dumps($, indent=4)

# Multi-argument arity placeholders ($ = 1st arg, $$ = 2nd arg)
total = numbers |> reduce($ + $$)
sorted_users = users |> sort_by($.age - $$.age)
swap_args = divide($$, $)
```

### Compile-Time Desugaring
When the parser detects arity sigils (`$`, `$$`, `$$$`) inside an expression call site or operator expression without a loop `:=` clause, it wraps the expression in a single- or multi-parameter lambda:
```python
# 'data |> json.dumps($, indent=4)' desugars to:
json.dumps(data, indent=4)

# 'add(10, $)' desugars to:
lambda _1: add(10, _1)

# 'reduce($ + $$)' desugars to:
reduce(lambda _1, _2: _1 + _2, numbers)
```


---

## 3. None-Coalescing Assignment Operator (`~~=`)

### Motivation
In Loh, `~~` acts as the binary None-coalescing operator. Developers frequently need to assign a fallback value to a variable or attribute if and only if its current value is `None`. This avoids verbose conditional checks or repeating the variable name.

### Proposed Syntax
```python
config.timeout ~~= 30
self.display_name ~~= "Guest"
```

### Compile-Time Desugaring
At parse/compile time, this operator desugars into:
```python
config.timeout = (30 ? config.timeout is None ?? config.timeout)
```



## 5. Destructuring Assignment with Default Values

### Motivation
Unpacking lists or dicts in standard Python raises `ValueError` if the shape of the data doesn't match perfectly. When reading API data or optional configurations, developers want safe unpacking with default fallback values.

### Proposed Syntax
```python
[name, role="guest", active=--] = get_user_details()
```

### Compile-Time Desugaring
Desugars into intermediate index extraction and fallback checks:
```python
_data = get_user_details()
name = _data[0]
role = _data[1] ? len(_data) > 1 ?? "guest"
active = _data[2] ? len(_data) > 2 ?? False
```

---

## 6. Type Coercion / Cast Operator (`: type`)

### Motivation
Developers often need to cast or coerce values explicitly (e.g., converting a string port to an integer). While Python uses function constructor calls like `int(value)`, using a dedicated cast operator `:` (borrowed from languages like TypeScript, Rust, and SQL) is more visually distinct and cleaner to chain.

Since type annotations in Python are statement-level constructs (e.g., `x: int = 5`), utilizing `:` as a binary operator in expression contexts is syntactically unambiguous.

### Proposed Syntax
```python
port = raw_port : int
```

### Compile-Time Desugaring
The cast operator desugars directly into constructor function calls:
```python
port = int(raw_port)
```

---

## 7. None-Safe Type Coercion Operator (`~: type`)

### Motivation
When parsing untrusted or optional input, type casting (e.g., `value : int`) can raise a `ValueError` or `TypeError`. Combining safe navigation `~` with the cast operator `:` provides a clean way to attempt a cast and return `None` on failure rather than crashing.

### Proposed Syntax
```python
# Tries to cast to int, returns None on failure
port = raw_port ~: int

# Easily chains with None-coalescing
port = raw_port ~: int ~~ 8080
```

### Compile-Time Desugaring
Desugars into an inline try-except call wrapper:
```python
def _safe_cast(v, t):
    try:
        return t(v)
    except (TypeError, ValueError):
        return None

port = _safe_cast(raw_port, int)
```

---

## 8. Dictionary Object Property Punning (`{=x, =y}`)

### Motivation
Creating a dictionary key-value pair where the key name matches the variable name (e.g., `{'x': x}`) is a repetitive task. In JavaScript, this is called property shorthand or punning (`{x, y}`). Since Loh already uses `=var` syntax for implicit parameter binding (`foo(=name)`), using `{=x, =y}` for dictionary literals is a logical extension.

### Proposed Syntax
```python
x = 10
y = 20
coord = {=x, =y}
```

### Compile-Time Desugaring
The parser translates this into keyword-style dict literals or standard key-value assignments:
```python
coord = {"x": x, "y": y}
```

### Design Note: `{=x, =y}` vs `{:x, :y}`
- **Call-Site Consistency**: Loh already uses `=var` for call-site parameter punning (e.g., `foo(=name)` desugars to `foo(name=name)`). Using `{=x, =y}` extends this exact `=var` sigil to dictionary key-value punning, maintaining visual consistency across function calls and dictionary construction.
- **Colon (`:`) Sigil Preservation**: In Loh, `:` is reserved for Type Annotations (`x: int`), Type Aliases (`: MyType`), Type Casts (`x : int`), and key-value separation inside dictionary literals (`{key: value}`). Using `{:x}` inside `{}` would introduce parsing collisions with dictionary keys and type hints.

---

## 9. None-Safe Callable Invocation (`func~()`)

### Motivation
Often a callback, event handler, or configuration strategy might be optional (`None`). Calling `callback()` directly throws a `TypeError: 'NoneType' object is not callable` if it is missing.

### Proposed Syntax
```python
# Safe execution of optional callback
on_complete~()

# With arguments
on_error~(code, message)
```

### Compile-Time Desugaring
```python
on_complete() if on_complete is not None else None
```

---

## 10. Range Slice Notation (`lst[start..stop]`)

### Motivation
Standard Python uses `lst[start:stop]`. Since we already planned range literals `start..stop`, we should allow them to be used inside subscripts as a clean alternative to standard slicing.

### Proposed Syntax
```python
# Get elements from index 1 to 4 (exclusive stop)
subset = lst[1..5]

# Open bounds
first_three = lst[..3]
from_index_five = lst[5..]
```

### Compile-Time Desugaring
Translates `a..b` inside subscript brackets to standard Python slice AST nodes: `slice(a, b, None)`.

---

## 11. Parenthesized Expression Blocks (`(expr; expr; expr)`)

### Motivation
Standard Python lacks support for multi-line or chained expression blocks, forcing developers to write helper functions or Immediately Invoked Function Expressions (IIFEs) for inline calculations. 

Using parentheses with semicolon-separated expressions `(statement; statement; expression)` provides a lightweight, visually clean syntax for inline blocks that avoids dict/set braces conflicts. The entire block evaluates to the value of the final expression.

### Proposed Syntax
```python
# Inline calculation of total cost
total_cost = (
    base = get_base_price();
    tax = get_tax_rate();
    base + tax
)

# Inline checks in function parameters
process( (x = get_value(); x ? x.status == 200 ?? --) )
```

### Compile-Time Desugaring
At parse time, the parser wraps the semicolon-terminated sequence of statements inside an immediately invoked function expression (IIFE):
```python
def _block():
    base = get_base_price()
    tax = get_tax_rate()
    return base + tax

total_cost = _block()
```

### Semicolon Requirement Note
Because Python's tokenizer ignores newlines inside parentheses to support implicit line continuation, semicolons `;` are strictly required even when statements are written on separate lines. Without semicolons, the tokenizer would treat the statements as if they were written on a single line, causing syntax errors. Semicolons also serve as the explicit marker distinguishing an expression block from standard parenthesized expressions.

---

## 13. Expression & Statement Decorators (`@decorator`)

### Motivation
In standard Python, decorators (`@decorator`) are strictly limited to preceding function, method, or class definitions. However, developers often need to wrap individual expressions or statement blocks in timing, logging, error-handling, or threading wrappers. 

Allowing decorators to precede any statement or expression—stacked vertically on the line above, matching standard decorator conventions—provides a highly readable, cohesive syntax for inline custom flow control and monadic blocks.

### Proposed Syntax
Decorators are placed on the line immediately preceding the target statement or expression:

#### **1. Expression / Assignment Decorators**
Decorating an assignment or standalone expression statement:
```python
# Timed execution of an assignment
@timed
result = heavy_calculation(x)

# Error-handling fallback
@ignore_errors(default=0)
value = db.fetch_user()
```

#### **2. Statement / Block Decorators**
Decorating a loop, condition, context block, or anonymous statement block:
```python
# Decorating a loop statement
@retry(attempts=3)
$ item <~ items:
    process(item)

# Decorating a code block
@transaction(db):
    user.balance -= 100
    user.save()
```

---

### Compile-Time Desugaring

#### **Expression/Assignment Decorators**
When a decorator precedes an assignment statement (`target = expr`) or a return statement (`-> expr`), the compiler wraps the target expression in a zero-argument lambda and passes it to the decorator:
```python
# For:
# @timed
# result = heavy_calculation(x)
result = timed(lambda: heavy_calculation(x))

# For return statement:
# @logged
# -> compute_value()
return logged(lambda: compute_value())
```

#### **Statement/Block Decorators**
When a decorator precedes a compound statement (like a loop or condition) or a block statement, the compiler wraps the target block in a nested helper function and invokes the decorator with it:
```python
# For:
# @retry(3)
# $ item <~ items: ...
def _loop_block():
    for item in items:
        process(item)

retry(3)(_loop_block)
```

Because standard Python syntax does not allow `@decorator` lines before general statements or assignments (only before `def` and `class`), this vertically stacked syntax has zero grammatical conflicts in the PEG parser.

---

## 14. Object & Dictionary Destructuring (`{prop1, prop2, **rest} = target`)

### Motivation
Standard Python requires writing explicit attribute or key lookups (`name = user.name; role = user.role`) for each property, which is verbose and repetitive when extracting multiple properties from an object or dictionary. Bringing JavaScript-style destructuring to Loh makes variable extraction extremely clean and DRY, supporting unified dict/object lookup, nested patterns, and rest captures.

### Proposed Syntax
Using brace syntax on the left-hand side of an assignment to extract attributes (for objects) or keys (for dictionaries):
```python
# Extracts name, role, and captures all remaining properties into rest
{name, role, **rest} = user

# Nested destructuring (extracts profile.address)
{name, profile: {address}} = user

# Explicit class type matching & extraction
User{name, role} = user
```

### Compile-Time Desugaring
To support nested destructuring, type validation, and fallback handling, the compiler borrows logic from CPython's structural pattern matching (`match` statement) and desugars the destructuring assignment into a single-case conditional block:
```python
# For: {name, role, **rest} = target
match target:
    # 1. Match dictionary targets:
    case {"name": name, "role": role, **rest} if isinstance(target, dict):
        pass
    # 2. Match general object targets:
    case _obj if hasattr(_obj, "__dict__"):
        name = _obj.name
        role = _obj.role
        rest = {k: v for k, v in _obj.__dict__.items() if k not in ("name", "role")}
    # 3. Raise an error if target is invalid:
    case _:
        raise ValueError("Cannot destructure value: object structure does not match pattern")
```



## 16. Margin-Controlled Multiline Strings (Margin Stripping)

### Motivation
Multiline string literals in Python preserve all leading whitespace and indentation of the source file. To keep code indentation consistent, developers are forced to write multi-line strings flush-left (which breaks the visual structure of the enclosing block) or call helper functions like `textwrap.dedent` at runtime.

Loh can solve this at compile-time by introducing margin controls using the pipe symbol `|`.

### Proposed Syntax
A leading `|` character at the start of each line in a multiline string indicates the margin boundary. The compiler automatically strips the margin character and all preceding whitespace from each line of the string literal at compile-time:
```python
query = """
    |SELECT name, role
    |FROM users
    |WHERE active = +
"""
```

### Compile-Time Desugaring
The parser strips the leading whitespace and `|` markers from the string token value during AST generation, compiling to a clean static string:
```python
query = "SELECT name, role\nFROM users\nWHERE active = True\n"
```

---


## 18. Function Pipeline Composition Operator (`f >> g`)

### Motivation
Loh's pipe operator `x |> f |> g` is excellent for feeding values through pipelines. However, sometimes developers need to compose functions into a new function without applying it to a value immediately. Using bitwise shift `>>` for composition aligns with functional programming paradigms.

### Proposed Syntax
```python
# Composes standard functions: h(x) equivalent to g(f(x))
process_text = str.strip >> str.upper >> (s) -> s.replace(" ", "_")

result = process_text("  hello world  ")  # "HELLO_WORLD"
```

### Compile-Time Desugaring
The parser translates `f >> g` into a nested lambda expression:
```python
process_text = lambda *args, **kwargs: (lambda s: s.replace(" ", "_"))(str.upper(str.strip(*args, **kwargs)))
```

---

## 19. Parameter Internal Aliasing (`external_name => internal_name`)

### Motivation
Exposing long, descriptive argument names to API callers is best practice for readability (e.g. `principal_amount`), but repeating these verbose names inside the function body clutters the logic. By allowing parameters to specify an internal alias using Loh's `=>` arrow symbol (which maps to `as`), functions can expose clean external names while keeping their internal implementation extremely concise.

### Proposed Syntax
```python
def calculate_interest(principal_amount => p, interest_rate => r, duration_years => t):
    # Inside the function, we use the short, clean internal aliases:
    return p * r * t
```

### Compile-Time Desugaring
The compiler keeps the external names in the function signature, and prepends local variable bindings at the top of the function body:
```python
def calculate_interest(principal_amount, interest_rate, duration_years):
    p = principal_amount
    r = interest_rate
    t = duration_years
    return p * r * t
```

---

## 20. Inline Expression Loops & Comprehensions

### Motivation
Standard Python comprehensions are verbose due to repeating the loop variable three times (e.g. `[u.email for u in users if u.is_active]`). By using the loop sigil `$` as the implicit element placeholder, Loh can make comprehensions extremely concise.

### Proposed Syntax
```python
# List comprehension
active_emails = [$.email <~ users ? $.is_active]

# Dict comprehension
user_map = {$.id: $ <~ users}

# Generator expression as function argument
total_price = sum($.price <~ items)
```

### Compile-Time Desugaring
Translates to standard Python comprehensions by replacing `$` with a compiler-generated variable:
```python
active_emails = [_item.email for _item in users if _item.is_active]
user_map = {_item.id: _item for _item in users}
total_price = sum(_item.price for _item in items)
```

---

## 21. The Syntactic Merge Operator (`{{ }}`)

### Motivation
When calling multiple methods or setting attributes on a single target, developers must repeat the receiver prefix (e.g. `plt.plot()`, `plt.title()`) or use context managers which add runtime lookup and function call overhead. Loh introduces a compile-time syntactic merge operator `{{ }}` that acts as a zero-cost macro, distributing a target expression over a block of statements.

### Proposed Syntax
```python
# Distributing a method receiver
plt. {{
    plot(x, y)
    title("Sales")
    show()
}}

# Distributing a function wrapper
print( {{
    "Processing..."
    calculate()
    "Done."
}} )

# Declarative Enum Generation (composing Class syntax with {{ }})
Color:Enum:
    {{ RED GREEN BLUE }} = auto()
```

### Compile-Time Desugaring
At parse-time, the compiler expands each statement or semicolon-separated fragment inside the double curly braces by prepending the target expression and appending the postfix expression (if any):
```python
# Expands to:
plt.plot(x, y)
plt.title("Sales")
plt.show()

# Expands to:
print("Processing...")
print(calculate())
print("Done.")

# Expands to:
class Color(Enum):
    RED = auto()
    GREEN = auto()
    BLUE = auto()
```

---

## 22. Anonymous Expression Classes (`::(...)`)

### Motivation
Creating one-off mock classes or ad-hoc event callbacks without polluting the module namespace is a common need. Expression-wrapped anonymous classes allow defining and instantiating class structures inline.

### Proposed Syntax
```python
mock = :BaseClass:(
    .get_value():
        -> 42
)

raw_object = ::(
    .x = 10,
    .y = 20
)
```

### Compile-Time Desugaring
Generates a unique local class declaration and instantiates it:
```python
class _AnonClass_1(BaseClass):
    def get_value(self):
        return 42
        
mock = _AnonClass_1()
```

---

## 23. First-Class Type-Scoped Variables

### Motivation
Elevating type hints from static analysis metadata into lexical, type-scoped local variable slots.

### Proposed Syntax
* **Anonymous Parameters**: `.foo(:int, :float, :str, var)`
* **Most-Recent Assignment Rule**: `print(:int)` resolves to the most recently assigned integer variable in the local scope.
* **Value-First Binding (`x:b`)**: Passes the value `x` using the type parameter `:b` as the target identifier key.
* **Type-First Binding (`:a = y`)**: Explicitly targets the type-scoped parameter `:a` and assigns `y` to it.

### Compile-Time Desugaring
The compiler tracks type-scoped local bindings statically and maps them to compiler-generated names in the local scope.

---

## 24. Class Attribute Property Shorthand (`.name: property`)

### Motivation
In standard Python, declaring class properties requires using `@property` and `@name.setter` decorators, which introduces significant boilerplate and nesting for simple getters and setters. Loh can provide a clean, declarative shorthand syntax for defining properties directly in the class block.

### Proposed Syntax
1. **Naked/Auto-Implemented Property**:
   Declaring a property without a body automatically backs it with a private, single-underscore-prefixed attribute:
   ```python
   Circle::
       .radius: property
   ```
2. **Read-Only Property**:
   ```python
   Circle::
       .radius: float
       
       .area: property -> 3.14159 * .radius ** 2
   ```
3. **Read-Write Property**:
   ```python
   Circle::
       ._radius: float
       
       .radius: property:
           get: -> ._radius
           set(val): ._radius = val
   ```

### Compile-Time Desugaring
The parser compiles these properties to standard Python property decorator methods:
1. **Naked/Auto-Implemented**:
   ```python
   class Circle:
       @property
       def radius(self):
           return self._radius
       @radius.setter
       def radius(self, val):
           self._radius = val
   ```
2. **Read-Only**:
   ```python
   class Circle:
       @property
       def area(self):
           return 3.14159 * self.radius ** 2
   ```
3. **Read-Write**:
   ```python
   class Circle:
       @property
       def radius(self):
           return self._radius
       @radius.setter
       def radius(self, val):
           self._radius = val
   ```


---

## 25. Conditional Pattern Matching (`? expr ?== pattern:`)

### Motivation
Python's structural pattern matching requires a full `match` statement, which adds a nesting level even when checking a single pattern. Integrating pattern matching directly into the `if` (`?`) condition allows checking and binding pattern variables inline.

### Proposed Syntax
```python
# Check if response matches success pattern and bind data
? response ?== {"status": "success", "data": payload}:
    process(payload)
??:
    raise ValueError("Request failed")
```

### Compile-Time Desugaring
At parse-time, the conditional pattern matches are desugared into a single-case match block or a helper function call:
```python
_matched = False
match response:
    case {"status": "success", "data": payload}:
        _matched = True
        process(payload)

if not _matched:
    raise ValueError("Request failed")
```

---

## 26. Parameter Destructuring in Function Signatures

### Motivation
When passing dictionaries or objects as options or complex structures to functions, developers manually unpack keys or attributes inside the function body. Allowing destructuring patterns directly in the function parameter signature keeps argument unpacking declarative and DRY.

### Proposed Syntax
```python
def draw_point({x, y, color="black"}):
    print(x, y, color)
```

### Compile-Time Desugaring
The compiler replaces the destructured parameter with an auto-generated parameter, and prepends destructuring assignments at the top of the function body:
```python
def draw_point(_point_obj):
    # Retrieve x, y, and color safely with default fallbacks
    x = _point_obj.x if hasattr(_point_obj, "x") else _point_obj["x"]
    y = _point_obj.y if hasattr(_point_obj, "y") else _point_obj["y"]
    color = _point_obj.color if hasattr(_point_obj, "color") else (_point_obj["color"] if "color" in _point_obj else "black")
    
    print(x, y, color)
```


## 28. None-Filtering Postfix Operator (`lst ~?`)

### Motivation
Filtering `None` values out of iterables is a frequent task that requires verbose list comprehensions (e.g. `[x for x in lst if x is not None]`). A dedicated none-filtering postfix operator `~?` provides a clean, zero-overhead way to compact sequences.

### Proposed Syntax
```python
# Filters out None values from the list
clean_results = fetch_results() ~?
```

### Compile-Time Desugaring
Translates directly into a list comprehension filtering out `None`:
```python
clean_results = [_item for _item in fetch_results() if _item is not None]

---

## 29. Call-Site Attribute Initializer Shorthand (`Circle(.radius = 5.0)`)

### Motivation
When constructing an object, setting several initial attributes usually requires writing explicit parameters in the constructor or declaring variable assignments on separate lines after instantiation. Using dot-prefixed arguments in the call-site argument list allows directly initializing object attributes inline, providing clean symmetry with constructor parameter properties.

### Proposed Syntax
```python
# Create an instance and set attributes directly
c = Circle(.radius = 5.0, .color = "blue")
```

### Compile-Time Desugaring
At parse-time, call-site arguments prefixed with a dot are extracted and compiled into an immediately invoked helper function that assigns properties to the instantiated object:
```python
def _init_Circle():
    _temp = Circle()
    _temp.radius = 5.0
    _temp.color = "blue"
    return _temp
c = _init_Circle()
```

---

## 30. Variable Swap Operator (`a <=> b`)

### Motivation
Swapping the values of two variables in standard Python requires writing a tuple unpacking assignment (`a, b = b, a`). Providing a dedicated infix swap operator `<=>` makes this common operation visually distinct, symmetrical, and concise.

### Proposed Syntax
```python
a <=> b
```

### Compile-Time Desugaring
The swap operator desugars directly into standard Python tuple assignment at parse-time:
```python
a, b = b, a
```

---

## 31. First-Class Regular Expression Literals (`/pattern/flags`)

### Motivation
Python requires importing the `re` module and calling `re.compile(r"pattern")` to work with regular expressions. Introducing JavaScript-style regex literals `/pattern/` avoids boilerplate imports and compile steps, making regex usage lightweight.

### Proposed Syntax
```python
# Matches email address case-insensitively
email_rx = /^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$/i

? email_rx.match(user_input):
    print("Valid email")
```

### Compile-Time Desugaring
The parser compiles `/pattern/flags` directly to `re.compile` calls, mapping flags like `i` (IGNORECASE), `m` (MULTILINE), and `s` (DOTALL):
```python
import re
email_rx = re.compile(r"^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$", re.IGNORECASE)
```

---

## 32. Main Entrypoint Block (`main:`)

### Motivation
The standard Python check `if __name__ == "__main__":` is notoriously verbose and boilerplate-heavy. Providing a simple top-level `main:` block cleanups module entrypoints.

### Proposed Syntax
```python
main:
    print("Application started")
```

### Compile-Time Desugaring
The parser maps the `main:` block header directly to the standard Python entrypoint guard:
```python
if __name__ == "__main__":
    print("Application started")
```

---

## 33. Standalone Argument Forwarding Shorthand (`*` and `**`)

### Motivation
Forwarding all positional and keyword arguments from one function to another (e.g. `log(*args, **kwargs)`) is a highly common delegation pattern that is verbose to repeat. Loh partially supports keyword forwarding via bare `**` parameters and arguments (binding to a local dictionary variable named `"**"`). We want to extend this to support bare `*` for positional arguments, creating a fully symmetric forwarding mechanism.

### Proposed Syntax
1. **Keyword Forwarding (Existing)**:
   ```python
   def setup_config(name, **):
       # Excess keyword arguments are captured in a dict variable named "**"
       **['name'] = name
       -> **
       
   # Call setup_config and forward parameters
   setup_config("test", **)
   ```
2. **Positional Forwarding (Proposed)**:
   Allowing a bare `*` at the end of a parameter signature to capture excess positional arguments, and a bare `*` in calls to forward them:
   ```python
   def delegate(a, *):
       # Positional arguments are captured in a tuple variable named "*"
       log(*, a)
   ```

### Compile-Time Desugaring
At parse-time, the parser treats `*` and `**` as standard variable Name nodes containing the identifiers `"*"` and `"**"`.
1. **Signature Parsing**:
   - `def foo(**)` maps the keyword arguments parameter (`kwarg`) to `Name("**")`.
   - `def foo(*)` maps the excess positional arguments parameter (`vararg`) to `Name("*")`.
2. **Call Parsing**:
   - `func(**)` maps to a keyword argument node `_PyAST_keyword(arg=NULL, value=Name("**"))` (dictionary unpacking).
   - `func(*)` maps to a starred expression node `_PyAST_Starred(value=Name("*"), ctx=Load)` (sequence unpacking).


---

## 34. Static and Class Method Signatures (`+method()` and `++method()`)

### Motivation
Declaring static and class methods in standard Python requires writing `@staticmethod` and `@classmethod` decorators above the function signature. Since Loh uses a single dot prefix `.` to signify instance context (`self`), using `+` and `++` prefixes for static and class methods provides a clean, compile-time shorthand that matches class method syntax.

### Proposed Syntax
```python
Helper::
    # Static method (no cls/self)
    +parse_int(val: str) -> int:
        return int(val)

    # Class method (receives cls)
    ++create_default() -> Helper:
        return cls()
```

### Compile-Time Desugaring
The parser compiles these prefixes to standard Python method decorators and parameter signatures:
```python
class Helper:
    @staticmethod
    def parse_int(val: str) -> int:
        return int(val)

    @classmethod
    def create_default(cls) -> Helper:
        return cls()
```

---

## 35. Unified Dict/Object Safe Navigation (`obj~.key`)

### Motivation
When working with heterogeneous data sources (like dynamic JSON payloads or ORM models), developers must write separate code paths to handle attribute lookup versus dictionary key lookup. Extending safe navigation `~.` to dynamically fall back to dictionary key access if the target is a dictionary provides a single, unified safe lookup.

### Proposed Syntax
```python
# Works whether 'user' is a custom object or a dictionary
role = user~.profile~.role
```

### Compile-Time Desugaring
The parser desugars `~.` into helper lookups that check for attribute presence first, falling back to dictionary get if applicable:
```python
_temp1 = user
if _temp1 is not None:
    _temp2 = _temp1.profile if hasattr(_temp1, "profile") else (_temp1.get("profile") if isinstance(_temp1, dict) else None)
    if _temp2 is not None:
        role = _temp2.role if hasattr(_temp2, "role") else (_temp2.get("role") if isinstance(_temp2, dict) else None)
    else:
        role = None
else:
    role = None
```

---

## 36. None-Safe Destructuring Assignment (`~[a, b] = target`)

### Motivation
Unpacking iterables or lists throws `ValueError` or `TypeError` if the collection is `None` or has fewer elements than expected. Standard destructuring default patterns are verbose. Prepending `~` to a destructuring assignment provides a safe unpack that automatically binds `None` to missing elements rather than throwing exceptions.

### Proposed Syntax
```python
# Safe unpack that binds None if list is short or None
~[first, second] = get_items()
```

### Compile-Time Desugaring
The parser desugars safe unpacking into checks on length and presence:
```python
_temp = get_items()
first = _temp[0] if _temp and len(_temp) > 0 else None
second = _temp[1] if _temp and len(_temp) > 1 else None
```

---

## 37. Compile-Time Class Constants (`const NAME = value`)

### Motivation
In standard Python, class attributes are mutable class variables by default. To make attributes read-only constants, developers must declare custom properties or read-only descriptors. Enforcing constants at compile-time prevents mutation without introducing runtime descriptor overhead.

### Proposed Syntax
```python
Circle::
    # Declares a compile-time class constant
    const PI = 3.14159
```

### Compile-Time Desugaring
At parse-time, the parser tracks all class-level `const` identifiers statically. If the compiler detects any assignments to a `const` field (like `Circle.PI = 4` or `.PI = 4` within methods), it raises a compile-time `SyntaxError`.

---

## 38. None-Safe Pipe Operator (`~|>`)

### Motivation
Piping variables through function chains (`data |> func1 |> func2`) throws `TypeError` if one of the intermediate steps or the initial value evaluates to `None`. Incorporating safe navigation into the pipe chain via `~|>` (matching safe navigation `~.`, safe subscript `~[`, and safe call `~()`) lets the pipeline short-circuit and evaluate to `None` if any stage is `None`.

### Proposed Syntax
```python
# Short-circuits and returns None if fetch_user() is None
username = fetch_user() ~|> .name ~|> str.strip
```

### Compile-Time Desugaring
At parse-time, the `~|>` operator compiles to inline conditional checks that short-circuit and propagate `None`:
```python
_val1 = fetch_user()
_val2 = _val1.name if _val1 is not None else None
username = str.strip(_val2) if _val2 is not None else None
```

---

## 39. Multi-Line Arrow Functions `(args) ->:`

### Motivation
Python's lambda expressions are strictly restricted to a single expression. Writing complex callbacks requires nesting local helper function definitions, which is verbose. Supporting block-bodied arrow functions provides clean inline multi-line callback declarations.

### Proposed Syntax
```python
# Pass a multi-line callback to a map function
doubles = list(map( (x) ->:
    y = x * 2
    -> y + 1
, numbers))
```

### Compile-Time Desugaring
At parse-time, a block-bodied arrow function is compiled by defining a unique local helper function and replacing the expression with the helper's reference:
```python
def _lambda_callback_1(x):
    y = x * 2
    return y + 1

doubles = list(map(_lambda_callback_1, numbers))
```

---

## 40. List Append/Push Operator (`lst << item`)

### Motivation
Appending single items to list-like collections in standard Python is in-place and returns `None` (e.g. `lst.append(item)`), which prevents fluent method chaining. Overloading the left-shift bitwise operator `<<` as a push operator allows appending items while returning the mutated collection receiver.

### Proposed Syntax
```python
# Mutates the list and returns it, allowing fluent chaining
users = [] << User("Alice") << User("Bob")
```

### Compile-Time Desugaring
At parse-time, the operator `<<` on list/set literals or identifiers compiles directly to a helper method that performs the append operation and returns the collection:
```python
def _loh_push(collection, item):
    if hasattr(collection, "append"):
        collection.append(item)
    elif hasattr(collection, "add"):
        collection.add(item)
    return collection

users = _loh_push(_loh_push([], User("Alice")), User("Bob"))
```

---

## 41. Parallel Pipe Operator (`|>*`)

### Motivation
Executing operations in parallel across iterables (e.g. fetching URLs or resizing images) usually requires importing `concurrent.futures`, setting up executor context managers, and mapping collections. A dedicated parallel pipe operator `|>*` lets developers map functions across iterables in parallel using a thread/process pool with zero boilerplate.

### Proposed Syntax
```python
# Fetches all page payloads in parallel
results = urls |>* fetch_page
```

### Compile-Time Desugaring
At parse-time, the `|>*` operator compiles directly to a map call running inside a thread pool executor:
```python
import concurrent.futures

def _parallel_map(func, iterable):
    with concurrent.futures.ThreadPoolExecutor() as executor:
        return list(executor.map(func, iterable))

results = _parallel_map(fetch_page, urls)
```

---

## 42. Optional Parameter Default Shorthand (`param = ?`)

### Motivation
In function signatures, parameters that default to `None` (optional parameters) require writing `= None` or `= --` (Loh's empty none constant). Using a single query mark `?` as the default value matches the syntax of optional/nullable values in other languages, making default-to-None assignments extremely concise.

### Proposed Syntax
```python
def fetch(url: str, timeout: int = ?, headers = ?):
    print(timeout, headers)
```

### Compile-Time Desugaring
At parse-time, a default value of `?` in a function parameter signature is desugared to Loh's empty none constant `None` (represented by CPython's `Py_None`):
```python
def fetch(url: str, timeout: int = None, headers = None):
    print(timeout, headers)
```

---

## 43. Dictionary Inversion Operator (`~dict`)

### Motivation
Inverting a mapping dictionary (swapping its keys and values) is a common task that typically requires writing a dictionary comprehension (e.g. `{v: k for k, v in d.items()}`). Overloading the unary bitwise NOT operator `~` on dictionary expressions provides a clean, native syntax for dict inversion.

### Proposed Syntax
```python
# Inverts the translation mapping
english_to_french = {"one": "un", "two": "deux"}
french_to_english = ~english_to_french
```

### Compile-Time Desugaring
At parse-time, if the operand of the unary `~` operator is a dictionary literal or expression, the compiler desugars it into a dictionary comprehension:
```python
french_to_english = {value: key for key, value in english_to_french.items()}
```

---

## 44. Pipe Operator Enhancements (`|>`, `<|`, `~|>`, `<|~`, `|*>`, `<*|`, etc.)

### Motivation
Loh currently supports the basic forward pipe operator (`|>`) for left-to-right single-argument pipeline flow. To support full functional expression flow, we want to introduce the complete family of pipe operators covering:
1. **Direction**: Forward (`|>`) and Backward (`<|`) (for parentheses-free nesting).
2. **Unpacking**: Positional (`*`) and Keyword (`**`) argument forwarding.
3. **None-Awareness (Safety)**: Using `~` to short-circuit to `None` if the input is `None` (matching Loh's safe-navigation `~.`, `~[]`, `~~`).

### Proposed Syntax
- **Forward & Backward Single-Argument Pipes**:
  ```python
  "   hello   " |> str.strip |> print
  print <| len <| str.strip <| data  # Avoids print(len(str.strip(data)))
  ```
- **Argument Unpacking Pipes**:
  ```python
  (2, 5) |*> range |> list |> print  # range(2, 5) -> [2, 3, 4]
  range <*| (2, 5)                  # range(2, 5)
  {"sep": ", ", "end": "\n"} |**> print <| "hello"
  ```
- **None-Aware (Safe) Pipes**:
  ```python
  fetch_user() ~|> (u) -> u.name |> print  # None-safe forward
  f <|~ x                                  # None-safe backward
  ```

### Compile-Time Desugaring
- `x |> f` $\rightarrow$ `f(x)`
- `f <| x` $\rightarrow$ `f(x)`
- `args |*> f` $\rightarrow$ `f(*args)`
- `kwargs |**> f` $\rightarrow$ `f(**kwargs)`
- `x ~|> f` $\rightarrow$ `f(x) if x is not None else None`
- `f <|~ x` $\rightarrow$ `f(x) if x is not None else None`

---


## 45. Index/Key-Safe Subscripting (`lst[x]~`)

### Motivation
In standard Python, looking up a missing key in a dictionary throws a `KeyError`, and looking up an out-of-bounds index in a list/tuple/string throws an `IndexError`. Developers are forced to write verbose boundary checks or use `dict.get()`. To make index and key lookups safe, Loh introduces a trailing-tilde subscript syntax `lst[x]~` that returns `None` if the element does not exist.

### Proposed Syntax
```python
x = lst[10]~
y = dct["missing_key"]~
```

### Compile-Time Desugaring
At parse-time, this desugars into a call to `_loh_safe_index(primary, slice)` which catches `IndexError` and `KeyError` at runtime and returns `None` (`~`).

---

## 46. Partial Function Application (`foo...(_)` / `foo...(arg)`)

Allows pre-binding arguments of a function call to create a new, partially-applied function. It is written using the postfix ellipsis `...` operator on a callable call:

### Proposed Syntax
- **Explicit Placeholders**: Using `_` as a placeholder to specify unbound/hole arguments.
  ```python
  divide_by_two = divide...(_, 2)
  # Desugars to: (x) -> divide(x, 2)
  
  calc_ten = calc...(_, 10, _)
  # Desugars to: (x, y) -> calc(x, 10, y)
  ```
- **Implicit Left-to-Right Binding**: If no placeholders are used, arguments are bound from left to right, and any remaining arguments are forwarded:
  ```python
  add_five = add...(5)
  # Desugars to: (*args, **kwargs) -> add(5, *args, **kwargs)
  
  greet_alice = greet...(name="Alice")
  # Desugars to: (*args, **kwargs) -> greet(*args, name="Alice", **kwargs)
  ```

---

## 47. Dotted Function Definitions (PEP 542)

### Motivation
PEP 542 proposes dot notation in function signatures to attach a function directly as a method of an existing object/class, avoiding the boilerplate of defining it locally and assigning it manually.

### Proposed Syntax
To explicitly distinguish between class methods (which receive the implicit `.` self parameter) and plain functions attached to namespaces:

1. **Class Methods (with `self` / `.`)**:
   The parameter list explicitly starts with `.`:
   ```python
   MyClass.method(., x):
       .x = x
   ```
2. **Plain Namespace Functions (no `self`)**:
   The parameter list does not start with `.`:
   ```python
   config.helper(x):
       -> x * 2
   ```

### Compile-Time Desugaring
At parse-time, the dotted function name is split. The function is defined using the base name, and an attribute assignment is appended immediately after:
```python
# MyClass.method(., x) desugars to:
def method(., x):
    .x = x
MyClass.method = method

# config.helper(x) desugars to:
def helper(x):
    -> x * 2
config.helper = helper
```




## 50. Post-Expression Exception Rescue (`expr \n ?^ Exception:`)

### Motivation
Standard Python exception handling (`try`/`except`) is statement-based and verbose, forcing developers to wrap single lines of code in deeply indented try blocks. 

By allowing an exception rescue block (`?^`) to immediately follow an expression statement, we can handle potential errors inline without nesting or introducing block-level boilerplate.

### Proposed Syntax
The rescue block applies directly to the preceding expression statement.

```python
parse_configuration(data)
?^ (FileNotFoundError | JSONDecodeError) => error:
    log_error("Could not parse configuration:", error)
    load_fallback()
```

#### Syntax Variations
- **Wildcard Rescue**: If no exception type is specified, it implicitly catches `Exception` (just like Loh's inline rescue `expr ?^ fallback` and standard `except:`):
  ```python
  parse_configuration(data)
  ?^:
      load_fallback()
  ```
- **Multiple Handlers**: You can stack multiple handlers sequentially:
  ```python
  parse_configuration(data)
  ?^ FileNotFoundError:
      load_from_backup()
  ?^ JSONDecodeError:
      load_default()
  ```

### Compile-Time Desugaring
At parse-time, the expression statement is wrapped inside a `try` block, and the subsequent `?^` blocks are mapped to `except` clauses:

```python
try:
    parse_configuration(data)
except (FileNotFoundError, JSONDecodeError) as error:
    log_error("Could not parse configuration:", error)
    load_fallback()
```

### Grammar Design
In `Grammar/python.gram`, we can define under `compound_stmt`:
```peg
rescued_stmt:
    | body=expression NEWLINE handlers=rescue_handlers
```
Where `rescue_handlers` parses one or more `?^` clauses:
```peg
rescue_handlers:
    | '?^' exc=expression? var=rescue_var_binding? ':' body=block rest=rescue_handlers?
```
And `rescue_var_binding` parses the `=> var` alias mapping.

---

## 51. Bare Expression Resolution (`m;`) & Self-Referencing Dicts with Lazy Expressions

### Motivation
In Loh, backtick expressions `` `expr` `` defer evaluation by creating a zero-argument thunk (`_LohLazy`). When building self-referential dictionaries (e.g. `m = {'a': 1, 'b': 2, 'c': `m['a']`}`), deferring evaluation during dictionary construction prevents `NameError` because `m` is not bound until after dictionary creation finishes. 

However, after `m` is bound, developers often want to convert all lazy proxies in `m` into concrete, eager values without writing boilerplate iteration loops or calling helper functions.

In standard Python, a standalone bare expression statement like `m;` or `m` is a no-op. In Loh, executing a bare reference statement `m;` triggers a parse-time resolution pass `_loh_resolve(m)` that evaluates and mutates all `_LohLazy` entries inside `m` in-place!

### Proposed Syntax

```python
# Self-referencing dict constructed with lazy thunks:
m = {'a': 1, 'b': 2, 'c': `m['a']`}

# Bare reference statement resolves 'm' in-place:
m;

# 'm' now contains plain concrete values!
print(m)  # {'a': 1, 'b': 2, 'c': 1}

# Inline 1-line style:
config = {'port': 8080, 'url': `f"http://localhost:{config['port']}"`}; config;
```

### Compile-Time Desugaring
In `python.gram`, top-level `expression_stmt` nodes (bare expression statements) are desugared at parse-time to wrap the target variable in a resolution pass:

```python
# Loh Source:
m;

# Desugars at parse-time to:
_loh_resolve(m)
```

Where `_loh_resolve(obj)` is a C runtime builtin in `bltinmodule.c` that iterates dict/list elements, calls `lazy_resolve()` on any `_LohLazy` objects, and replaces them in-place with concrete resolved values.

---

## 52. Parenthesized Prefix Function Application (`(f) expr`)

### Motivation
When calling single-argument functions or wrapping expressions in transformations (e.g. `print("User: " + name)`, `int(a + b)`), standard Python requires enclosing the arguments in trailing parentheses `f(...)`. In functional pipelines or nested function compositions, trailing parentheses add syntax noise `f(g(h(x)))`. Writing `(f) expr` provides a concise right-to-left prefix function application syntax that greedily wraps the right-hand side expression as `f(expr)`.

### Proposed Syntax
By placing a parenthesized function or callable `(f)` immediately before an expression `expr`, `(f)` acts as a greedy prefix application operator, wrapping the entire right-hand expression:

```python
# Function wrapping single-line expressions:
(print) "User count: " + str(count)   # -> print("User count: " + str(count))
val = (int) a + b                     # -> int(a + b)
dist = (abs) x1 - x2                  # -> abs(x1 - x2)
name = (str.upper) first + " " + last # -> str.upper(first + " " + last)

# Prefix chaining (evaluates right-to-left):
result = (int) (abs) a - b            # -> int(abs(a - b))
```

### Precedence & Scoping Rules
`(f) expr` operates at the top-level `expression` precedence rule in Python's PEG grammar (matching `lambda` and `if/else` conditional expressions). It greedily consumes the entire expression to its right.

To bound the application to a specific sub-expression, standard grouping parentheses can be used around the call:
```python
# Bounded call (only applies x to y, then adds 1):
((x) y) + 1                           # -> x(y) + 1
```

### Compile-Time Desugaring
At parse-time, the PEG parser translates `(f) expr` into a standard CPython function call AST node (`_PyAST_Call`):

```peg
expression[expr_ty]:
    | '(' f=disjunction ')' e=expression { _PyAST_Call(f, _PyPegen_singleton_seq(p, e), NULL, NULL, NULL, EXTRA) }
```

---

## 53. Compile-Time Expression Evaluation (` ``expr`` `)

### Motivation
In standard Python, constant expressions (such as unit calculations `1024 * 1024 * 64`, static regex compiles `re.compile(...)`, or pre-computed lookup tables) incur runtime evaluation overhead every time a module or function executes. While standard Python performs basic literal folding, complex expressions or function calls cannot be evaluated at compile-time.

Loh introduces double backticks ` ``expr`` ` as a `constexpr` operator that evaluates the enclosed expression at parse-time/compile-time during AST generation, emitting pre-computed constant literals directly into CPython bytecode.

This completes Loh's 3-tier backtick execution continuum:
* **` ``expr`` `**: **Compile-Time Evaluation** (evaluated early at parse-time)
* **`expr`**: **Standard Evaluation** (evaluated normally at runtime)
* **`` `expr` ``**: **Lazy Evaluation** (evaluated late on-demand via zero-argument thunk)

### Proposed Syntax
```python
# Pre-calculates 67108864 directly into bytecode constant
MAX_PAYLOAD = ``64 * 1024 * 1024``

# Pre-calculates 86400 seconds
DAY_IN_SECONDS = ``24 * 60 * 60``

# Pre-compiles regex pattern at compile-time and stores in module constants
URL_PATTERN = ``re.compile(r"^https?://[^\s]+$")``

# Pre-builds static lookup table at compile-time
POWERS_OF_TWO = ``{i: 2**i for i in range(16)}``

# Freezes timestamp/version string at build-time
BUILD_TIME = ``time.strftime("%Y-%m-%d %H:%M:%S")``
```

### Technical & AST Desugaring
1. **Tokens**:
   Define `DOUBLE_BACKTICK` (` `` `) in [Tokens](file:///Users/robbarnes/Development/loh/Grammar/Tokens).

2. **Grammar Rule in [python.gram](file:///Users/robbarnes/Development/loh/Grammar/python.gram)**:
   ```peg
   atom[expr_ty]:
       | ''``'' e=test ''``'' { _PyPegen_constexpr_eval(p, e) }
   ```

3. **AST Helper**:
   At parse-time, `_PyPegen_constexpr_eval` in `action_helpers.c` evaluates `e` using Python's C-API and returns a `_PyAST_Constant` node. If `e` cannot be evaluated at parse-time (e.g., depends on undefined runtime variables), a parse-time `SyntaxError: constexpr expression cannot be evaluated at compile time` is raised.

---

## 54. Symbolic Fold Expressions (`*(items)`, `+(items)`, `|(flags)`)

### Motivation
Reducing a collection with a binary operator (such as multiplying all numbers in a list, summing elements, or combining bitwise flags) in standard Python requires writing manual loops or importing helper functions like `functools.reduce(operator.mul, factors)`. Borrowing from C++17 fold expressions, Loh allows prefixing a parenthesized collection with a binary operator symbol to fold that operator across all elements in the collection.

### Proposed Syntax
```python
# Multiply all numbers in a list -> reduce(operator.mul, factors)
product = *(factors)

# Sum all numbers in a list -> sum(prices)
total = +(prices)

# Bitwise OR fold across a list of flags -> reduce(operator.or_, flag_list)
all_flags = |(flag_list)

# Logical AND fold across booleans -> all(bool_list)
all_valid = &(bool_list)
```

### Compile-Time Desugaring
At parse-time, the PEG parser translates binary operator fold expressions into calls to `functools.reduce` using the corresponding operator function:

```python
# *(factors) desugars to:
import functools as _functools, operator as _op
_functools.reduce(_op.mul, factors)

# +(prices) desugars to:
sum(prices)
```

---

## 41. Asset & File Imports (`/ "file.ext" : type => alias`)

### Motivation
In Loh, forward slash `/` syntax is used for module imports (e.g. `/math/sqrt => s`). Importing raw data files (JSON, YAML, HTML templates, binary models) in standard Python requires writing repetitive file-reading boilerplate (`with open(...) as f:`). To make loading static assets DRY and declarative, Loh extends slash import syntax to quoted string paths, automatically parsing JSON/YAML or reading text/binary files directly into local variables.

### Proposed Syntax
Because standard Python module imports use unquoted identifiers (`/math`), using a **quoted string literal** (`"..."` or `'...'`) immediately following `/` unambiguously identifies a **File Asset Import**.

#### **A. Basic Asset Imports with Auto-Inferred Variable Names**
If `=> alias` is omitted, the variable name is automatically inferred from the file basename (stripping folder paths and extensions):
```python
/ "configs/database.json" : dict       # Parses JSON into local variable 'database'
/ "templates/email/header.html" : str  # Reads text into local variable 'header'
/ "../shared/logo.png" : bytes         # Reads binary into local variable 'logo'
```

#### **B. Explicit Variable Binding (`=> alias`)**
```python
/ "configs/prod.json" : dict => config
/ "templates/view.html" : str => view_tpl
/ "models/net.onnx" : bytes => weights
```

### Type Modifiers & Extension Inferences

| Import Expression | Inferred / Explicit Mode | Resulting Data Type |
| :--- | :--- | :--- |
| **`/ "data.json" : dict`** | Auto-inferred `.json` | Parsed Dictionary / List |
| **`/ "config.yaml" : dict`** | Auto-inferred `.yaml` | Parsed Dictionary / List |
| **`/ "page.html" : str`** | Auto-inferred `.html` / `.txt` / `.md` | `str` (Text String) |
| **`/ "blob.dat" : bytes`** | Auto-inferred `.bin` / `.dat` / `.png` | `bytes` (Byte Stream) |
| **`/ str "data.json"`** | Explicit `str` modifier | Raw JSON string (unparsed) |
| **`/ bytes "data.json"`** | Explicit `bytes` modifier | Raw UTF-8 bytes |

### Path Resolution & Compile-Time Desugaring
Asset paths inside quotes support relative folder paths (`"../../assets/data.json"`). Path resolution occurs relative to the directory of the executing `.loh` source file.

At parse-time, the PEG parser in `python.gram` parses `/ "path" : type => alias` and compiles it into an inline file loading helper call:
```python
# / "configs/database.json" : dict => config desugars to:
import json as _json, pathlib as _pathlib
_asset_path = _pathlib.Path(__file__).parent / "configs/database.json"
with open(_asset_path, "r", encoding="utf-8") as _f:
    config = _json.load(_f)
```

---

## 42. Class-Body Dot Properties & Auto-Dataclass Synthesis (`.foo = 1`)

### Motivation
Creating record classes, value objects, and dataclasses in standard Python requires importing and applying the `@dataclass` decorator, writing type hints, or manually implementing `__init__`, `__repr__`, `__eq__`, and pattern-matching `__match_args__`.

In Loh, declaring dot-prefixed fields directly inside a class body (e.g. `.name: str`, `.role = "guest"`) explicitly identifies instance attributes of the class. When a class contains dot-prefixed fields at class scope without a custom constructor, Loh automatically synthesizes dataclass / record behavior at compile-time.

### Proposed Syntax
```python
# Multi-field record / dataclass
User::
    .name: str
    .role = "guest"
    .active = +

    .is_admin() -> .role == "admin"

# Single-line pure value record
Point:: .x: float; .y: float
```

### Compile-Time Desugaring
When the parser encounters dot-prefixed attribute declarations `.field` directly inside a class block (`Class::`):
1. **Auto-Generates `__init__`**: Synthesizes `.(.name, .role = "guest", .active = +)` setting `self.name`, `self.role`, and `self.active`.
2. **Auto-Generates `__repr__`**: Returns `User(name='Alice', role='guest', active=True)`.
3. **Auto-Generates `__eq__`**: Compares value equality across all declared fields.
4. **Auto-Generates `__match_args__`**: Sets `__match_args__ = ("name", "role", "active")`, enabling positional structural pattern matching (`?== User("Alice", r):`).

---

## 55. Side-Effect Pipeline Tapping (`|>!`) / `also` Scope Function

### Motivation
In functional data transformation chains, inspecting, logging, or executing side-effects on intermediate values currently requires breaking the pipeline to assign a temporary variable or using verbose wrapper lambdas. Inspired by Kotlin's `also` scope function, Loh introduces the **Tap Pipe Operator** `|>!`.

### Proposed Syntax
`expr |>! action` evaluates the right-hand action for side-effects (with `$` bound to `expr` or passing `expr` as an argument), but always evaluates to the original left-hand value `expr`, allowing pipeline processing to continue unimpeded.

```python
result = (
    fetch_raw_payload()
    |>! print("Raw payload:", $)        # Inspects data, returns raw payload
    |> parse_json
    |>! logger.info("Parsed user: {}", $.id)
    |> save_to_db
)
```

### Compile-Time Desugaring
At parse-time, the `|>!` operator compiles to an inline helper call that executes the side-effect and returns the left-hand receiver value:

```python
def _loh_tap(val, func):
    func(val)
    return val

result = save_to_db(_loh_tap(parse_json(_loh_tap(fetch_raw_payload(), lambda _: print("Raw payload:", _))), lambda _: logger.info(...)))
```

---



## 57. Conditional Value Filtering (`takeIf` / `takeUnless`) (`?|` / `?!|`)

### Motivation
Filtering or validating a scalar expression without declaring temporary variables, writing multi-line `if`/`else` blocks, or repeating the variable name in a ternary expression is a frequent requirement in data processing pipelines. Inspired by Kotlin's `takeIf` and `takeUnless`, Loh introduces the conditional value filter operators `?|` and `?!|`.

### Proposed Syntax
- **`expr ?| condition`** (`takeIf`): Evaluates to `expr` if `condition` (or predicate with `$`) is truthy, otherwise evaluates to `None` (`~`).
- **`expr ?!| condition`** (`takeUnless`): Evaluates to `expr` unless `condition` (or predicate with `$`) is truthy, otherwise evaluates to `None` (`~`).

```python
# Evaluates to score if score >= 70, otherwise None (~)
passed_score = raw_score ?| ($ >= 70)

# Seamlessly chain with None-coalescing (~~)
grade = student.score ?| ($ >= 70) ~~ "Failing"

# takeUnless variant: reject blank strings
clean_name = raw_name ?!| ($.strip() == "") ~~ "Guest"
```

### Compile-Time Desugaring
At parse-time, the filter expression compiles to a single-evaluation ternary check:

```python
_val = raw_score
passed_score = _val if (_val >= 70) else None

_val2 = student.score
grade = (_val2 if (_val2 >= 70) else None)
if grade is None: grade = "Failing"
```

---

## 58. Scope `defer` Block for Automatic Cleanup (`defer:` / `*:`)

### Motivation
Resource cleanup (closing file handles, releasing locks, restoring previous environment settings) in standard Python requires wrapping code in explicit `try ... finally` blocks. When multiple resources are opened sequentially, nested `try/finally` blocks lead to deep indentation ("pyramid of doom"). Inspired by Swift and Go, Loh can support scope-level `defer:` (or standalone `*:`) blocks that automatically schedule cleanup execution when the enclosing function or lexical block exits, regardless of whether it exits via `return`, `break`, `continue`, or an exception.

### Proposed Syntax
```python
open_connection(url):
    conn = connect(url)
    defer: conn.close()  # Executed automatically when function exits
    
    # Or using Loh's cleanup sigil (*:)
    *: conn.close()
    
    data = conn.fetch_data()
    -> process(data)
```

### Compile-Time Desugaring
At parse-time, the PEG parser rewrites the statements following a `defer:` block into a CPython `try...finally` AST node (`_PyAST_Try`), placing the `defer:` statements inside the `finally` block:
```python
def open_connection(url):
    conn = connect(url)
    try:
        data = conn.fetch_data()
        return process(data)
    finally:
        conn.close()
```

---

## 59. Key-Path Attribute Extractors (`.property`)

### Motivation
Extracting attribute values across collections (e.g. `[user.name for user in users]` or `list(map(lambda u: u.name, users))`) is a common pattern that requires writing verbose anonymous functions or comprehensions. Swift introduces key-path syntax (`\.name`) to treat property paths as first-class functions. In Loh, using a standalone leading-dot `.property` in expression contexts (like pipe pipelines or function arguments) creates an anonymous getter lambda.

### Proposed Syntax
```python
# Extract attribute 'name' from each element in users
names = users |> map(.name)

# Sort users by 'age'
sorted_users = users |> sort_by(.age)

# Multi-level nested key-path lookup
zip_codes = users |> map(.address.zip_code)
```

### Compile-Time Desugaring
At parse-time, a standalone dot-prefixed property access in expression position desugars into an anonymous single-parameter lambda:
```python
# 'users |> map(.name)' desugars to:
map(lambda _x: _x.name, users)

# '.address.zip_code' desugars to:
lambda _x: _x.address.zip_code
```

---

## 60. Half-Open and Closed Range Literals (`a..<b`, `a...b`)

### Motivation
Standard Python uses `range(start, stop)` for iteration, which is half-open (exclusive of `stop`). Loh currently supports `a..b` range literals ([README.md](file:///Users/robbarnes/Development/loh/README.md#L791-L800)). Borrowing explicit range operators from Swift makes range boundaries crystal clear:
- `a..<b`: Half-open range (includes `a`, excludes `b`).
- `a...b`: Closed range (includes both `a` and `b`).

### Proposed Syntax
```python
# Half-open range: iterates 0, 1, 2, 3, 4 (range(0, 5))
$ i <~ 0..<5:
    print(i)

# Closed range: iterates 0, 1, 2, 3, 4, 5 (range(0, 6))
$ i <~ 0...5:
    print(i)
```

### Compile-Time Desugaring
At parse-time, the PEG parser translates range operators directly into `range()` calls:
```python
# 0..<5 desugars to:
range(0, 5)

# 0...5 desugars to:
range(0, 5 + 1)
```

---

## 61. Trailing Closure Syntax (`func(...) (args) ->:`)

### Motivation
When passing anonymous callbacks or inline handler blocks to higher-order functions (such as asynchronous tasks, animation wrappers, event handlers, or transaction runners), placing multi-line lambdas inside function call parentheses `func(arg, lambda: ...)` creates awkward nesting and visually cluttered syntax. Borrowing from Swift, Loh can support **Trailing Closure Syntax**, allowing a trailing lambda parameter to be written outside the call parentheses as an attached block. This enables custom higher-order functions to feel like native language control structures.

### Proposed Syntax
When the final parameter of a function call is a callable/closure, the lambda expression can be placed after the closing parenthesis `)` as a trailing block:

```python
# Standard inline lambda inside parentheses:
fetch_user(user_id=42, callback=(user) -> print(user.name))

# Trailing closure block attached to call:
fetch_user(user_id=42) (user) ->:
    print(f"Loaded user: {user.name}")
    save_to_cache(user)

# Zero-argument trailing closure block:
animate(duration=2.0) () ->:
    view.alpha = 1.0
```

### Compile-Time Desugaring
At parse-time, the PEG parser in [python.gram](file:///Users/robbarnes/Development/loh/Grammar/python.gram) detects a function call followed by a trailing arrow lambda block `(args) ->:` and appends the lambda AST node to the function call's positional argument list (`_PyAST_Call`):

```python
# 'fetch_user(user_id=42) (user) ->:' desugars to:
fetch_user((user) -> (
    print(f"Loaded user: {user.name}"),
    save_to_cache(user)
), user_id=42)
```

---

## 62. Indented Multiline String Literals (`i"""..."""` / `i'''...'''`)

### Motivation
Multiline string literals in Python (`"""..."""` or `'''...'''`) preserve all leading whitespace and indentation of the source file. To keep code indentation clean and readable within functions or nested blocks, developers are often forced to write multiline strings flush-left (which breaks visual indentation hierarchy) or call runtime helpers like `textwrap.dedent`.

Loh introduces the `i` string prefix (**i**ndented / **i**ndent-stripped). At compile-time, the parser automatically measures the minimum common leading indentation of all non-empty lines in an `i"""..."""` string literal and strips it, producing a clean, properly formatted string with zero runtime overhead.

### Proposed Syntax
```python
fetch_users() -> str:
    # The 'i' prefix strips the leading 8 spaces of indentation at compile-time
    query = i"""
        SELECT id, username, email
        FROM users
        WHERE active = +
        ORDER BY id DESC
    """
    -> query

# Also works with single quotes i'''...''' and f-string combinations (if"""...""")
message = if"""
    Hello {.name},
    Your order #{.order_id} has been processed!
    Total: ${.total:.2f}
"""
```

### Compile-Time Desugaring
During AST construction, the parser inspects `i"""..."""` tokens, calculates the common leading whitespace of non-empty lines, strips that whitespace, and returns a standard CPython string constant (`_PyAST_Constant`).

```python
# Compiles into static string constant in bytecode:
query = "SELECT id, username, email\nFROM users\nWHERE active = True\nORDER BY id DESC\n"
```

---

## 63. Dictionary Deep Merge Operator (`dictA / dictB`, `dictA /= dictB`)

### Motivation
Standard Python 3.9 dictionary union (`dictA | dictB`) performs a **shallow** top-level merge. If both dictionaries contain nested sub-dictionaries, `dictA | dictB` completely replaces the nested dictionary from `dictA` with the one from `dictB`, discarding any non-conflicting nested keys.

Inspired by Nix's configuration overlay system, Loh repurposes the binary division operator `/` on dictionary operands (`dictA / dictB`) to perform a **recursive (deep) dictionary merge**. Sub-dictionaries are merged recursively, preserving nested key-value pairs from both sides while overriding matching keys with values from the right-hand dictionary.

### Proposed Syntax
```python
base_config = {
    "server": {"host": "localhost", "port": 8080},
    "logging": {"level": "INFO", "file": "app.log"}
}

override_config = {
    "server": {"port": 9000},
    "logging": {"level": "DEBUG"}
}

# Deep merge using '/': preserves 'host' and 'file' while overriding 'port' and 'level'
merged = base_config / override_config

print(merged)
# Outputs:
# {
#     "server": {"host": "localhost", "port": 9000},
#     "logging": {"level": "DEBUG", "file": "app.log"}
# }

# In-place deep merge assignment:
config /= override_config
```

### Compile-Time Desugaring / C Runtime Helper
Because standard CPython dict objects do not implement `__truediv__` (`/`), Loh implements deep merge at the runtime C level in `Objects/dictobject.c` or desugars `dictA / dictB` into a call to a built-in C helper function `_loh_dict_deep_merge(dictA, dictB)`:

```python
def _loh_dict_deep_merge(d1, d2):
    result = d1.copy()
    for k, v in d2.items():
        if k in result and isinstance(result[k], dict) and isinstance(v, dict):
            result[k] = _loh_dict_deep_merge(result[k], v)
        else:
            result[k] = v
    return result

merged = _loh_dict_deep_merge(base_config, override_config)
```

---



## 65. Postfix Conditional Statements (`statement ? condition`)

### Motivation
In standard Python, executing a single statement conditionally requires wrapping it in a full `if condition:` block on a new line, adding vertical height and block nesting for simple early returns, guard assertions, or logging calls.

By leveraging Loh's question mark sigil `?` for `if` and `?!` for `if not`, single-statement guard checks can be appended to the end of a statement row as a postfix modifier.

### Proposed Syntax
```python
# Early returns / exits
-> ? error_occurred
-> default_val ?! is_valid

# Control flow guards
$> ? i == 10       # Break loop if i == 10
$< ?! active       # Continue loop if not active

# Single statement calls & assignments
cleanup() ? finished
log("Warning") ? debug_mode
```

### Compile-Time Desugaring
At parse-time, a statement followed by `? condition` or `?! condition` desugars directly into a standard CPython `If` AST node:

```python
# '-> ? error_occurred' desugars to:
if error_occurred:
    return

# 'cleanup() ? finished' desugars to:
if finished:
    cleanup()

# '-> default_val ?! is_valid' desugars to:
if not is_valid:
    return default_val
```

---


## 67. Call-Site Parameter Auto-Forwarding (`func(=)`)

### Motivation
In standard Python, passing local variables to a function call with matching parameter names requires repeating each variable name explicitly (`func(host=host, port=port, username=username)`). 

Loh expands its property punning syntax (`=var`) to support a naked call-site punning operator `=`. Placing `=` inside a function call tells the compiler to automatically inspect the target function's parameter names and bind any matching local variables from the caller scope.

### Proposed Syntax
```python
def configure_db(host, port, username, password, timeout=30):
    ...

def setup_app():
    host = "localhost"
    port = 5432
    username = "admin"
    password = "secret"
    
    # Auto-matches host, port, username, password from local scope:
    db = configure_db(=)

    # Explicit override combined with auto-forwarding:
    test_db = configure_db(timeout=5, =)
```

### Compile-Time Desugaring
At parse-time, the compiler resolves `=` by matching the target function's parameter signature against variables available in the local scope, desugaring to standard keyword arguments:

```python
# 'configure_db(=)' desugars to:
configure_db(host=host, port=port, username=username, password=password)

# 'configure_db(timeout=5, =)' desugars to:
configure_db(timeout=5, host=host, port=port, username=username, password=password)
```

---

## 68. Value-Returning Loops & Break with Value (`$: ... $> val`)

### Motivation
In languages like Rust, infinite loops (`loop`) can act as expressions that evaluate to a value when terminated with `break value;`. This pattern is extremely common when polling an API, retrying operations until success, or searching data structures. 

In Loh, infinite loops are represented concisely by `$:` (while True) and loop break is `$>`. Extending `$>` to accept an expression payload (`$> expr`) allows `$:` loops to yield an evaluation result directly to variable assignments or function parameters without declaring mutable accumulator variables outside the loop.

### Proposed Syntax
```python
# Retries an API request until successful and yields the result
data = $:
    res = fetch_api()
    ? res.status == 200:
        $> res.json()  # Breaks out of loop and evaluates the '$:' block to res.json()
    time.sleep(1)

# Polling a queue until an item arrives
item = $:
    val = queue.pop()
    ? val !~~:
        $> val
```

### Compile-Time Desugaring
At parse-time, an expression-assigned `$:` loop is wrapped in a compiler-generated immediately invoked helper function:

```python
def _loop_expr():
    while True:
        res = fetch_api()
        if res.status == 200:
            return res.json()
        time.sleep(1)

data = _loop_expr()
```

---

## 69. Dynamic Built-in Type Modification (Clearing `Py_TPFLAGS_IMMUTABLETYPE`)

### Motivation
In standard CPython (3.10+), core built-in types such as `str`, `int`, `float`, `list`, `dict`, and `tuple` set the `Py_TPFLAGS_IMMUTABLETYPE` flag on their `PyTypeObject` definitions. This flag causes `type_setattro` in the VM to block dynamic attribute or method assignment, raising `TypeError: can't set attributes of built-in/extension type 'str'`.

Because Python string literals (`"hello"`), numbers (`42`), and collections (`[1, 2]`) are directly instantiated as built-in types by the C runtime, developers cannot extend core types with utility methods at runtime. By clearing `Py_TPFLAGS_IMMUTABLETYPE` (and adjusting `tp_setattro`) during Loh runtime startup, core built-in types become dynamically extensible.

### Proposed Architecture & C Changes
During Loh interpreter initialization in `pythonrun.c` / `pylifecycle.c`, Loh iterates over core built-in types and modifies their type flags:

1. **Clear Immutable Flag**:
   ```c
   PyUnicode_Type.tp_flags &= ~Py_TPFLAGS_IMMUTABLETYPE;
   PyLong_Type.tp_flags &= ~Py_TPFLAGS_IMMUTABLETYPE;
   PyList_Type.tp_flags &= ~Py_TPFLAGS_IMMUTABLETYPE;
   PyDict_Type.tp_flags &= ~Py_TPFLAGS_IMMUTABLETYPE;
   ```
2. **Enable Dynamic Attribute Assignment**:
   Update `tp_setattro` on target built-in types to allow setting methods and attributes directly on built-in type dictionaries (`PyUnicode_Type.tp_dict`).

### Loh Usage
This enables defining extension blocks directly on built-in types:

```python
# Extending built-in str directly at runtime
str::
    .shout():
        -> .upper() + "!"

# Standard string literals immediately gain the new method:
print("hello".shout())  # Outputs: "HELLO!"
```

---

## 70. Expression Early Return via Return Sigil in Coalescing Operators (`~~ ->` & `?? ->`)

### Motivation
In languages like Rust, unwrapping an optional value or returning early from a function when a value is missing or invalid is a fundamental flow-control pattern (`let val = option?;`). In Loh, returning from a function uses the arrow sigil `->` (e.g. `-> val`).

Instead of introducing special postfix operators, allowing the return statement/expression `->` to appear as an expression on the right-hand side of Loh's existing None-coalescing (`~~`) and truthy-coalescing (`??`) operators enables zero-boilerplate early returns for missing or falsy values.

### Proposed Syntax
```python
get_user_avatar(user_id):
    # If fetch_user returns None (~), immediately return None (-> ~) from function
    user = fetch_user(user_id) ~~ -> ~

    # If avatar is None (~), return fallback string early
    avatar = user.avatar ~~ -> "default_avatar.png"

    # If validate returns falsy, return error message early
    status = validate(user) ?? -> "invalid status"

    -> avatar
```

### Compile-Time Desugaring
At parse-time, when the RHS of a `~~` or `??` operator is a return expression `-> val`, the compiler desugars the binary expression into an inline conditional statement:

```python
# 'user = fetch_user(user_id) ~~ -> ~' desugars to:
_tmp = fetch_user(user_id)
if _tmp is None:
    return None
user = _tmp

# 'avatar = user.avatar ~~ -> "default_avatar.png"' desugars to:
_tmp = user.avatar
if _tmp is None:
    return "default_avatar.png"
avatar = _tmp

---

## 71. Relational Constants (`% VAR == val`), Container Freezing (`% expr`), and Class Invariants (`% condition`)

### Motivation
Standard Python lacks first-class language constructs for compile-time/runtime constants, frozen data structures, and state invariants. Enforcing immutability or validation requires verbose annotations (`typing.Final`), wrapper calls (`MappingProxyType`, `tuple()`), or manual assertions scattered throughout methods.

Loh unifies immutability, constant declarations, and contract enforcement under the `%` sigil. By using comparison operators (`==` for value equality binding, `===` for deep identity lock) instead of assignment (`=`), constant declarations read as immutable relational assertions, completely unifying constant declarations with class state invariants (`% condition`).

### Proposed Syntax

#### 1. Immutable Constant Declarations (`% VAR == value` / `%% VAR === value`)
Prefixing a variable binding with `%` and using relational comparison `==` (or `===`) marks the binding as an immutable equality assertion. Reassigning to `% VAR` raises a compiler or runtime error:

```python
# Standard Immutable Constant (value equality assertion)
% MAX_CONNECTIONS == 100
% API_BASE_URL == "https://api.example.com"

# Deep Immutable Constant (identity equality lock, recursively freezing container contents)
%% CONFIG === {
    "db": {"host": "localhost", "port": 5432},
    "roles": ["admin", "user"]
}
```

#### 2. Container Freezing (`% expr` / `%% expr`)
Using `%` as a prefix operator on a container literal or expression freezes it into an immutable structure:

```python
# Shallow freeze (converts top-level list to tuple)
immutable_list = % [1, 2, 3]

# Deep freeze (recursively converts all nested dicts/lists to immutable proxies/tuples)
deep_frozen_config = %% {"server": {"host": "localhost"}}
```

#### 3. Class State Invariants (`% condition` / `%% condition`)
Placing `% condition` directly inside a class body declares a class state invariant using relational comparison operators (`==`, `>=`, `<=`, `!=`). The compiler automatically verifies this condition:
- **`% condition` (Exit Invariant)**: Verified upon completing `.(...)` (constructor) and exiting every public instance method.
- **`%% condition` (Strict Invariant)**: Verified on both entry and exit of every method.

```python
BankAccount::
    .(.balance: float = 0.0):
        ...

    .deposit(amount: float):
        .balance += amount

    .withdraw(amount: float):
        .balance -= amount

    # CLASS INVARIANT: balance must NEVER be negative
    % .balance >= 0.0, "Account balance cannot be negative"
```

### Compile-Time Desugaring
- `% VAR == val` $\rightarrow$ Evaluates assignment `VAR = val` at definition and marks `VAR` as read-only in the AST symbol table.
- `%% VAR === val` $\rightarrow$ Evaluates assignment with deep container freeze and marks `VAR` as read-only.
- `% [1, 2]` $\rightarrow$ `tuple([1, 2])`
- `%% {"a": 1}` $\rightarrow$ `types.MappingProxyType(...)`
- `% .balance >= 0` $\rightarrow$ Injects `assert self.balance >= 0` check calls at the exit of `__init__` and all public class methods.

---

## 72. Unary `**` Object-to-Dict Coercion Operator (`**obj`)

### Motivation
Converting a class instance, object, dataclass, or custom structure into a plain dictionary in standard Python requires calling `vars(obj)`, accessing `obj.__dict__`, using `dataclasses.asdict(obj)`, or writing custom serialization helpers.

Since `**` is Python's native operator for keyword dict unpacking inside function calls and dict literals (e.g. `{"a": 1, **other_dict}`), extending `**` to act as a unary prefix operator in general expression contexts provides a zero-boilerplate, unified syntax for coercing any object's state into a dictionary.

### Proposed Syntax
```python
# Unary ** on a class instance converts its instance dictionary to a dict
acc = Account("Alice", 500)
d = **acc
print(d)  # {"owner": "Alice", "balance": 500}

# Direct dictionary merging with object attributes
full_profile = {"status": "active", **user_instance}
```

### Compile-Time Desugaring
At parse-time, a unary `**` expression `**expr` desugars into a runtime helper call `_loh_to_dict(expr)`:
```python
def _loh_to_dict(obj):
    if isinstance(obj, dict):
        return obj
    if hasattr(obj, "__dict__"):
        return dict(obj.__dict__)
    if hasattr(obj, "__slots__"):
        return {k: getattr(obj, k) for k in obj.__slots__ if hasattr(obj, k)}
    return dict(obj)

---

## 73. Hybrid Array-Dict Table Literals (`{ 10, key: "val" }`)

### Motivation
In standard Python, list literals `[1, 2]` store ordered positional elements, while dictionary literals `{"a": 1}` store key-value pairs. Mixing unkeyed elements and key-value pairs inside a single brace literal `{}` is a syntax error (or parsed as a set if no keys are present).

Inspired by Lua tables, Loh can allow unkeyed positional elements to be interleaved with key-value entries inside `{}` dictionary literals. Unkeyed elements are automatically assigned zero-indexed integer keys (`0, 1, 2, ...`) in order of appearance.

### Proposed Syntax
Unkeyed positional values and keyed entries can be freely interleaved inside dict literals:
```python
# Unkeyed positional elements auto-index starting at 0:
request = { "GET", auth_type: "bearer", "/api/v1/users", timeout: 5000, "payload" }

# Desugars at parse-time to standard dict:
# { 0: "GET", "auth_type": "bearer", 1: "/api/v1/users", "timeout": 5000, 2: "payload" }

print(request[0])          # "GET"
print(request[1])          # "/api/v1/users"
print(request[2])          # "payload"
print(request["timeout"])  # 5000
```

### Technical Note on Implicit Dict Merging
Allowing unkeyed values inside `{}` dictionary literals creates a parsing overlap with implicit dict unpacking (where `{ "a": 1, {"b": 2} }` auto-merges nested dicts without `**`). To resolve this grammar ambiguity, adopting hybrid array-dict table literals may require removing or restricting implicit dict merging in favor of explicit unpacking `{ "a": 1, **{"b": 2} }`.

### Compile-Time Desugaring
The PEG parser tracks the count of unkeyed positional items in the dict literal and wraps them in integer key-value AST `Dict` pairs (`0: item1`, `1: item2`, etc.).

---

## 74. Flexible Positional Wildcard Unpacking (`*, y, z` / `a, *, z` / `a, b, *`)

### Motivation
Standard Python unpacking allows `a, *rest, c = seq`, requiring a named variable like `rest` or `_` to capture unneeded elements. When developers only care about specific positional elements (e.g. head & tail, or last N elements), using a bare wildcard `*` at the beginning, middle, or end of unpacking targets provides a clean, self-documenting way to discard unused positional items.

Note: Unlike soft-default unpacking, wildcard `*` unpacking strictly enforces sequence bounds; if the sequence contains fewer items than the required non-wildcard targets, a `ValueError` is raised (identical to standard Python unpacking).

### Proposed Syntax
```python
# 1. Beginning wildcard (*, y, z): Right-aligned unpacking (discards leading items)
*, y, z = (10, 20, 30, 40)   # y = 30, z = 40
*, y, z = (10, 20)           # y = 10, z = 20 (exact match)
*, y, z = (10,)              # Raises ValueError: not enough values to unpack

# 2. Middle wildcard (a, *, z): Head & Tail unpacking (discards intermediate items)
a, *, z = (1, 2, 3, 4, 5)   # a = 1, z = 5
a, *, z = (1, 2)            # a = 1, z = 2 (exact match)

# 3. End wildcard (a, b, *): Left-aligned unpacking (discards trailing items)
a, b, * = (100, 200, 300, 400) # a = 100, b = 200
```

### Compile-Time Desugaring
At parse-time, a bare `*` target in an assignment target tuple/list desugars into an anonymous compiler-generated discard variable (e.g. `_` or `_discard`):
```python
# 'a, *, z = seq' desugars to:
a, *_discard, z = seq
del _discard
```

---

## 75. Inline Postfix Expression Decorators (`expr @ dec1 @ dec2`)

### Motivation
In standard Python, decorators `@decorator` are placed vertically on lines preceding function, method, or class declarations. Applying wrappers or monadic transformers to individual expressions inline (such as timing, retrying, caching, or logging) currently requires wrapping expressions in nested function calls `dec2(dec1(expr))` or verbose helper functions.

Introducing an inline postfix decorator operator `@` allows decorators to be chained from left to right directly after expressions.

### Proposed Syntax
```python
# Inline decorator application
result = heavy_computation(data) @ timed

# Chained inline decorators
user = fetch_user(user_id) @ retry(attempts=3) @ cached(ttl=60)
```

### Alignment with Vertically Stacked Decorators
In Python, vertically stacked `@` decorators execute bottom-up (inside-out):
```python
@dec2
@dec1
def foo(): ...

# Evaluates to: dec2(dec1(foo))
```

Inline postfix expression decorators (`expr @ dec1 @ dec2`) evaluate left-to-right (`(expr @ dec1) @ dec2`), ensuring that `dec1` wraps `expr` first and `dec2` wraps `dec1`. This creates a 1:1 syntactic alignment between vertical stacked `@` decorators and horizontal inline `@` decorators:

$$\text{\texttt{expr @ dec1 @ dec2}} \quad \equiv \quad \begin{array}{l} \text{\texttt{@dec2}} \\ \text{\texttt{@dec1}} \\ \text{\texttt{expr}} \end{array} \quad \equiv \quad \text{\texttt{dec2(dec1(expr))}}$$

### Compile-Time Desugaring
At parse-time, `expr @ dec` desugars into passing `expr` as a zero-argument lambda to `dec`:
```python
# 'fetch_user(id) @ retry(3) @ cached(60)' desugars to:
cached(60)(retry(3)(lambda: fetch_user(id)))
```

---

## 76. Universal Smart Resolution Operator (`%%`) for Auto-Await & Lazy Evaluation

### Motivation
Standard Python `await` raises a runtime `TypeError: object X can't be used in 'await' expression` when applied to non-awaitable values (e.g. `await 42` or `await "hello"`). Furthermore, evaluating deferred lazy expressions (`` lazy_val = `x + 5` ``) currently requires accessing the proxy variable or invoking its evaluation helper.

By reserving `%` for declaring `async` functions (`% func():`) and establishing `%%` as the expression-level **Universal Smart Resolution Operator**, Loh unifies async futures, lazy evaluation proxies, and scalar values into a single, failure-proof resolution model.

### Proposed Syntax
```python
# 1. Async Function Definition (Single '%')
% fetch_user_data(user_id):
    # 2. Smart Await on Coroutine (Double '%%')
    raw_json = %% api.get_async(f"/users/{user_id}")
    
    # Deferred Lazy Expression
    lazy_profile = `build_profile(raw_json)`
    
    # 3. Smart Resolution of Lazy Expression (Double '%%')
    profile = %% lazy_profile
    
    # 4. Safe Passthrough on Scalars (Double '%%') - Returns 42 directly without TypeError!
    val = %% 42
    
    -> profile
```

### Resolution Rules & Operational Semantics

$$\text{\texttt{\%\% expr}} = \begin{cases} 
\text{await } \textit{expr} & \text{if awaitable (coroutine / Task / Future)} \\ 
\textit{expr}\text{.eval()} & \text{if lazy backtick proxy (`` `expr` ``)} \\ 
\textit{expr} & \text{if plain value (42, "hello", dict, etc.)} 
\end{cases}$$

### Compile-Time & Runtime Desugaring
At parse-time, `%% expr` compiles to an internal runtime helper `_loh_smart_resolve(expr)`:

```c
PyObject* _loh_smart_resolve(PyObject* obj) {
    if (PyCoro_CheckExact(obj) || PyGen_CheckExact(obj) || _PyCoro_GetAwaitableIter(obj)) {
        return await(obj); // Executes CPython await opcode logic
    }
    if (LohLazy_CheckExact(obj)) {
        return LohLazy_Force(obj); // Evaluates lazy backtick proxy
    }
    Py_INCREF(obj);
    return obj; // Safe passthrough for scalars
}
```

---

## 77. Native `Result` / `Option` Types and Postfix `?` Unwrap Operator

### Motivation
In Rust, `Result<T, E>` (`Ok(val)` / `Err(err)`) and `Option<T>` (`Some(val)` / `None`) combined with the postfix `?` operator provide type-safe, explicit error handling without exception overhead or deep conditional nesting.

Standard Python relies heavily on exception raising or verbose manual type checking (`if isinstance(res, Err): return res`). Bringing a zero-dependency C-level `Result` type (`Ok`, `Err`) alongside a postfix `?` unwrap operator to Loh introduces Rust-grade error propagation directly into Python code.

### Proposed Syntax & Semantics

1. **Constructing `Result` Values**:
   - `Ok(val)` represents a successful result wrapping `val`.
   - `Err(err)` represents a failed result wrapping `err`.

2. **Postfix `?` Unwrap & Early Return**:
   Evaluating `expr?`:
   - If `expr` is `Ok(val)` or `Some(val)` $\rightarrow$ unwraps and evaluates to `val`.
   - If `expr` is `Err(err)` $\rightarrow$ immediately early-returns `Err(err)` from the current function.
   - If `expr` is `~` (`None`) $\rightarrow$ immediately early-returns `~` (`None`) from the current function.

3. **Exception Conversion (`expr ?^ ExceptionClass`)**:
   Catches the specified exception if raised, automatically wrapping it in `Err(e)` and returning it early.

### Example Code

```python
# Function returning Result[dict, str]
fetch_user_data(user_id: int) -> Result:
    ? user_id <= 0:
        -> Err("Invalid user ID")
    
    # Postfix '?' unwraps Ok(data) or early-returns Err(...) from fetch_user_data
    raw = http_get(f"/users/{user_id}")?
    user = parse_json(raw)?
    
    -> Ok(user)

main():
    res = fetch_user_data(42)
    ?== res:
        Ok(user): print(f"User loaded: {user.name}")
        Err(err): print(f"Failed to load user: {err}")
```

### Compile-Time & Runtime Desugaring
At parse-time, `val = expr?` desugars into an inline evaluation check:

```python
_tmp = expr
if isinstance(_tmp, Err):
    return _tmp
elif _tmp is None:
    return None
val = _tmp.unwrap() if hasattr(_tmp, 'unwrap') else _tmp
```

---

## 78. Parameter-Binding Pipe Operator (`|var>`)

### Motivation
Piping collections through standard transformations like mapping and filtering (e.g. `users |> filter((u) -> u.is_active) |> map((u) -> u.name)`) is clean but introduces boilerplate by forcing the developer to repeatedly declare arrow functions. 

Loh introduces a **Parameter-Binding Pipe Operator** (`|var>`) that binds the element of the piped collection to `var` in the following expression. This collapses the pipeline and the lambda definitions into a single cohesive structure.

### Proposed Syntax
Using `|var>` pipes a collection and binds its individual elements to `var` in the expression that follows:

```python
# 1. Standard filter/map operations:
active_names = (
    users 
    |u> filter(u.is_active)
    |u> map(u.name)
)

# 2. Combined filter & map (inline comprehension style):
active_names = users |u> u.name ? u.is_active
```

### Compile-Time Desugaring
At parse-time, the parameter-binding pipe desugars directly into list comprehensions or standard generator expressions, keeping runtime execution extremely fast and avoiding lambda function call overhead:

```python
# 1. 'users |u> filter(u.is_active)' desugars to:
[u for u in users if u.is_active]

# 2. 'users |u> map(u.name)' desugars to:
[u.name for u in users]

# 3. 'users |u> u.name ? u.is_active' desugars to:
[u.name for u in users if u.is_active]
```

### Grammar Design
In `Grammar/python.gram`, we can define a new binary expression node under the pipe precedence level:
```peg
binding_pipe_expr:
    | lhs=expression '|' var=NAME '>' rhs=expression {
        _PyPegen_make_binding_pipe(p, lhs, var, rhs)
    }
```
The AST helper `_PyPegen_make_binding_pipe` detects the shape of `rhs` and returns a list comprehension (`_PyAST_ListComp`) or generator expression:
- If `rhs` is `filter(cond)`: `[var for var in lhs if cond]`
- If `rhs` is `map(expr)`: `[expr for var in lhs]`
- If `rhs` is `expr ? cond`: `[expr for var in lhs if cond]`
- If `rhs` is a plain expression: `[rhs for var in lhs]`
```

---

## 79. Expression Holes and Implicit Lambdas via Diamond Operator (`<>`)

### Motivation
Writing inline lambda callbacks (`(x) -> x * 2`) or partially applying functions (`functools.partial`) in Python/Loh requires verbose syntax and parameter naming. 

Loh unifies implicit lambdas and partial function application around the **Diamond Operator** (`<>`) used as an expression placeholder (or "hole"). Any expression containing `<>` is automatically desugared into a single- or multi-parameter lambda at compile-time.

### Proposed Syntax

#### 1. Unary Lambda Shorthand (Member Access & Operators)
Using `<>` as a placeholder inside an expression automatically wraps it in a single-argument lambda:
```python
# Member property / predicate check
active = users |> filter(<>.is_active)
# Desugars to: filter((x) -> x.is_active, users)

# Method calls
admins = users |> filter(<>.has_role("admin"))
# Desugars to: filter((x) -> x.has_role("admin"), users)

# Operators
doubled = numbers |> map(<> * 2)
# Desugars to: map((x) -> x * 2, numbers)
```

#### 2. Partial Function Application & Argument Forwarding (`*<>` / `**<>`)
Passing `<>` as an argument to a function call pre-binds specific parameters, yielding a partially applied function. Using the splat operators `*<>` or `**<>` forwards arbitrary positional (`*args`) or keyword (`**kwargs`) arguments:
```python
# Bind the second argument of divide to 2:
divide_by_two = divide(<>, 2)
# Desugars to: (x) -> divide(x, 2)

# Bind keyword arguments:
greet_alice = greet(name="Alice", message=<>)
# Desugars to: (msg) -> greet(name="Alice", message=msg)

# Forward arbitrary positional arguments:
add_five = add(5, *<>)
# Desugars to: (*args) -> add(5, *args)

# Forward arbitrary keyword arguments:
config = configure(timeout=30, **<>)
# Desugars to: (**kwargs) -> configure(timeout=30, **kwargs)

# Full forwarding (functools.partial replacement):
log_error = log("ERROR", *<>, **<>)
# Desugars to: (*args, **kwargs) -> log("ERROR", *args, **kwargs)
```

#### 3. Multi-Parameter Lambdas
If multiple `<>` holes are present in the same expression or function call, they compile to a multi-parameter lambda matching the left-to-right order of appearance:
```python
# Source
foo(<>, 2, 3, z=<>)

# Desugars to:
lambda _1, _2: foo(_1, 2, 3, z=_2)
```

### Compile-Time Desugaring
At parse-time, the PEG parser detects occurrences of the `<>` token inside expressions. It calculates the boundary of the lambda scope (typically the nearest enclosing function argument or expression statement) and wraps it in a CPython `Lambda` AST node (`_PyAST_Lambda`) with compiler-generated arguments (`_1`, `_2`, `*_args`, `**_kwargs`, etc.):

```python
# Source
doubled = map(<> * 2, numbers)
log_error = log("ERROR", *<>, **<>)

# Compiles to CPython AST equivalent to:
doubled = map(lambda _1: _1 * 2, numbers)
log_error = lambda *_args, **_kwargs: log("ERROR", *_args, **_kwargs)
```

### Grammar Design
In `Grammar/python.gram`, we can define `<>` as a primary term:
```peg
hole_expr[expr_ty]:
    | '<>' { _PyPegen_make_hole(p) }
```
And add `hole_expr` to the `primary` rule. The compiler's parser AST phase then propagates these holes upward to the enclosing call or expression boundary to synthesize the lambda node. For splatted holes, `*` followed by `<>` is parsed as a starred expression where the child is a hole:
```peg
starred_hole[expr_ty]:
    | '*' a=hole_expr { _PyPegen_make_starred_hole(p, a) }
    | '**' a=hole_expr { _PyPegen_make_double_starred_hole(p, a) }
```

---

## 80. Read-Only Property Arrow Definitions (`.property -> expr`)

### Motivation
In standard Python, defining read-only class properties requires writing the `@property` decorator above a standard method definition. In Loh, omitting the parameter parentheses `()` in an instance method definition and directly using the return arrow `->` explicitly declares a read-only instance property, saving boilerplate and making class property definitions extremely clean.

### Proposed Syntax
```python
Circle::
    .radius: float
    
    # Read-only property (no parentheses in signature)
    .diameter -> .radius * 2
    
    # Standard method (requires parentheses)
    .scale(factor):
        .radius *= factor
```

### Compile-Time Desugaring
At parse-time, a class body entry with a dot prefix, an identifier, no parentheses, and a return arrow `->` is compiled as a getter function decorated with `@property`:

```python
class Circle:
    def __init__(self, radius: float):
        self.radius = radius
        
    @property
    def diameter(self):
        return self.radius * 2
        
    def scale(self, factor):
        self.radius *= factor
```

---

## 81. Safe Subscript Default Indexing (`lst~[idx, default]`)

### Motivation
Dictionary indexing supports `.get(key, default)` to avoid raising errors for missing keys. List indexing lacks a native fallback method, raising a loud `IndexError` when indices are out of bounds. Loh expands its none-safe subscript operator `~[` to support a second fallback argument, safely returning the default value if the index is out of bounds or if the list is `None`.

### Proposed Syntax
```python
# Safe list index with fallback
item = names~[5, "unknown"]
```

### Compile-Time Desugaring
At parse-time, the two-argument none-safe subscript compiles into an inline boundary check:
```python
_lst = names
item = _lst[5] if (_lst is not None and 0 <= 5 < len(_lst)) else "unknown"
```

---

## 82. Bracket-Based Generator Call Shorthand (`func[expr $ var := items]`)

### Motivation
Calling aggregation functions (like `sum()`, `any()`, `all()`, or `", ".join()`) with generator comprehensions requires typing enclosing parentheses twice (e.g. `sum(x for x in items)`). Using square brackets `[]` directly on a callable with a loop expression converts it into an inline generator comprehension.

### Proposed Syntax
```python
# Sum of ages
total_age = sum[ $.age $ := users ]

# Check if any user is admin
has_admin = any[ $.is_admin $ := users ]
```

### Compile-Time Desugaring
At parse-time, calling a function with square brackets containing a loop sigil `$` is compiled into a standard CPython function call with a generator expression:
```python
total_age = sum(_item.age for _item in users)
has_admin = any(_item.is_admin for _item in users)
```

```






