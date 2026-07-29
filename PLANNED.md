# Loh Planned Features

This document tracks features that have been approved for implementation in **Loh**.

---

## 1. Support Explicit and Implicit Walrus Loop Iteration (`:=`) in Comprehensions


### Motivation
Loh already supports using the walrus/binding operator `:=` in statement-level `for` loops (e.g., `$ x := items:`). To make loop syntax fully consistent across the language and allow deprecating the `<~` operator, we will extend comprehensions to support `:=` for both explicit and implicit loop iterations.

### Proposed Syntax

#### **A. Explicit Loops in Comprehensions**
```python
# List comprehension using :=
evens = [x $ x := 0..10 ? x % 2 == 0]

# Dict comprehension using :=
squared = {x: x**2 $ x := 1..5}
```

#### **B. Implicit/Inline Loops in Comprehensions**
If the loop target variable is omitted, the sigil `$` acts as the implicit target, allowing highly concise comprehensions:
```python
# Implicit list comprehension
active_emails = [$.email := users ? $.is_active]

# Implicit dict comprehension
user_map = {$.id: $ := users}

# Implicit generator as function argument
total_price = sum($.price := items)
```

### Grammar Design
In `Grammar/python.gram`, update `for_if_clause` to accept either the `in` rule or the `:=` token, and support optional target variables mapping to `$` when omitted:
```peg
for_if_clause[comprehension_ty]:
    | async for a=star_targets (in | ':=') ~ b=disjunction c[asdl_expr_seq*]=(if z=disjunction { z })* {
        CHECK_VERSION(comprehension_ty, 6, "Async comprehensions are", _PyAST_comprehension(a, b, c, 1, p->arena)) }
    | for a=star_targets (in | ':=') ~ b=disjunction c[asdl_expr_seq*]=(if z=disjunction { z })* {
        _PyAST_comprehension(a, b, c, 0, p->arena) }
    # Implicit loop in comprehensions (omitted target variable defaults to '_dollar_item'):
    | for (in | ':=') ~ b=disjunction c[asdl_expr_seq*]=(if z=disjunction { z })* {
        _PyAST_comprehension(_PyAST_Name(_PyPegen_new_identifier(p, "_dollar_item"), Store, EXTRA), b, c, 0, p->arena) }
```
This is free of any parsing conflicts because the `for`/`$` keyword explicitly guards the start of the comprehension clause.

---

## 2. Asset & File Imports (`/ "file.ext" : type => alias`)






### Motivation
In Loh, forward slash `/` syntax is used for module imports (e.g. `/math/sqrt => s`). Importing raw data files (JSON, YAML, HTML templates, binary models) in standard Python requires writing repetitive file-reading boilerplate (`with open(...) as f:`). To make loading static assets DRY and declarative, Loh extends slash import syntax to quoted string paths, automatically parsing JSON/YAML or reading text/binary files directly into local variables.

### Proposed Syntax
Because standard Python module imports use unquoted identifiers (`/math`), using a **quoted string literal** (`"..."` or `'...'`) immediately following `/` unambiguously identifies a **File Asset Import**.

#### **A. Basic Asset Imports with Auto-Inferred Variable Names**
If `=> alias` is omitted, the variable name is automatically inferred from the file basename (stripping folder paths and extensions):
```python
/ "configs/database.json" : dict       # Parses JSON into local variable 'database'
/ "templates/email/header.html" : str  # Reads text into local variable 'header'
/ "../shared/logo.png" : bytes         # Reads binary into local variable 'logo'
```

#### **B. Explicit Variable Binding (`=> alias`)**
```python
/ "configs/prod.json" : dict => config
/ "templates/view.html" : str => view_tpl
/ "models/net.onnx" : bytes => weights
```

### Type Modifiers & Extension Inferences

| Import Expression | Inferred / Explicit Mode | Resulting Data Type |
| :--- | :--- | :--- |
| **`/ "data.json" : dict`** | Auto-inferred `.json` | Parsed Dictionary / List |
| **`/ "config.yaml" : dict`** | Auto-inferred `.yaml` | Parsed Dictionary / List |
| **`/ "page.html" : str`** | Auto-inferred `.html` / `.txt` / `.md` | `str` (Text String) |
| **`/ "blob.dat" : bytes`** | Auto-inferred `.bin` / `.dat` / `.png` | `bytes` (Byte Stream) |
| **`/ str "data.json"`** | Explicit `str` modifier | Raw JSON string (unparsed) |
| **`/ bytes "data.json"`** | Explicit `bytes` modifier | Raw UTF-8 bytes |

### Path Resolution & Compile-Time Desugaring
Asset paths inside quotes support relative folder paths (`"../../assets/data.json"`). Path resolution occurs relative to the directory of the executing `.loh` source file.

At parse-time, the PEG parser in `python.gram` parses `/ "path" : type => alias` and compiles it into an inline file loading helper call:
```python
# / "configs/database.json" : dict => config desugars to:
import json as _json, pathlib as _pathlib
_asset_path = _pathlib.Path(__file__).parent / "configs/database.json"
with open(_asset_path, "r", encoding="utf-8") as _f:
    config = _json.load(_f)
```
