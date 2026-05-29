# Loh Planned Features

This document tracks features that have been approved for implementation in **Loh**.

---

## 1. Inline Scope Initializer Block (`obj() { .method(); .prop = val }`)

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
