import unittest

class Tests(unittest.TestCase):
    def test_exception_unified_basic(self):
        ran = False
        ^:
            result = 10 / 0
        ?^ ZeroDivisionError => e:
            ran = True
        self.assertTrue(ran)

    def test_exception_unified_full(self):
        ran_except = False
        ran_else = False
        ran_finally = False
        
        # Scenario 1: Exception is raised
        ^:
            result = 10 / 0
        ?^ ZeroDivisionError:
            ran_except = True
        ?!^:
            ran_else = True
        *:
            ran_finally = True

        self.assertTrue(ran_except)
        self.assertFalse(ran_else)
        self.assertTrue(ran_finally)

        # Scenario 2: No exception is raised
        ran_except = False
        ran_else = False
        ran_finally = False
        
        ^:
            result = 10 / 2
        ?^ ZeroDivisionError:
            ran_except = True
        ?!^:
            ran_else = True
        *:
            ran_finally = True

        self.assertFalse(ran_except)
        self.assertTrue(ran_else)
        self.assertTrue(ran_finally)

    def test_exception_unified_star(self):
        ran_except_star = False
        ^:
            raise ExceptionGroup("group", [ValueError("error")])
        ?^* ValueError => eg:
            ran_except_star = True

        self.assertTrue(ran_except_star)

    def test_exception_multiple_handlers(self):
        handler_matched = None
        ^:
            raise TypeError("type error")
        ?^ ValueError:
            handler_matched = "value"
        ?^ TypeError:
            handler_matched = "type"
        ?^:
            handler_matched = "any"

        self.assertEqual(handler_matched, "type")

if __name__ == "__main__":
    unittest.main()
