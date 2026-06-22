# Loh Planned Features

This document tracks features that have been approved for implementation in **Loh**.

## 1. Lazy Evaluation & Late-Bound Expressions (`` `expr` ``)

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


