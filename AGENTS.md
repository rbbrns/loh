Loh is an expiremental superset of Python that adds concise syntax to make Python
more intuitive and DRY. By replacing builtin keywords with symbols, the intent and reasoning
of the solution is emphasized.

See @README.md for more details on the language.

See samples, tests and git history for even deeper context.

## Loh Feature Addition Process

1. **AST Helper Implementation**:
   - Declare AST desugaring helper functions in [pegen.h](file:///Users/robbarnes/Development/loh/Parser/pegen.h).
   - Implement the helpers in [action_helpers.c](file:///Users/robbarnes/Development/loh/Parser/action_helpers.c).
   
2. **Grammar Modification**:
   - Define new token definitions in [Tokens](file:///Users/robbarnes/Development/loh/Grammar/Tokens) if new symbols are introduced.
   - Add new parser rules under appropriate nodes (e.g., `primary`, `expression`) in [python.gram](file:///Users/robbarnes/Development/loh/Grammar/python.gram).
   
3. **Parser Regeneration**:
   - Regenerate the parser and token representations using the local synchronized interpreter:
     ```bash
     make regen-all PYTHON_FOR_REGEN=./python.exe
     ```
   
4. **Compilation**:
   - Recompile the interpreter with the updated parser and helpers:
     ```bash
     make -j8 python.exe
     ```
   
5. **Unit Testing**:
   - Add targeted test cases under `Lib/test/test_loh/` (e.g., [test_none_operators.py](file:///Users/robbarnes/Development/loh/Lib/test/test_loh/test_none_operators.py)).
   - Run the Loh test suite using the local interpreter:
     ```bash
     ./python.exe -m test test_loh
     ```
   - Ensure standard CPython parser/compiler tests pass cleanly:
     ```bash
     ./python.exe -m test test_grammar test_syntax test_compile test_ast test_dis
     ```
   
6. **Documentation**:
   - Add the new syntax to the mapping cheat sheet table and write a dedicated features section in [README.md](file:///Users/robbarnes/Development/loh/README.md).
   - Verify README examples by writing assertions in [test_readme.py](file:///Users/robbarnes/Development/loh/Lib/test/test_loh/test_readme.py).

7. **Feature Lifecycle Management**:
   - **IDEAS.md** acts as a general pool of potential/proposed language feature ideas.
   - When an idea is selected to be implemented next, it is moved from **IDEAS.md** to **PLANNED.md**.
   - A feature must never exist in both **IDEAS.md** and **PLANNED.md** at the same time.
   - Once a feature is fully implemented, verified, and committed, it must be completely removed from **PLANNED.md** (and also from **IDEAS.md** if any traces remain there).

