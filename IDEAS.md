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

## 11. Automatic f-Strings (Implicit Interpolation)

### Motivation
Python requires prefixing string literals with `f` (e.g., `f"Hello {name}"`) for interpolation. Forgetting the `f` prefix is a very common bug.

### Proposed Syntax
Any double-quoted or single-quoted string containing unescaped `{expression}` brackets is automatically compiled as an f-string without requiring the prefix:
```python
name = "Alice"
msg = "Hello {name}!"  # Auto f-string
```

### Compile-Time Desugaring
The parser checks string token content; if it contains braces `{}` and is not a raw string (`r""`), it tokenizes and parses it as a `JoinedStr` (f-string) AST node.

---

## 12. None-Safe Attribute Assignment (`obj~.prop = value`)

### Motivation
Safe navigation `user~.profile~.address` protects against attribute reads crashing on `None` values. However, trying to assign to a nested property where a parent might be `None` still results in a traceback. 

Applying safe navigation to assignment allows writing values safely to nested properties, silently short-circuiting and doing nothing if any parent object in the chain is `None`.

### Proposed Syntax
```python
# Silently does nothing if user or profile is None
user~.profile~.address = new_address
```

### Compile-Time Desugaring
Desugars into nested conditional statement blocks checking each segment:
```python
_val1 = user
if _val1 is not None:
    _val2 = _val1.profile
    if _val2 is not None:
        _val2.address = new_address
```

---

## 13. Infinite Loops Shorthand (`$:`)

### Motivation
Python lacks a dedicated infinite loop construct, requiring `while True:`. Loh can use the loop sigil alone to represent an infinite loop.

### Proposed Syntax
```python
$:
    # Loop runs forever until broken
    print("Processing...")
    ? should_stop:
        $>>
```

### Compile-Time Desugaring
Translates directly to `while True:`.

---

## 14. Parenthesized Expression Blocks (`(expr; expr; expr)`)

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

## 15. Lazy Parameter Evaluation & Memoized Thunks (`lazy`)

### Motivation
Eager argument evaluation in Python can waste resources when passing expensive computations to functions that might ignore them (e.g., debug loggers or conditional paths). Standard Python requires passing lambdas (`lambda: expr`), which adds syntax boilerplate to both the call site (`lambda:`) and the function body (`param()`).

We want a unified syntax that allows:
1. **Definition-Site Lazy Parameters**: Declaring a method parameter as `lazy` so that the function body can treat it as a normal variable, with the compiler automatically handling evaluation.
2. **Call-Site Lazy Thunks**: Explicitly passing a deferred/memoized expression to matching parameters using the `lazy` keyword.

### Proposed Syntax

#### **1. Definition Site**
Prefixing a parameter with the `lazy` keyword inside a function/method signature:
```python
log_debug(lazy message: str):
    ? debug_enabled:
        # Accessed naturally like a string (no message() calling needed)
        print(message)
```

#### **2. Call Site**
Passing a lazy thunk to the function:
```python
log_debug(lazy generate_heavy_report())
```

### Compile-Time Desugaring

#### **Call Site desugaring**
The parser compiles `lazy expr` into a built-in lazy proxy constructor:
```python
log_debug(_LohLazy(lambda: generate_heavy_report()))
```

#### **Definition Site desugaring**
Inside the function body, the compiler registers the parameter as a lazy variable and automatically wraps all read references to force evaluation:
```python
def log_debug(message):
    if debug_enabled:
        # References to the lazy parameter are auto-desugared to force evaluation
        print(message._force() if hasattr(message, "_force") else message)
```

### Runtime Memoization Proxy
At runtime, `_LohLazy` wraps the deferred lambda, forcing evaluation exactly once upon the first access and caching the value:
```python
class _LohLazy:
    def __init__(self, func):
        self._func = func
        self._evaluated = False
        self._value = None

    def _force(self):
        if not self._evaluated:
            self._value = self._func()
            self._evaluated = True
        return self._value

    # Proxy magic methods to dynamically force evaluation when accessed transparently
    def __getattr__(self, name):
        return getattr(self._force(), name)

    def __str__(self):
        return str(self._force())

    def __bool__(self):
        return bool(self._force())
```


