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

## 2. Pipe Operator Placeholders (`_`)

### Motivation
Allows calling multi-argument functions within a pipe chain without wrapping them in an anonymous lambda.

### Proposed Syntax
```python
data |> json.dumps(_, indent=4)
"log.txt" |> open(_, "w")
```

### Compile-Time Desugaring
If the parser detects a `_` identifier within the call argument list of the piped expression, it replaces `_` with the piped variable.

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

---

## 4. Inline Match-Case Expressions (`?==`)

### Motivation
Pattern matching (`?==`) in Loh is currently statement-only. In functional pipelines or variable declarations, developers often need to match a value against patterns and return/assign the result directly, matching the expression-oriented design of other modern programming languages.

### Proposed Syntax
```python
status_desc = code ?== {
    200: "Success",
    404: "Not Found",
    500: "Server Error",
    _: "Unknown Status"
}
```

### Compile-Time Desugaring
This expression parses into an AST that compiles to a nested set of conditional checks or an inline self-evaluating match structure:
```python
status_desc = ("Success" ? code == 200 
               ?? "Not Found" ? code == 404 
               ?? "Server Error" ? code == 500 
               ?? "Unknown Status")
```

---

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

## 12. Lazy Evaluation & Late-Bound Expressions (`` `expr` ``)

### Motivation
Eager evaluation in Python forces all variables, function arguments, and default parameters to be evaluated immediately. This is inefficient for optional computations and leads to major issues like mutable default arguments (`x=[]`) or static definitions (`now=datetime.now()`).

Loh can resolve this by introducing a **single, unified** lazy evaluation operator: the **Backtick Code Quote Operator (`` `expr` ``)**. This single operator is used for general-purpose lazy variables, deferred arguments, and call-time parameter defaults, with the compiler automatically resolving the appropriate scoping rules based on context.

---

### Proposed Syntax

#### **1. General Lazy Expressions**
Enclosing an expression in backticks `` ` `` defers its evaluation, creating a thunk. It evaluates automatically (and caches the result) when accessed:
```python
# Deferred evaluation
heavy_data = `load_huge_dataset()`

# Evaluated on first read and cached
print(heavy_data.summary)
```

#### **2. Late-Bound Parameter Defaults**
To evaluate a parameter default expression at call-time rather than definition-time, you simply enclose it in backticks:
```python
Foo::
    # 'x' defaults to the late-bound expression '.a + .b'
    .method(x = `.a + .b`):
        print(x)
```

---

### Scoping Rules

#### **A. Lexical Closures (For general expressions)**
Outside function signatures, lazy expressions use standard **Lexical Scope**. They capture the variables in their enclosing environment by reference at the moment the thunk is declared:
```python
a = 10
lazy_val = `a + 5`

a = 20
print(lazy_val) # Evaluates to 25 (uses the updated value of 'a')
```

#### **B. Method Body Scope (For parameter defaults)**
Inside function signatures, a quoted default argument evaluates inside the **Method Body Scope** upon invocation. This gives it access to variables only defined at call-time, such as the instance receiver context (`self`/`.`) and previous method parameters:
```python
calculate(width, height = `width * 2`):
    # 'height' can safely reference 'width' because it evaluates inside the body
    print(width, height)
```

---

### Compile-Time Desugaring

#### **Backtick Desugaring (General Expression)**
The parser compiles `` `expr` `` into a memoization proxy wrapping a lambda:
```python
heavy_data = _LohLazy(lambda: load_huge_dataset())
```
The runtime helper `_LohLazy` uses magic methods to force evaluation on first access and delegate properties.

#### **Parameter Default Desugaring**
The parser compiles a quoted default argument by setting its signature default to a sentinel, and injecting the fallback evaluation check at the start of the function body:
```python
# For: .method(x = `.a + .b`)
_MISSING = object()

def method(self, x=_MISSING):
    if x is _MISSING:
        x = self.a + self.b
    # ... Rest of body ...
