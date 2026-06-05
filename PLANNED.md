# Loh Planned Features

This document tracks features that have been approved for implementation in **Loh**.

## 1. Multi-Key Subscript Slicing for Dictionaries (`dict[key1, key2]`)

### Motivation
Often developers have a dictionary and want to extract a subset of its keys (e.g., forwarding config keys to a function). In standard Python, this requires a dict comprehension `{k: d[k] for k in ('a', 'b') if k in d}`, which is verbose and repetitive. Using multiple keys in the subscript bracket provides a clean slicing/subsetting syntax.

### Proposed Syntax
Passing a tuple of keys to a dictionary subscript returns a sub-dictionary containing only those keys:
```python
# Extracts only 'host' and 'port' from config
connection_args = config['host', 'port']
```

### Compile-Time Desugaring
Subscript slices containing multiple keys on dict targets desugar to dict comprehensions:
```python
connection_args = {k: config[k] for k in ('host', 'port') if k in config}
```

---

## 2. Implicit Return from Final Function Expression (`from __loh__ import implicit_returns`)

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
