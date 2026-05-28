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

## 2. Null-Safe Navigation / Safe Attribute Access

### The Lexical & Parser Conflict with `?.` (Question-Dot)
In Loh, `?` is used for conditional `if` and ternary expressions, and `.` is used for accessing properties on `self`. This creates a severe grammatical conflict when parsing `?.`:
```python
print(x ? .y ?? z)  # Valid ternary expression checking x, returning self.y or z
```
If `?.` is treated as a safe navigation operator, the parser cannot distinguish it from a standard ternary followed by a self-attribute access (`x ? .y`), leading to parsing ambiguities and syntax errors. To resolve this, Loh must adopt a conflict-free safe-navigation operator.

---

### Candidate A: The "Dot-with-a-line" Operator (`!`)
We use the exclamation mark `!` as an infix binary member-access operator.
```python
city = user!profile!address!city
```
* **Visual Rationale**: `!` is visually a dot with a line above it, indicating a modified/conditional property access.
* **Grammar Conflict Check**: No conflict. In Loh, `!` is a unary prefix operator (logical NOT). Having `expression ! attribute` (infix) is not valid standard Python/Loh syntax, allowing the parser to disambiguate it with 100% certainty.
* **Pros**: Extremely concise (1 character) and highly readable.

---

### Candidate B: The "Dot-Pipe" Operator (`.|`)
We use `.|` (Dot followed by vertical bar) as the operator.
```python
city = user.|profile.|address.|city
```
* **Visual Rationale**: It uses the standard dot `.` for member access, combined with the pipe `|` to represent a diversion or alternative route if the value is `None` (analogous to logical OR `||` but for member access).
* **Grammar Conflict Check**: No conflict. Standard grammar never allows `expression . | attribute`, making `.|` completely unique.
* **Pros**: Explicitly preserves the dot `.` for property access while cleanly signaling fallback semantics.

---

### Candidate C: The "None-Safe" Operator (`~.`)
We use the tilde-dot `~.` as the operator.
```python
city = user~.profile~.address~.city
```
* **Visual Rationale**: Links directly to Loh's symbol for `None` (`~`).
* **Grammar Conflict Check**: No conflict. Tilde `~` is a unary operator in Python, meaning `expression ~ .attribute` is syntactically invalid.
* **Pros**: Semantically aligns with Loh's representation of nullability (`~`).

---

### Compile-Time Desugaring
Regardless of the chosen operator, the syntax desugars at parse-time into nested ternary checks:
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

## 5. Null-Coalescing Binary Operator (`~?`)

### Motivation
A clean operator to assign fallback values only when the left-hand value is strictly `None` (represented by `~` in Loh), avoiding unexpected behavior with falsy values like `0` or `""`.

### Proposed Syntax
```python
host = config.host ~? "localhost"
```

### Compile-Time Desugaring
```python
host = config.host if config.host is not None else "localhost"
```
