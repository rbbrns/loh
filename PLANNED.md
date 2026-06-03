# Loh Planned Features

This document tracks features that have been approved for implementation in **Loh**.

## 1. None-Safe Attribute Assignment (`obj~.prop = value`)

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

## 2. Infinite Loops Shorthand (`$:`)

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
