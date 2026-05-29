# Loh Planned Features

This document tracks features that have been approved for implementation in **Loh**.

---

## 1. Range/Slice Literals (`..`)

### Motivation
Standard Python relies on `range(start, stop)` for iteration and sequences. In mathematical notations and languages like Rust or Ruby, range/interval literals (`1..10`) are used to represent sequences cleanly. Adding this to Loh makes loop variables and slice checks extremely compact.

### Proposed Syntax
```python
# Loop 0 to 9
$ i <~ 0..10:
    print(i)

# Check if value in range
? x <~ 1..100:
    print("In bounds")
```

### Compile-Time Desugaring
Translates directly to `range()` constructor calls:
```python
range(0, 10)
x in range(1, 100)
```

---

## 2. Constructor Shorthand (`.()`) and Parameter Properties (`.param`)

### Motivation
Writing custom class constructors in standard Python requires declaring `def __init__(self, owner, balance):` and manually assigning each parameter to its corresponding instance attribute: `self.owner = owner`, etc. This generates significant boilerplate. We want a clean, unified shorthand that omits the constructor name and automatically binds parameters prefixing with a dot `.` as attributes.

### Proposed Syntax
Using `.(...)` defines the class constructor. Parameters starting with a dot `.` (such as `.owner` or `.balance`) automatically become instance attributes:

```python
Account::
    .(.owner, .balance, email):
        # owner and balance are automatically assigned to self.owner and self.balance
        # email is a normal constructor parameter
        self.email_domain = email.split('@')[1]
```

### Compile-Time Desugaring
At parse/compile-time, this maps directly to a standard Python `__init__` constructor with parameter assignments prepended to the body:

```python
class Account:
    def __init__(self, owner, balance, email):
        self.owner = owner
        self.balance = balance
        self.email_domain = email.split('@')[1]
```
