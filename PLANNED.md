# Loh Planned Features

This document tracks features that have been approved for implementation in **Loh**.


## 1. Parameter Keyword-Only Multi-Name Aliases (`limit | l = 100`)

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

## 2. Loop Syntax & Control Flow Upgrades

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

