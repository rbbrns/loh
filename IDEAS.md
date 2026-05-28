# Loh Language Enhancement Ideas

This document tracks potential future language enhancements and syntax upgrades for **Loh**.

---

## 1. Loop Auto-Enumeration with `$, item <~ collection` (Featured)

### Motivation
Tracking iteration indices is a common pattern that currently requires wrapping collections in `enumerate(collection)`. This adds visual nesting and boilerplate. We want a lightweight, unambiguous way to capture the loop index directly using the loop sigil `$`.

### Proposed Syntax
By placing a comma `,` immediately after the loop keyword `$`, the loop symbol itself doubles as the index variable name. Inside the loop body, `$` evaluates to the current iteration index:

```python
# Loop with index
$, item <~ items:
    print($, item)  # '$' is the index
```

### Compile-Time Desugaring
At parse-time, the PEG parser translates the `$` index variable to a compiler-safe variable name (e.g., `_dollar_idx`) and wraps the collection in a call to `enumerate()`:

1. **Header Translation**:
   ```python
   $, item <~ items:
   # becomes:
   for _dollar_idx, item in enumerate(items):
   ```
2. **Body Translation**:
   Any standalone `$` inside the loop body is parsed as `_dollar_idx`:
   ```python
   print($, item)
   # becomes:
   print(_dollar_idx, item)
   ```

### Technical Implementation Path
This is implemented entirely in the parser, with zero changes required to the bytecode compiler, runtime, or VM:

1. **Grammar Expression Rule** (in [python.gram](file:///Users/robbarnes/Development/loh/Grammar/python.gram) under `atom`):
   ```peg
   atom[expr_ty]:
       | '$' { _PyAST_Name(CHECK(PyObject*, _PyPegen_new_identifier(p, "_dollar_idx")), Load, EXTRA) }
   ```
2. **Grammar Statement Rule** (in [python.gram](file:///Users/robbarnes/Development/loh/Grammar/python.gram) under `for_stmt`):
   ```peg
   for_stmt[stmt_ty]:
       | '$' ',' t=star_targets 'in' ~ ex=star_expressions ':' tc=[TYPE_COMMENT] b=block el=[loop_else_block] {
           _PyAST_For(
               _PyAST_Tuple(CHECK(asdl_expr_seq*, _PyPegen_seq_insert_in_front(p, 
                   _PyAST_Name(CHECK(PyObject*, _PyPegen_new_identifier(p, "_dollar_idx")), Store, EXTRA),
                   _PyPegen_singleton_seq(p, t)
               )), Store, EXTRA),
               _PyAST_Call(
                   _PyAST_Name(CHECK(PyObject*, _PyPegen_new_identifier(p, "enumerate")), Load, EXTRA),
                   _PyPegen_singleton_seq(p, ex),
                   NULL, NULL, NULL, EXTRA
               ),
               b, el, NEW_TYPE_COMMENT(p, tc), EXTRA
           )
       }
   ```

---

## 2. None-Safe Navigation / Safe Attribute Access (`~.`)

### The Lexical & Parser Conflict with `?.` (Question-Dot)
In Loh, `?` is used for conditional `if` and ternary expressions, and `.` is used for accessing properties on `self`. This creates a severe grammatical conflict when parsing `?.`:
```python
print(x ? .y ?? z)  # Valid ternary expression checking x, returning self.y or z
```
If `?.` is treated as a safe navigation operator, the parser cannot distinguish it from a standard ternary followed by a self-attribute access (`x ? .y`), leading to parsing ambiguities. 

To resolve this conflict and align with Loh's established symbols, we adopt the **`~.` (Tilde-Dot)** operator.

---

### Proposed Syntax
Using the tilde-dot `~.` operator signifies a "maybe `.property`" access:
```python
city = user~.profile~.address~.city
```
* **Mental Model**: *"Access `profile` if `user` is not `None` (`~`)."*
* **Grammar Conflict Check**: No conflict. Tilde `~` is a unary operator in Python/Loh, meaning `expression ~ .attribute` is syntactically invalid, allowing the parser to disambiguate `~.` with 100% certainty.

---

### Alternative Candidates Evaluated
1. **The "Dot-with-a-line" Operator (`!`)**: `config!host`. Uses exclamation as a modified dot. Dropped because `!` represents logical NOT, and Swift/Kotlin developers expect `!` to mean force-unwrap (unsafe) rather than safe-navigation.
2. **The "Dot-Pipe" Operator (`.|`)**: `config.|host`. Combines dot and pipe to represent a diversion path if the value is `None`.

---

### Compile-Time Desugaring
The `~.` operator desugars at parse-time into nested ternary checks:
```python
# Desugared Python AST
_val1 = user
city = (_val1.profile.address.city if (_val2 := _val1.profile) and _val2.address else None) if _val1 else None
```

---

## 3. TypeScript-Style Constructor Parameter Properties (`.param`)

### Motivation
Automatically declares constructor parameters as instance attributes and generates assignments at the beginning of the function body.

### Proposed Syntax
```python
Account:BaseAccount:
    .__init__(.owner, .balance, .email):
        pass # assignments are implicitly generated
```

### Compile-Time Desugaring
```python
class Account(BaseAccount):
    def __init__(self, owner, balance, email):
        self.owner = owner
        self.balance = balance
        self.email = email
```

---

## 4. Pipe Operator Placeholders (`_`)

### Motivation
Allows calling multi-argument functions within a pipe chain without wrapping them in an anonymous lambda.

### Proposed Syntax
```python
data |> json.dumps(_, indent=4)
"log.txt" |> open(_, "w")
```

### Compile-Time Desugaring
If the parser detects a `_` identifier within the call argument list of the piped expression, it replaces `_` with the piped variable.

---

## 5. None-Coalescing Binary Operator (`~~`)

### Motivation
Similar to how JavaScript/TypeScript doubled the conditional symbol `?` to create the null-coalescing operator `??`, Loh doubles its native `None` symbol `~` to create the None-coalescing operator `~~`. This operator behaves like logical OR (`||`), but checks strictly for `None` instead of general truthiness.

---

### Proposed Syntax
```python
host = config.host ~~ "localhost"
port = config.port ~~ 8080
```

### Compile-Time Desugaring
At parse-time, the binary `~~` operator desugars into a standard conditional expression:
```python
# Desugared Python AST
host = config.host if config.host is not None else "localhost"
```

### Grammar Conflict Check
No conflict. In Python/Loh, `~` is a unary operator (bitwise NOT). The grammar never allows `expression ~ ~ expression` without an infix binary operator in between, enabling `~~` to be cleanly parsed as a single binary operator.
