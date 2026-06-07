# Loh Planned Features

This document tracks features that have been approved for implementation in **Loh**.


## 1. Loop Syntax & Control Flow Upgrades

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