```
This requires zero caller-side changes and remains fully compatible with standard CPython signature inspection.

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

---

## 15. Pattern Matching Loop Filters (`$ pattern <~ collection`)

### Motivation
When iterating over collections containing heterogeneous structured data (like list of dicts, tuples, or API responses), developers often need to filter items that match a certain pattern and extract sub-values. In standard Python, this requires a nested `match` block with a conditional `continue`/pass, adding significant indentation and boilerplate.

### Proposed Syntax
Loh can allow a pattern to be placed directly in the loop header instead of a simple target name/tuple. The loop body will execute only for items that match the pattern, with pattern variables bound in the loop body scope:
```python
# Loops only over active admins and prints their username
$ {"role": "admin", "active": +, "username": name} <~ users:
    print(name)
```

### Compile-Time Desugaring
Translates directly into a standard loop containing a structural pattern match check:
```python
for _item in users:
    match _item:
        case {"role": "admin", "active": True, "username": name}:
            print(name)
```

---

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

## 17. Implicit Receiver Context Blocks (`with obj:`)

### Motivation
When configuring an object or performing a sequence of method calls/attribute updates on a single target, standard Python forces you to repeat the object name (e.g., `plt.plot()`, `plt.title()`, `plt.xlabel()`). Repurposing the `with` statement to implicitly bind Loh's leading-dot receiver context `.` to the target object inside the block eliminates this repetition.

### Proposed Syntax
Using `with obj:` to run a block where any leading-dot reference resolves against `obj`:
```python
with matplotlib.pyplot:
    .plot(x, y)
    .title("Sales Over Time")
    .xlabel("Month")
    .show()
```

### Compile-Time Desugaring
The parser creates a temporary variable referencing the target and prefix-rewrites any leading-dot attribute/method nodes inside the block:
```python
_receiver = matplotlib.pyplot
_receiver.plot(x, y)
_receiver.title("Sales Over Time")
_receiver.xlabel("Month")
_receiver.show()
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

## 27. Partial Function Application (`func(args)...`)

### Motivation
Standard Python requires `functools.partial` or lambda wrapping to pre-bind arguments to a callable. Appending the ellipsis `...` to a call expression provides a highly readable, native syntax to create partially applied function thunks at compile time using standard library mechanisms.

### Proposed Syntax
```python
# Create a partial function pre-binding the first argument
add_ten = add(10)...

# Pre-bind keyword arguments
configure_local = configure(host="localhost")...
```

### Compile-Time Desugaring
At parse-time, any call expression followed by the ellipsis operator `...` is wrapped in a call to `functools.partial`:
```python
import functools

add_ten = functools.partial(add, 10)
configure_local = functools.partial(configure, host="localhost")
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

## 44. Pipe Unpacking Operators (`|*>`, `|**>`)

### Motivation
Standard pipes (`|>`) feed a single value as the first argument to a function call. However, when a pipeline needs to pass multiple arguments (unpacked from an iterable or sequence) or keyword arguments (unpacked from a dictionary), developers must wrap the function call in a lambda expression (e.g. `args |> lambda x: func(*x)`). Introducing dedicated unpacking pipes—`|*>` for positional argument unpacking and `|**>` for keyword argument unpacking—makes pipeline argument forwarding extremely direct and expressive.

### Proposed Syntax
```python
# 1. Positional unpack pipe (|*>)
(10, 20) |*> math.pow  # Equivalent to: math.pow(10, 20)

# 2. Keyword unpack pipe (|**>)
{"sep": ", ", "end": "\n"} |**> print("A", "B", ?)
```

### Compile-Time Desugaring
At parse-time, the unpacking pipes desugar into function calls with standard Python unpacking operators (`*` and `**`):
```python
# (10, 20) |*> math.pow translates to:
math.pow(*(10, 20))

# kwargs |**> func translates to:
func(**kwargs)
```




