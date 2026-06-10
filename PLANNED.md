# Loh Planned Features

This document tracks features that have been approved for implementation in **Loh**.

## 1. Multiple Dispatch using Parameter-Level Match Query (`param ? pattern`)

Allows matching and destructuring parameters directly in the function signature, and merging adjacent function definitions with the same name into a single multiple-dispatch function.

### Proposed Syntax
- **Parameter Destructuring / Validation**:
  ```python
  process_user(u ? User(name, age)):
      print(name, age)
  ```
- **Multiple Dispatch**:
  ```python
  collide(c1 ? Circle(r1), c2 ? Circle(r2)):
      -> r1 + r2

  collide(c1 ? Circle(r), c2 ? Rectangle(w, h)):
      -> r + w + h
  ```

### Compile-Time Desugaring
- For a single function with pattern parameters, checks are inserted sequentially at the beginning of the function:
  ```python
  def process_user(u):
      match u:
          case User(name, age): pass
          case _: raise TypeError()
      print(name, age)
  ```
- For adjacent functions with the same name, they are merged into a single function using a `match` on the arguments tuple:
  ```python
  def collide(c1, c2):
      match (c1, c2):
          case (Circle(r1), Circle(r2)):
              return r1 + r2
          case (Circle(r), Rectangle(w, h)):
              return r + w + h
          case _:
              raise TypeError("No matching signature found")
  ```

