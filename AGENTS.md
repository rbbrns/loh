# Loh Feature Addition Process

This document outlines the systematic workflow for implementing new syntax features in the Loh language.

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
