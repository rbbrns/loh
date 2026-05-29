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
Writing custom class constructors in standard Python requires declaring `def __init__(self, owner, balance):` and manually assigning each parameter to its corresponding instance attribute: `self.owner = owner`, etc. This generates significant boilerplate. We want a clean, unified shorthand that omits the constructor name, and a parameter properties feature that automatically binds parameters prefixed with a dot `.` as attributes.

### Proposed Syntax
Using `.(...)` defines the class constructor. Parameters starting with a dot `.` (such as `.owner` or `.balance`) automatically become instance attributes.

In addition, dot-prefixed parameter properties (`.param`) are supported on **both** the constructor and **any other class method**.

```python
Account::
    # Constructor shorthand with parameter properties
    .(.owner, .balance, email):
        .email_domain = email.split('@')[1]

    # Standard class method with parameter properties
    .update_status(.status, details):
        .log(details)
```

### Compile-Time Desugaring
At parse/compile-time, the constructor maps directly to `__init__`, and dot-prefixed parameter properties map to corresponding attribute assignments prepended to the start of the method body:

```python
class Account:
    def __init__(self, owner, balance, email):
        self.owner = owner
        self.balance = balance
        self.email_domain = email.split('@')[1]

    def update_status(self, status, details):
        self.status = status
        self.log(details)
```

---

## 3. Unified Exception Handling Block Syntax (`^:`, `^?`, `^?*`, `^??:`, `^*:`)

### Motivation
Exceptions and error handling in Loh are centered around the caret `^` symbol (`^^^` raise, `^?!` assert). The current try-except syntax (`~^:` for try, `?^` for except, etc.) is slightly verbose and lacks visual consistency. Grouping all exception handling clauses under the unified `^` prefix creates a cohesive, highly readable block structure.

### Proposed Syntax
All clauses in an exception handling block are prefixed with `^`:
* `^:` — try
* `^? Error => e:` — except
* `^?* Error => e:` — except*
* `^??:` — else (try-else)
* `^*:` — finally

```python
^:
    result = 10 / 0
^? ZeroDivisionError => e:
    print(f"Error: {e}")
^??:
    print("Success")
^*:
    print("Cleanup")
```

### Compile-Time Desugaring
Directly translates to standard Python `try-except-else-finally` blocks:
```python
try:
    result = 10 / 0
except ZeroDivisionError as e:
    print(f"Error: {e}")
else:
    print("Success")
finally:
    print("Cleanup")
```

---

## 4. Inline Scope Initializer Block (`obj() { .method(); .prop = val }`)

### Motivation
Constructing and configuring objects in standard Python requires multiple statement-level assignments, making inline initialization (such as inside list comprehensions or function calls) impossible without bloating constructors. 

Adding an inline `{}` scope block allows developers to configure an object immediately upon instantiation, executing method calls and attribute assignments using Loh's established leading-dot `.` receiver context. The block returns the initialized object.

### Proposed Syntax
The block uses the leading dot `.` prefix to assign properties and invoke methods on the constructed object:
```python
manager = RestaurantManager("The Loh Bistro") {
    # Method invocation on receiver
    .add_menu_item(101, "Truffle Fries", 12.50, "appetizer")
    .add_menu_item(102, "Loh Burger", 18.99, "main")
    
    # Attribute setting on receiver
    .active = ++
}
```

### Compile-Time Desugaring
The parser compiles this block by creating a temporary builder scope that instantiates the receiver, executes the dot-prefixed assignments and calls, and returns the receiver instance:
```python
def _init_RestaurantManager():
    _inst = RestaurantManager("The Loh Bistro")
    _inst.add_menu_item(101, "Truffle Fries", 12.50, "appetizer")
    _inst.add_menu_item(102, "Loh Burger", 18.99, "main")
    _inst.active = True
    return _inst

manager = _init_RestaurantManager()
```
Because standard Python syntax does not allow curly braces immediately following a function/constructor call, this syntax has zero grammatical conflicts.

