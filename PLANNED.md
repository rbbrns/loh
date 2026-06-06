# Loh Planned Features

This document tracks features that have been approved for implementation in **Loh**.


## 1. Implicit Return from Final Function Expression (`from __loh__ import implicit_returns`)


### Motivation
Writing explicit return statements `->` in short function bodies is still slightly verbose. In expression-oriented languages (like Rust, Ruby, and Scala), the final expression evaluated in a block/function is automatically returned, making method and helper declarations extremely concise.

To prevent breaking compatibility with standard Python code (where side-effect functions ending with expression statements are expected to return `None`), this feature is opt-in per module via a future-style import.

### Proposed Syntax
Using the `implicit_returns` future import from the `__loh__` module:
```python
from __loh__ import implicit_returns

# Automatically returns base_price * 1.05
calculate_total(base_price):
    tax_rate = 0.05
    base_price * (1 + tax_rate)
```

### Compile-Time Desugaring
If the `implicit_returns` feature is active for the module, the compiler detects when the final statement of a function body is an `Expr` node (and not a control flow keyword or a docstring) and wraps it in a `Return` AST node:
```python
from __loh__ import implicit_returns

def calculate_total(base_price):
    tax_rate = 0.05
    return base_price * (1 + tax_rate)
```

---

## 2. Exception & Loop Control Syntax Cleanups

### Motivation
Loh's exception and loop control features currently have some redundant symbols, overloading conflicts (such as `^?` being used for both `except` and `assert not`), and overly verbose symbols (such as `?!$>>:` for loop-else). To streamline the language grammar, reduce cognitive load, and resolve symbol conflicts, a series of syntax cleanups and tweaks are approved.

### Proposed Changes

#### **1. Exception Blocks & Rescue**
* **Try / Guard**: **`^:`**
* **Except / Catch**: **`?^`** (Replaces `^?` to resolve the conflict with `assert not`)
* **Except-Star / Exception Groups**: **`?^*`** (Replaces `^?*`)
* **Try-Else**: **`?!^:`** (Translates to "if not exception", representing `?` + `!` + `^`)
* **Finally / Cleanup**: **`*:`** (Replaces `^*:` / `?*:` with a clean, 2-character wildcard symbol meaning "always/all cases")
* **Inline Exception Rescue**: **`?^`** (Replaces `^?` to remain consistent with the catch/except symbol, e.g., `expr ?^ fallback` / `expr ?^ ValueError -> fallback`)

#### **2. Assertions**
* **Assert**: **`^?!`**
* **Assert Not**: **`^?`** (Kept exclusively for assertions, since `except` is moved to `?^`)

#### **3. Loop Control & Loop-Else**
* **Break**: **`$>`** (Replaces `$>>` with a shorter, 2-character exit arrow)
* **Continue**: **`$<`** (Replaces `$<<` with a shorter, 2-character return arrow)
* **Loop-Else**: **`?!$>:`** (Translates to "if not break". By shortening break to `$>`, this block header naturally becomes much cleaner to read and write)

### Compile-Time Desugaring
1. In `Grammar/python.gram`:
   - Redefine tokens:
     - `break: ( 'break' | '$>' )`
     - `continue: ( 'continue' | '$<' )`
     - `except: ( 'except' | '?^' )`
     - `finally: ( 'finally' | '*:' )`
   - Remove legacy Set 1 exception tokens (`~^`, ?*).
2. The loop-else block parser rule `loop_else_block` naturally parses `?!$>:` as `if not break :` due to token mapping.
3. The try-else block parser rule `try_else_block` naturally parses `?!^:` as `if not '^' :` due to token mapping.

---

## 3. Parameter Keyword-Only Multi-Name Aliases (`limit | l = 100`)

### Motivation
For command-like API interfaces or backward-compatibility during refactoring, developers often want parameters to accept multiple keyword argument names (e.g., accepting both `limit` and `l`). Using the pipe `|` symbol allows specifying keyword fallbacks. To prevent positional arguments from bleeding into alias slots, the aliases are compiled as keyword-only arguments.

### Proposed Syntax
```python
def fetch(limit | l = 100, offset | o = 0):
    # The body only references the primary names: limit and offset
    print(limit, offset)
```

### Compile-Time Desugaring
The compiler places the aliases after a `*` separator in the generated signature to make them keyword-only. It then checks for conflicts and resolves the value to the primary name:
```python
_LOH_SENTINEL = object()

def fetch(limit=_LOH_SENTINEL, offset=_LOH_SENTINEL, *, l=_LOH_SENTINEL, o=_LOH_SENTINEL):
    # 1. Raise TypeError if caller passed both names
    if limit is not _LOH_SENTINEL and l is not _LOH_SENTINEL:
        raise TypeError("fetch() got multiple values for alias parameter 'limit'/'l'")
    if offset is not _LOH_SENTINEL and o is not _LOH_SENTINEL:
        raise TypeError("fetch() got multiple values for alias parameter 'offset'/'o'")
        
    # 2. Resolve to the primary variable
    limit = limit if limit is not _LOH_SENTINEL else (l if l is not _LOH_SENTINEL else 100)
    offset = offset if offset is not _LOH_SENTINEL else (o if o is not _LOH_SENTINEL else 0)
    
    # 3. Clean up the alias name so they cannot be accessed in the body
    del l, o
    
    print(limit, offset)
```

---

## 4. Loop Syntax & Control Flow Upgrades

### Motivation
To simplify common loop patterns and remove nesting boilerplate, Loh can support short-form implicit loops and inline header filtering.

### Proposed Syntax
1. **Implicit Loops (`$ <~`)**: Omitting the target variable name binds the loop sigil `$` to the current element inside the loop body.
   ```python
   # Print each item
   $ <~ items:
       print($)
   ```
2. **Inline Loop Filters**: Allows placing a query filter `? condition` directly in the loop signature row.
   ```python
   # Loops only over active users
   $ user <~ users ? user.is_active:
       send_email(user)
   ```

### Compile-Time Desugaring
1. `$ <~ collection` desugars to:
   ```python
   for _dollar_item in collection:
       # Any standalone '$' in the body resolves to '_dollar_item'
   ```
2. `$ user <~ users ? user.is_active:` desugars to:
   ```python
   for user in users:
       if not (user.is_active):
           $<  # continue
   ```
