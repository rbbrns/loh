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
