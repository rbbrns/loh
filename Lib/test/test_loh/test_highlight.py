import unittest
import sys
import os
import tokenize

# Import highlight functions
from Tools.scripts.highlight_loh import highlight_loh, get_token_color, Theme

class TestLohHighlighting(unittest.TestCase):
    def setUp(self):
        self.theme = Theme()

    def test_basic_highlighting(self):
        # Comments
        code = "# hello world"
        highlighted = highlight_loh(code, self.theme)
        self.assertIn(self.theme.syntax.comment, highlighted)
        self.assertIn("# hello world", highlighted)

        # Numbers
        code = "123.45"
        highlighted = highlight_loh(code, self.theme)
        self.assertIn(self.theme.syntax.number, highlighted)

        # Strings
        code = '"hello string"'
        highlighted = highlight_loh(code, self.theme)
        self.assertIn(self.theme.syntax.string, highlighted)

    def test_loh_constants(self):
        # ++, --, ~ should be highlighted as keyword_constant
        code = "x = ++"
        highlighted = highlight_loh(code, self.theme)
        self.assertIn(self.theme.syntax.keyword_constant, highlighted)
        self.assertIn("++", highlighted)

        code = "y = --"
        highlighted = highlight_loh(code, self.theme)
        self.assertIn(self.theme.syntax.keyword_constant, highlighted)
        self.assertIn("--", highlighted)

        code = "z = ~"
        highlighted = highlight_loh(code, self.theme)
        self.assertIn(self.theme.syntax.keyword_constant, highlighted)
        self.assertIn("~", highlighted)

    def test_loh_control_flow(self):
        # ?, ??, $, -> should be highlighted as keyword
        code = "? x == 1:"
        highlighted = highlight_loh(code, self.theme)
        self.assertIn(self.theme.syntax.keyword, highlighted)
        self.assertIn("?", highlighted)

        code = "-> value"
        highlighted = highlight_loh(code, self.theme)
        self.assertIn(self.theme.syntax.keyword, highlighted)
        self.assertIn("->", highlighted)

        code = "$ item <~ items:"
        highlighted = highlight_loh(code, self.theme)
        self.assertIn(self.theme.syntax.keyword, highlighted)
        self.assertIn("$", highlighted)
        self.assertIn("<~", highlighted)

    def test_shorthand_self_dot(self):
        # Shorthand self dots should be highlighted as keyword_constant
        code = ".value = 1"
        highlighted = highlight_loh(code, self.theme)
        self.assertIn(self.theme.syntax.keyword_constant, highlighted)

        # Standalone self dot
        code = "."
        highlighted = highlight_loh(code, self.theme)
        self.assertIn(self.theme.syntax.keyword_constant, highlighted)

        # Normal attribute dot should NOT be keyword_constant (should be op/reset)
        code = "obj.value = 1"
        highlighted = highlight_loh(code, self.theme)
        # Find the dot token's position
        dot_idx = highlighted.index(".")
        # The dot shouldn't have the keyword_constant color prefix
        self.assertNotIn(self.theme.syntax.keyword_constant, highlighted[dot_idx - 10 : dot_idx + 1])

    def test_definitions(self):
        # Class definitions (ClassName:: or ClassName:Bases:)
        code = "MyClass::"
        highlighted = highlight_loh(code, self.theme)
        self.assertIn(self.theme.syntax.definition, highlighted)
        self.assertIn("MyClass", highlighted)

        code = "Child:(Parent):"
        highlighted = highlight_loh(code, self.theme)
        self.assertIn(self.theme.syntax.definition, highlighted)
        self.assertIn("Child", highlighted)

        # Standard Python def/class
        code = "def my_func():"
        highlighted = highlight_loh(code, self.theme)
        self.assertIn(self.theme.syntax.definition, highlighted)
        self.assertIn("my_func", highlighted)

        code = "class PyClass:"
        highlighted = highlight_loh(code, self.theme)
        self.assertIn(self.theme.syntax.definition, highlighted)
        self.assertIn("PyClass", highlighted)

        # Loh function definitions (func(...) :)
        code = "add_task(name: str):"
        highlighted = highlight_loh(code, self.theme)
        self.assertIn(self.theme.syntax.definition, highlighted)
        self.assertIn("add_task", highlighted)

        code = "run_day() -> str:"
        highlighted = highlight_loh(code, self.theme)
        self.assertIn(self.theme.syntax.definition, highlighted)
        self.assertIn("run_day", highlighted)

        # But function call should NOT be definition
        code = "print(result)"
        highlighted = highlight_loh(code, self.theme)
        self.assertNotIn(self.theme.syntax.definition, highlighted)

    def test_empty_theme(self):
        # When color is disabled, output should match input exactly
        from Tools.scripts.highlight_loh import get_theme
        no_color_theme = get_theme(force_no_color=True)

        code = "? x === ~:\n    .value = + # comment\n"
        highlighted = highlight_loh(code, no_color_theme)
        self.assertEqual(highlighted, code)

if __name__ == "__main__":
    unittest.main()
