# Loh Planned Features

This document tracks features that have been approved for implementation in **Loh**.

---

## 1. Inline Scope Initializer Block (`obj { .method(); .prop = val }`)

### Motivation
Constructing, configuring, and updating objects in standard Python requires multiple statement-level assignments, making inline initialization (such as inside list comprehensions or function calls) impossible without bloating constructors. 

Adding an inline `{}` scope block following any object expression (such as a constructor call, an identifier, or a function return) allows developers to configure or modify the object inline using Loh's established leading-dot `.` receiver context. The entire expression evaluates to the receiver object.

### Proposed Syntax
The block uses the leading dot `.` prefix to assign properties and invoke methods on the object. It can be applied directly to a constructor call or an existing variable:
```python
# Initialization on construction
manager = RestaurantManager("The Loh Bistro") {
    .add_menu_item(101, "Truffle Fries", 12.50, "appetizer")
    .active = ++
}

# Configuration on an existing reference
configured_user = user {
    .name = "Alice",
    .update_status("active")
}
```

### Compile-Time Desugaring
The parser compiles this block by evaluating the receiver expression, executing the dot-prefixed assignments and calls, and returning the receiver instance:
```python
# For: configured_user = user { .name = "Alice", .update_status("active") }
def _init_block(_inst):
    _inst.name = "Alice"
    _inst.update_status("active")
    return _inst

configured_user = _init_block(user)
```
Because standard Python syntax does not allow curly braces immediately following a primary expression (variables, constructor calls), this syntax has zero grammatical conflicts in the PEG parser.

