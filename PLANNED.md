# Loh Planned Features

This document tracks features that have been approved for implementation in **Loh**.

## 1. Partial Function Application (`foo...(_)` / `foo...(arg)`)

Allows pre-binding arguments of a function call to create a new, partially-applied function. It is written using the postfix ellipsis `...` operator on a callable call:

### Proposed Syntax
- **Explicit Placeholders**: Using `_` as a placeholder to specify unbound/hole arguments.
  ```python
  divide_by_two = divide...(_, 2)
  # Desugars to: (x) -> divide(x, 2)
  
  calc_ten = calc...(_, 10, _)
  # Desugars to: (x, y) -> calc(x, 10, y)
  ```
- **Implicit Left-to-Right Binding**: If no placeholders are used, arguments are bound from left to right, and any remaining arguments are forwarded:
  ```python
  add_five = add...(5)
  # Desugars to: (*args, **kwargs) -> add(5, *args, **kwargs)
  
  greet_alice = greet...(name="Alice")
  # Desugars to: (*args, **kwargs) -> greet(*args, name="Alice", **kwargs)
  ```

