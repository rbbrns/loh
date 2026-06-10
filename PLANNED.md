# Loh Planned Features

This document tracks features that have been approved for implementation in **Loh**.

## 1. Function Aliasing (`foo | bar(x, b):`)

Allows defining a function (or method) with one or more aliases:
```python
foo | bar(x, b):
    -> x + b
```
Inside classes, both symmetric and asymmetric styles are supported for class methods, while keeping mismatched mixtures (e.g. `foo | .bar`) invalid:
```python
MyClass::
    .foo | .bar(x, b):
        ...
```
This desugars to:
```python
def foo(x, b):
    -> x + b
bar = foo
```
