# Loh Planned Features

This document tracks features that have been approved for implementation in **Loh**.

---

## 1. Self-Referential Dictionary Literals (`.{}`)

### Motivation
In Loh, `.` is used inside class methods and constructors to represent `self` (`.balance`). To allow dictionary literals to access sibling keys using `.` without creating scope ambiguity with enclosing class instance attributes, Loh introduces explicit self-binding dictionary literals `.{}`. Standard `{}` literals remain 100% standard Python dictionary literals.

### Proposed Syntax
```python
# Standalone self-referential dictionary
server = .{
    host: "localhost",
    port: 8080,
    url: f"http://{.host}:{.port}"  # '.' accesses server['host'] and server['port']
}

# Inside a class method
Account::
    .discount = 0.10  # Enclosing class instance attribute

    .get_invoice():
        -> .{
            subtotal: 100,
            # '.subtotal' is the dict key; 'self.discount' (or '.discount' via fallback) is outer class
            total: .subtotal * (1 - self.discount)
        }
```

### Compile-Time Desugaring
At parse-time, a `.{}` dictionary literal is compiled into an immediately-invoked helper function (IIFE) or sequential dictionary builder where keys defined earlier in the dictionary are tracked and accessible to subsequent values:
```python
def _build_dict_1():
    _d = {}
    _d["host"] = "localhost"
    _d["port"] = 8080
    _d["url"] = f'http://{_d["host"]}:{_d["port"]}'
    return _d

server = _build_dict_1()
```

---

## 2. Dict & Object Extension Key Separators (`|:`, `+:`, `?:`, `-:`)

### Motivation
When merging dictionaries or constructing extended configurations (`base | { ... }` or `{ **base, ... }`), standard key-value assignments (`key: val`) replace parent keys completely. Adding modifier sigils to the dictionary key-value separator `:` enables fine-grained control over list appending, deep dictionary merging, default fallbacks, and key deletion.

### Proposed Syntax

| Key Separator | Name | Behavior / Semantics | Example |
| :--- | :--- | :--- | :--- |
| **`key: val`** | **Overwrite** | Replaces parent value completely (Standard Python). | `port: 8080` |
| **`key \|: val`** | **Union / Merge** | Merges dictionaries (`d1 \| d2`), unions sets (`s1 \| s2`), or deduplicates. | `headers \|: {"Auth": "Bearer"}` |
| **`key +: val`** | **Append / Concat** | Appends to lists (`l1 + l2`), concatenates strings, or adds numbers. | `tags +: ["v2"]` |
| **`key ?: val`** | **Fallback Default** | Sets `key` to `val` **if and only if** `key` is missing or `None` in base. | `timeout ?: 30` |
| **`key -: ~`** | **Omit / Remove** | Removes `key` entirely from the resulting dictionary. | `deprecated -: ~` |

```python
base_config = .{
    host: "localhost",
    port: 8000,
    tags: ["v1"],
    headers: {"Accept": "application/json"},
    timeout: 30
}

# Extending base_config:
prod_config = base_config | .{
    port: 8443,                              # Overwrite
    tags +: ["v2"],                          # Append -> ["v1", "v2"]
    headers |: {"Authorization": "Bearer"},  # Dict union merge
    timeout ?: 60,                           # Keeps 30 since it exists
    old_setting -: ~                         # Removes key from output
}
```

