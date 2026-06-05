import unittest

class Tests(unittest.TestCase):
    def test_rescue_catch_any_basic(self):
        # Successful expression
        self.assertEqual(int("42") ^? 0, 42)
        
        # Exception raised and rescued
        self.assertEqual(int("invalid") ^? 0, 0)
        self.assertEqual(int("invalid") ^? 99, 99)

    def test_rescue_catch_specific(self):
        # ValueError is caught
        self.assertEqual(int("invalid") ^? ValueError -> -1, -1)
        
        # TypeError is propagated (not caught)
        with self.assertRaises(TypeError):
            _ = int(None) ^? ValueError -> -1

    def test_rescue_multiple_exceptions(self):
        # ValueError is caught
        self.assertEqual(int("invalid") ^? (ValueError | TypeError) -> -1, -1)
        # TypeError is caught
        self.assertEqual(int(None) ^? (ValueError | TypeError) -> -1, -1)
        
        # KeyErrors are not caught
        with self.assertRaises(KeyError):
            _ = {}["missing"] ^? (ValueError | TypeError) -> -1

    def test_rescue_variable_binding(self):
        # ValueError caught and exception instance bound to 'e'
        self.assertEqual(int("invalid") ^? (ValueError => e) -> len(e.args), 1)
        
        # Multiple exceptions with binding
        self.assertEqual(int(None) ^? ((ValueError | TypeError) => err) -> type(err).__name__, "TypeError")

    def test_rescue_flow_control_exceptions(self):
        # KeyboardInterrupt and SystemExit should NOT be caught by standard catch-any
        # since standard catch-any defaults to catching Exception (not BaseException)
        def raise_keyboard_interrupt():
            raise KeyboardInterrupt()

        with self.assertRaises(KeyboardInterrupt):
            _ = raise_keyboard_interrupt() ^? 0

        def raise_system_exit():
            raise SystemExit()

        with self.assertRaises(SystemExit):
            _ = raise_system_exit() ^? 0

        # However, if explicitly specified, BaseException or KeyboardInterrupt can be caught
        self.assertEqual(raise_keyboard_interrupt() ^? KeyboardInterrupt -> 99, 99)
        self.assertEqual(raise_system_exit() ^? BaseException -> 99, 99)

    def test_rescue_single_evaluation(self):
        # Ensure that the body is evaluated exactly once, and fallback is only evaluated if exception occurs
        eval_count = 0
        fallback_eval_count = 0

        def get_val():
            nonlocal eval_count
            eval_count += 1
            return 42

        def get_fallback():
            nonlocal fallback_eval_count
            fallback_eval_count += 1
            return 99

        # Success case: body evaluated once, fallback NOT evaluated
        self.assertEqual(get_val() ^? get_fallback(), 42)
        self.assertEqual(eval_count, 1)
        self.assertEqual(fallback_eval_count, 0)

        # Failure case: body evaluated, fallback evaluated
        def get_fail():
            nonlocal eval_count
            eval_count += 1
            raise ValueError()

        eval_count = 0
        self.assertEqual(get_fail() ^? get_fallback(), 99)
        self.assertEqual(eval_count, 1)
        self.assertEqual(fallback_eval_count, 1)

if __name__ == '__main__':
    unittest.main()
