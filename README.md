# **Loh: A Superset of Python**

Welcome to **Loh** is superset of the Python syntax focused on making Python more concise, intuitive, and enjoyable to read and write. Loh provides symbol-based alternatives for all Python keywords and introduces extra syntactic sugar to reduce boilerplate and improve clarity.

Since Loh is a superset, **all existing Python code is valid Loh code**.

## Getting Started
```
./configure --prefix=$INSTALL_DIR
make regen-all
make install
ln -s /usr/local/bin/loh $INSTALL_DIR/bin/python3 
```

## Keyword Mapping

Loh provides symbolic aliases for Python's keywords. 

| **Python** |    **Loh**      |
| ---------- | --------------- |
| True       | ++              |
| False      | --              |
| None       | \~  *or Implict*|
| and        | &&              |
| or         | \|\|            |
| not        | \~\~ or !!!     |
| is         | ===             |
| in         | <\~             |
| if         | ?               |
| else       | ?!              |
| elif       | ?!?             |
| for        | $               |
| while      | $?              |
| break      | $>>             |
| continue   | $<<             |
| try        | \~^             |
| except     | ?^              |
| finally    | ?\*             |
| raise      | ^^^             |
| assert     | ^?!             |
| with       | &               |
| as         | =>              |
| import     | <.              |
| from       | .>              |
| del        | =>              |
| return     | ->              |
| yield      | \~>             |
| async      | %               |
| await      | %               |
| lambda     | (params)->...   |
| pass       | ...             |
| class      | Name::          |
| def        | (params):       |
| type       | :               |
| match      | ? ... ==:       |
| case       | *pattern*:      |


### Logic

```python
# Python
if (x is True and y is not False) or z is None:
    print("Logic!")
# Loh
? (x === ++ && y === !!! --) || z ===:
    print("Logic!")
```

### Control Flow

```python
# Python
if score > 90:\
    grade = 'A'
elif score > 80:
    grade = 'B'
else:
    grade = 'C'
# Loh
? score > 90:
    grade = 'A'
?!? score > 80:
    grade = 'B'
?!:
    grade = 'C'
```


### Exception Handling

```python
# Python
try:
    result = 10 / 0
except ZeroDivisionError as e:
    print("Error!")
finally:\
    print("Done.")
# Loh
~^:
    result = 10 / 0
?^ ZeroDivisionError => e:
    print("Error!")
?*:
    print("Done.")
```

### Imports

```python
# Python
import math
from math import sqrt
# Loh
<. math
.> math <. sqrt
```

### Match

```python
# Python
match status:
    case 400:
        return "Bad request"
    case 404:
        return "Not found"
    case _:
        return "Something else"
# Loh
?== status:
    400:
        -> "Bad request"
    404:
        -> "Not found"
    _:
        -> "Something else"
```

### Implicit None

```python
# Python
def foo(a=None, b=None):
  ...
    
# Loh
foo(a=, b=):
    ...

# Python
x = None
# Loh
x =

# Python
none_tuple = (None, None)
none_list = [None, None]
none_set = {None, None}
none_dict = {'a': None, 'b': None}
# Loh
none_tuple = (,,)
none_list = [,,]
none_set = {,,}
none_dict = {'a':, 'b':}

```


### Function Definition **
```python
# Python
def my_func(a, b):
    pass
def my_func_with_return(a, b) -> int:
    return a + b
# Loh
my_func(a, b):
    ...
my_func_with_return(a, b) -> int:
    -> a + b
```

### Class Definition

```python
# Python
class MyClass(Base):
    def __init__(self, value):
        self.value = value
    def get_value(self):
        return self.value
# Loh
MyClass:Base:
  .__init__(value):
     .value = value
  .get_value():
     -> .value
```

### Type Alias 

```python
# Python
type IntOrFloat = int | float
# Loh
: IntOrFloat = int | float
```

### Lambdas
```python
# Python
map(lambda x: x * 2, [1, 2, 3])
# Loh
map((x) -> x * 2, [1, 2, 3])
```

### Loops

```python
# Python
total = 0
for i in range(10):
    total += i
# Loh
total = 0
$ i := range(10):
    total += i

# Python
evens = [i for i in range(10) if i % 2 == 0]
# Loh
evens = [i $ i ~> range(10) ? i % 2 == 0]
```