import ast
import unittest


class LohAssertNotTests(unittest.TestCase):
    def test_caret_question_parses_as_assert_not(self):
        tree = ast.parse("^? x")
        stmt = tree.body[0]

        self.assertIsInstance(stmt, ast.Assert)
        self.assertIsInstance(stmt.test, ast.UnaryOp)
        self.assertIsInstance(stmt.test.op, ast.Not)
        self.assertIsInstance(stmt.test.operand, ast.Name)
        self.assertEqual(stmt.test.operand.id, "x")

    def test_caret_question_supports_message(self):
        tree = ast.parse("^? x, 'boom'")
        stmt = tree.body[0]

        self.assertIsInstance(stmt, ast.Assert)
        self.assertIsInstance(stmt.msg, ast.Constant)
        self.assertEqual(stmt.msg.value, "boom")

    def test_caret_question_runtime_behavior(self):
        namespace = {"x": True}
        with self.assertRaises(AssertionError):
            exec(compile(ast.parse("^? x"), "<loh-test>", "exec"), namespace, namespace)

    def test_existing_assert_alias_still_works(self):
        tree = ast.parse("^?! x")
        stmt = tree.body[0]

        self.assertIsInstance(stmt, ast.Assert)
        self.assertIsInstance(stmt.test, ast.Name)
        self.assertEqual(stmt.test.id, "x")


if __name__ == "__main__":
    unittest.main()
