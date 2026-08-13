# Loh Rejected Ideas

This document tracks language enhancement ideas that were proposed but ultimately rejected, along with the technical rationale for their rejection.

---

## 1. Index/Key-Safe Subscripting Postfix Operator (`lst[x]~`)

### Proposed Syntax
```python
item = lst[x]~
```
Designed to return `None` on `IndexError` or `KeyError` instead of throwing an exception.

### Reason for Rejection
**Syntactic Conflict with Postfix Assignment**: 
In Loh, a trailing `~` acts as a postfix statement assignment meaning "assign `None`" (e.g. `x~` $\rightarrow$ `x = None`, and `obj.prop~` $\rightarrow$ `obj.prop = None`).
If `lst[x]~` were introduced as an expression, it would create a severe collision where writing `lst[x]~` on its own line (expecting a safe lookup) would be compiled as an implicit assignment statement mutating the list:
```python
lst[x] = None
```
To maintain the rule that **postfix `~` is strictly for None value assignment/comparison**, this syntax was rejected.

---

## 2. Implicit Collection Concatenation & Merging (`[1] [2]`, `{a} {b}`)

### Proposed Syntax
```python
# Implicit dict merging
config = {"host": "localhost"} {"port": 8080}

# Implicit list concatenation
items = [1, 2] [3, 4]
```

### Reason for Rejection
**Grammatical Ambiguity & Low Utility**:
1. **List/Tuple Subscript & Call Collisions**: In standard Python, `[1, 2][0]` is valid subscripting (indexing element 0 of `[1, 2]`), and `(1,)(2,)` is function invocation. Supporting space-separated collection literals for lists/tuples introduces severe syntactic ambiguity with indexing and calls.
2. **Standard Unpacking and Merge Operators Are Superior**: Python and Loh already provide explicit, unambiguous unpacking (`[*l1, *l2]`, `{**d1, **d2}`) and dedicated merge/extension operators (`|`, `+:`, `|:`). Implicit space-separated merging introduces hidden behavior and low practical utility compared to explicit unpacking.

---

## 3. Postfix Loop Statements (`statement := collection`)

### Proposed Syntax
```python
process($) := items
print($.name) := users ? $.is_active
```

### Reason for Rejection
**Syntactic Redundancy & Overlap with List Comprehensions**:
1. **Overlap with Comprehension Syntax**: Loh uses `:=` for collection binding inside list comprehensions (e.g., `[process($) := items]`). Allowing `statement := collection` outside brackets introduces ambiguity between statement-level side effects and list comprehension expressions.
2. **Clearer Separation of Concerns**: Reserving `:=` for comprehensions (`[expr := items]`) and using explicit loop blocks (`$ item := items:`) for side effects maintains a clean distinction between functional mapping and control flow.

---

## 4. Smart Type Guard & Alias Unpacking (`? x : Type => var:`)

### Proposed Syntax
```python
? payload : dict => d:
    print("User ID:", d~['id'])
```

### Reason for Rejection
**Redundancy with Unified Structural Pattern Matching**:
Pattern matching in [PLANNED.md](file:///Users/robbarnes/Development/loh/PLANNED.md#L49-L120) already covers type checking, destructuring, and local variable binding via `? target => pattern:` (e.g. `? payload => dict(d):` or `? payload => str(s):`). Introducing a separate `: Type => var` syntax inside conditional headers creates unnecessary syntactic overlap and redundancy.

---

## 5. Implicit Receiver Context Blocks (`with obj:`)

### Proposed Syntax
```python
with matplotlib.pyplot:
    .plot(x, y)
    .title("Sales Over Time")
    .show()
```

### Reason for Rejection
**Semantic Collision & Superior Macro Alternatives**:
1. **Collision with Context Managers**: In Loh, `with` statements use the `&` sigil (`& open(...) => f:`). Overloading `& obj:` or `with obj:` to mean "receiver context block" creates severe semantic confusion with standard `__enter__` / `__exit__` context managers.
2. **Superior Macro Alternative**: Loh's **Syntactic Merge Operator** `plt. {{ plot(); title() }}` (Idea 21) already solves receiver distribution at compile-time with zero runtime overhead and without hijacking context manager syntax.