### Compile-Time Desugaring
In [python.gram](file:///Users/robbarnes/Development/loh/Grammar/python.gram), the key-value pair rule `kvpair` is expanded to recognize `|:`, `+:`, `?:`, and `-:`. The parser translates extension key-value pairs into calls to a C runtime helper `_loh_object_extend(base, key, val, mode)`:
```python
# Desugars prod_config dict extension to:
_temp = base_config.copy()
_loh_extend_field(_temp, "port", 8443, MODE_OVERWRITE)
_loh_extend_field(_temp, "tags", ["v2"], MODE_APPEND)
_loh_extend_field(_temp, "headers", {"Authorization": "Bearer"}, MODE_UNION)
_loh_extend_field(_temp, "timeout", 60, MODE_FALLBACK)
_loh_extend_field(_temp, "old_setting", None, MODE_REMOVE)
prod_config = _temp
```

---

## 3. Deep-Copy Container Unpacking (`[***x]`, `{***x}`, `{***z}`)

### Motivation
Python supports 1-level shallow unpacking inside container literals using `[*y]` (lists), `{**x}` (dicts), and `{*z}` (sets). To provide a conflict-free, complementary syntax for deep copies without colliding with function argument unpacking (`func(*args)` or `func(**kwargs)`), Loh introduces `***` (Triple-Star) as a recursive deep-unpacker inside container literals.

### Proposed Syntax

| Unpacking Type | List Literal | Dict Literal | Set Literal |
| :--- | :--- | :--- | :--- |
| **Shallow Copy (1-Level)** | `[*y]` | `{**x}` | `{*z}` |
| **Deep Copy (Recursive)** | **`[***y]`** | **`{***x}`** | **`{***z}`** |

```python
original_dict = {"items": [1, 2, 3], "meta": {"owner": "Alice"}}

# Deep-copies original_dict into a new dictionary literal:
deep_dict = {***original_dict}

# Deep-copies a nested list inside an object extension:
new_config = base_config | .{
    backup_items: [***original_dict["items"]]
}
```

### Function Call Disambiguation
Inside container brackets (`[...]` / `{...}`), the surrounding literal unambiguously signals container construction to the parser:
* `func(*x)` $\rightarrow$ Unpacks `x` into positional function arguments.
* `func(**x)` $\rightarrow$ Unpacks `x` into keyword function arguments.
* `func([***x])` $\rightarrow$ Deep-copies `x` into a new list and passes a single argument.
* `func({***x})` $\rightarrow$ Deep-copies `x` into a new dict and passes a single argument.

### Compile-Time Desugaring
At parse-time, the PEG parser compiles `***x` inside a container literal by wrapping `x` in a call to `copy.deepcopy()` before sequence or mapping construction:
```python
# {***x} compiles to:
{**copy.deepcopy(x)}

# [***y] compiles to:
[*copy.deepcopy(y)]
```

---

## 4. Implicit Dictionary Unpacking inside Dict Literals (`{}` / `.{}`)

### Motivation
In standard Python, placing a dictionary expression or dict comprehension (`{ k: v for ... }`) directly inside another dictionary literal without a colon or `**` (e.g. `{ "a": 1, {"b": 2} }`) is a syntax/type error. In Loh, writing a bare dictionary or dict comprehension as a standalone entry inside a dictionary literal (`{}` or `.{}`) automatically unpacks (`**`) its key-value pairs into the enclosing dictionary. This enables inline dictionary comprehensions alongside static keys without `**{...}` wrapping boilerplate or special key bracket hacks.

### Proposed Syntax
```python
app_config = .{
    env: "prod",
    port: 8080,

    # Bare dict comprehension inside .{} -> AUTOMATICALLY UNPACKED!
    { f"node_{i}": f"http://10.0.0.{i}" $ i <~ 1..4 },

    # Combined with field extension
    tags +: ["cluster_a"]
}
```

### Evaluates To
```python
{
    "env": "prod",
    "port": 8080,
    "node_1": "http://10.0.0.1",
    "node_2": "http://10.0.0.2",
    "node_3": "http://10.0.0.3",
    "tags": ["cluster_a"]
}
```

### Zero Set Ambiguity
Python distinguishes sets from dicts by the presence of colons (`:`). Because dictionaries are unhashable and can never be stored inside sets (`{ {"a": 1} }` raises `TypeError`), placing a bare dictionary expression `{ k: v ... }` inside an outer dictionary `{}` or `.{}` is 100% grammatically unambiguous: it can only mean unpacking its key-value pairs (`**`) into the outer dictionary.

### Compile-Time Desugaring
At parse-time, if an entry in a dict literal grammar rule `dict` or `self_dict` is an un-keyed dictionary or dict comprehension expression, the parser automatically wraps it in dictionary unpacking (`_PyAST_starred` / `dict_unpacking` node):
```python
# Un-keyed dict comprehension inside dict literal:
{ f"node_{i}": f"http://10.0.0.{i}" $ i <~ 1..4 }

# Desugars at parse-time to:
**{ f"node_{i}": f"http://10.0.0.{i}" for i in range(1, 4) }
```

---

## 5. Set Relational Comparisons with General Iterables (`set <= iterable`)

### Motivation
In standard Python, comparing a `set` with a non-set iterable (like a `list`, `dict`, or `tuple`) using relational operators (`<`, `<=`, `>`, `>=`) raises a `TypeError`. In Loh, we extend set comparison operators to dynamically cast the non-set operand to a temporary set at runtime. This allows checking if all elements of a set are contained within any iterable sequence, mapping, or collection.

Specifically, it enables check patterns like `{x} <= a_list` or `{k} <= a_dict` to check single-element containment, as well as `{a, b} <= a_list` to cleanly assert multi-element containment without writing verbose `a in a_list and b in a_list` checks.

### Proposed Syntax
```python
# Check if key/element is inside list/dict/set
? {x} <= a_list:
    ...
? {k} <= a_dict:
    ...

# Check if multiple elements are inside a list
? {a, b} <= a_list:
    ...
```

### Runtime Implementation
Modify the C-level set rich comparison function `set_richcompare` in `Objects/setobject.c` to support non-set operands for inequality operations (`<`, `<=`, `>`, `>=`). If the right-hand operand `w` is not a set, it is temporarily converted to a set using `PySet_New(w)` for the duration of the comparison:

```c
static PyObject *
set_richcompare(PyObject *self, PyObject *w, int op)
{
    PySetObject *v = _PySet_CAST(self);
    PyObject *temp_set = NULL;
    PyObject *result = NULL;

    if (!PyAnySet_Check(w)) {
        // We only convert for relational inequality operators to avoid
        // making sets compare equal (==) to other types of collections.
        if (op == Py_EQ || op == Py_NE) {
            Py_RETURN_NOTIMPLEMENTED;
        }
        temp_set = PySet_New(w);
        if (temp_set == NULL) {
            PyErr_Clear();
            Py_RETURN_NOTIMPLEMENTED;
        }
        w = temp_set;
    }
    
    // ... Perform standard set comparison logic ...

    Py_XDECREF(temp_set);
    return result;
}
```


