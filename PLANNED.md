# Loh Planned Features

This document tracks features that have been approved for implementation in **Loh**.


## 1. Implicit Return from Final Function Expression (`from __loh__ import implicit_returns`)


### Motivation
Writing explicit return statements `->` in short function bodies is still slightly verbose. In expression-oriented languages (like Rust, Ruby, and Scala), the final expression evaluated in a block/function is automatically returned, making method and helper declarations extremely concise.

To prevent breaking compatibility with standard Python code (where side-effect functions ending with expression statements are expected to return `None`), this feature is opt-in per module via a future-style import.

### Proposed Syntax
Using the `implicit_returns` future import from the `__loh__` module:
```python
from __loh__ import implicit_returns

# Automatically returns base_price * 1.05
calculate_total(base_price):
    tax_rate = 0.05
    base_price * (1 + tax_rate)
```

### Compile-Time Desugaring
If the `implicit_returns` feature is active for the module, the compiler detects when the final statement of a function body is an `Expr` node (and not a control flow keyword or a docstring) and wraps it in a `Return` AST node:
```python
from __loh__ import implicit_returns

def calculate_total(base_price):
    tax_rate = 0.05
    return base_price * (1 + tax_rate)
```
