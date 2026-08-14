# Loh Planned Features

This document tracks features that have been approved for implementation in **Loh**.

---

## 1. Parameter Pattern Destructuring, Multi-Clause Functions, and Signature Guards

### Motivation
Standard Python requires manually unpacking dictionaries or objects inside function bodies (`name = user['name']`) and wrapping polymorphic handlers or recursive base-cases in nested `match` statements.

This feature elevates pattern matching and guard validation directly to function parameter signatures.

### Proposed Syntax

#### 1. Parameter Pattern Destructuring & Target Binding (`param => pattern` / `pattern`)
Parameter slots in function declarations can contain structural patterns. If `param => pattern` is used, `param` captures the raw argument object while sub-properties are extracted:

```python
# Unpacks dictionary keys directly into local scope
process_user({"name": name, "role": role}):
    print(f"User {name} has role {role}")

# Unpacks raw target 'user' AND extracts sub-properties
process_user(user => {"name": name, "role": role}):
    print(f"User {name} (Raw obj: {user})")

# Unpacks tuple coordinates
distance((x1, y1), (x2, y2)):
    -> sqrt((x2 - x1)**2 + (y2 - y1)**2)
```

#### 2. Multi-Clause Pattern Matching Functions
Consecutive function definitions sharing the exact same identifier are automatically merged into a single multi-clause function that evaluates argument patterns top-to-bottom:

```python
# Multi-clause event handlers
handle_event({"type": "click", "pos": (x, y)}):
    print(f"Clicked at {x}, {y}")

handle_event({"type": "scroll", "delta": d}):
    print(f"Scrolled {d}")

handle_event(_):
    print("Unknown event")

# Mathematical recursion
factorial(0): -> 1
factorial(n): -> n * factorial(n - 1)
```

#### 3. Parameter Signature Guards (`param ? guard`)
Constrain parameters directly in the signature using `? guard` conditions:

```python
# Validates numerical boundaries directly in parameter signature
calculate_log(n ? n > 0):
    -> math.log(n)

calculate_log(_):
    ^^^ ValueError("Logarithm undefined for non-positive numbers")
```

### Execution & Fallthrough Semantics
- **Multi-Clause Functions**: When invoked, arguments are matched against clauses top-to-bottom. If a pattern or guard fails, execution falls through to the next clause. If all clauses fail, a `TypeError` is raised.
- **Single-Clause Functions**: If a single-clause function receives an argument that fails its parameter pattern or signature guard, it immediately raises a `TypeError`.

### Compile-Time Desugaring
Consecutive functions sharing an identifier are merged into a single AST function node containing an internal `match` dispatcher on entry.

---

## 2. Multi-Dispatch Function Overloads (`func(...)+:`) & Parameter Pattern Matching (`(=> pattern)`)

### Motivation
Standard Python lacks built-in syntax for multiple dispatch and function overloading across argument types, pattern stencils, and guard contracts.

This feature introduces `func(...)+:` for declaring explicit multi-dispatch overload clauses, while enforcing Loh's universal rule: **pattern matching ALWAYS requires `=>`**.

### Proposed Syntax

#### 1. Multi-Dispatch Signature Extension (`func(...)+:`)
Using `+:` on a function signature explicitly appends the clause to `func`'s multi-dispatch table:

```python
# Base function definition (fallback handler)
process(data):
    ^^^ TypeError(f"Unsupported data: {data}")

# Overload 1: Type narrowing (no pattern, so no =>)
process(n: int)+:
    print(f"Processing integer: {n}")

# Overload 2: Pattern matching (requires =>)
process(=> {"status": 200, "user": u})+:
    print(f"Processing user: {u}")

# Overload 3: Target binding AND pattern matching (requires =>)
process(data => {"status": 200, "data": payload})+:
    print(f"Payload {payload} in raw data {data}")
```

#### 2. Multi-Parameter Pattern Matching Overloads
When multiple parameters perform pattern matching, `=>` is required in each pattern parameter slot:

```python
# Base function
collide(a, b):
    -> 0

# Pattern matching overloads (requires => on pattern slots)
collide(=> Circle(r1), => Circle(r2))+:
    -> r1 + r2

collide(=> Circle(r), => Rectangle(w, h))+:
    -> r + w + h
```

#### 3. Parameter Component Ordering (Types, Patterns, Guards, Defaults)
Parameter slots combine components in a strict, natural left-to-right order:

$$\text{\texttt{param : Type => pattern ? guard = default}}$$

```python
# 1. Type Hint + Pattern Match
fetch_profile(res: dict => {"status": 200, "user": u}):
    print(u)

# 2. Pattern Match + Default Value (passes default into pattern if caller omits argument)
connect(=> {"host": h, "port": p} = {"host": "localhost", "port": 8080}):
    print(f"Connecting to {h}:{p}")

# 3. All Components Combined (Type hint, pattern, guard, default)
configure(cfg: dict => {"retries": r, "timeout": t} ? t > 0 = {"retries": 3, "timeout": 30}):
    print(f"Configured retries={r}, timeout={t}")
```

### Execution & Fallthrough Semantics
- When invoked, argument signatures evaluate components in order:
  1. **Default Evaluation**: If caller omitted argument, use `default`.
  2. **Type Check**: Verifies `: Type` hint (fails if incorrect type).
  3. **Pattern Check**: Matches `=> pattern` stencil and extracts sub-variables into scope.
  4. **Guard Evaluation**: Verifies `? guard` condition.
- **Overload Fallthrough**: In multi-dispatch functions (`func(...)+:`), if a signature's type check, pattern match (`=>`), or guard (`?`) fails, execution falls through to the next overload clause.
- **Base Fallback**: If all overload clauses fail, the base function definition `func(...):` executes (or raises `TypeError` if no base fallback exists).

### Compile-Time Desugaring
`func(...)+:` registers the clause signature and body into `func`'s AST multi-dispatch table.





