"""
Test cases for the pipe operator (|>)

These tests should be added to Lib/test/test_syntax.py or a new file
like Lib/test/test_pipe.py in your CPython fork.
"""

import unittest
import sys
import operator
from test.support import check_syntax_error


class TestPipeOperator(unittest.TestCase):
    """Test cases for the pipe operator (|>)"""
    
    # Basic functionality tests
    
    def test_simple_pipe(self):
        """Test basic pipe: x |> f becomes f(x)"""
        def double(x):
            return x * 2
        
        result = 5 |> double
        self.assertEqual(result, 10)
    
    def test_pipe_chain(self):
        """Test chained pipes: x |> f |> g becomes g(f(x))"""
        def add_one(x):
            return x + 1
        
        def double(x):
            return x * 2
        
        result = 5 |> add_one |> double
        self.assertEqual(result, 12)  # (5 + 1) * 2
    
    def test_long_pipe_chain(self):
        """Test longer pipe chains"""
        def add_one(x):
            return x + 1
        
        result = 1 |> add_one |> add_one |> add_one |> add_one
        self.assertEqual(result, 5)
    
    def test_pipe_with_builtin(self):
        """Test piping to built-in functions"""
        result = "hello" |> len
        self.assertEqual(result, 5)
        
        result = [1, 2, 3] |> sum
        self.assertEqual(result, 6)
        
        result = "hello" |> str.upper
        self.assertEqual(result, "HELLO")
    
    def test_pipe_with_lambda(self):
        """Test piping to lambda functions"""
        result = 5 |> (x) -> x * 2
        self.assertEqual(result, 10)
        
        result = 10 |> (x) -> x + 1 |> (x) -> x * 2
        self.assertEqual(result, 22)
    
    def test_pipe_with_type_constructors(self):
        """Test piping to type constructors"""
        result = "123" |> int
        self.assertEqual(result, 123)
        
        result = 3.14 |> str
        self.assertEqual(result, "3.14")
        
        result = [1, 2, 3] |> tuple
        self.assertEqual(result, (1, 2, 3))
    
    # Precedence tests
    
    def test_pipe_precedence_with_arithmetic(self):
        """Test that arithmetic binds tighter than pipe"""
        def double(x):
            return x * 2
        
        # (5 + 3) |> double
        result = 5 + 3 |> double
        self.assertEqual(result, 16)
        
        # (10 - 2) |> double
        result = 10 - 2 |> double
        self.assertEqual(result, 16)
    
    def test_pipe_precedence_with_comparison(self):
        """Test that comparisons bind tighter than pipe"""
        def bool_to_int(x):
            return 1 if x else 0
        
        # (5 > 3) |> bool_to_int
        result = 5 > 3 |> bool_to_int
        self.assertEqual(result, 1)
        
        # (5 == 5) |> bool_to_int
        result = 5 == 5 |> bool_to_int
        self.assertEqual(result, 1)
    
    def test_pipe_with_parentheses(self):
        """Test explicit parentheses override precedence"""
        def double(x):
            return x * 2
        
        def add_ten(x):
            return x + 10
        
        # With parens: 5 |> (double + add_ten) would be invalid
        # But: (5 |> double) + 10 is explicit
        result = (5 |> double) + 10
        self.assertEqual(result, 20)
    
    # Complex expression tests
    
    def test_pipe_with_list_comprehension(self):
        """Test piping with list comprehensions"""
        result = [1, 2, 3, 4, 5] |> (lst) -> [x * 2 for x in lst]
        self.assertEqual(result, [2, 4, 6, 8, 10])
    
    def test_pipe_with_method_calls(self):
        """Test piping to bound methods"""
        class Doubler:
            def double(self, x):
                return x * 2
        
        d = Doubler()
        result = 5 |> d.double
        self.assertEqual(result, 10)
    
    def test_pipe_with_nested_calls(self):
        """Test that functions can contain pipe operators"""
        def process(x):
            return x |> (lambda n: n + 1) |> (lambda n: n * 2)
        
        result = 5 |> process
        self.assertEqual(result, 12)
    
    def test_pipe_in_function_argument(self):
        """Test pipe operator in function arguments"""
        def combine(a, b):
            return a + b
        
        result = combine(5 |> (lambda x: x * 2), 3)
        self.assertEqual(result, 13)
    
    def test_pipe_with_generator_expression(self):
        """Test piping with generator expressions"""
        result = range(5) |> (lambda g: sum(x * 2 for x in g))
        self.assertEqual(result, 20)
    
    # Data pipeline tests (realistic use cases)
    
    def test_data_pipeline(self):
        """Test realistic data processing pipeline"""
        data = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
        
        result = (
            data
            |> (lambda lst: filter(lambda x: x % 2 == 0, lst))
            |> list
            |> (lambda lst: map(lambda x: x * 2, lst))
            |> list
        )
        
        self.assertEqual(result, [4, 8, 12, 16, 20])
    
    def test_string_pipeline(self):
        """Test string processing pipeline"""
        result = (
            "  hello world  "
            |> str.strip
            |> str.upper
            |> (lambda s: s.replace(" ", "_"))
        )
        
        self.assertEqual(result, "HELLO_WORLD")
    
    def test_numeric_pipeline(self):
        """Test numeric processing pipeline"""
        from math import sqrt
        
        result = 16 |> sqrt |> int |> (lambda x: x ** 2)
        self.assertEqual(result, 16)
    
    # Edge cases and error conditions
    
    def test_pipe_with_non_callable(self):
        """Test that piping to non-callable raises TypeError"""
        with self.assertRaises(TypeError):
            result = 5 |> 10
    
    def test_pipe_with_none(self):
        """Test piping None through functions"""
        def identity(x):
            return x
        
        result = None |> identity
        self.assertIsNone(result)
    
    def test_pipe_preserves_exceptions(self):
        """Test that exceptions in piped functions propagate"""
        def raise_error(x):
            raise ValueError("test error")
        
        with self.assertRaises(ValueError):
            result = 5 |> raise_error
    
    def test_pipe_with_multiple_args_function(self):
        """Test piping to function expecting multiple args fails correctly"""
        def add(x, y):
            return x + y
        
        # This should raise TypeError: add() missing 1 required positional argument
        with self.assertRaises(TypeError):
            result = 5 |> add
    
    # Integration with other Python features
    
    def test_pipe_in_comprehension(self):
        """Test pipe operator inside comprehensions"""
        def double(x):
            return x * 2
        
        result = [x |> double for x in range(5)]
        self.assertEqual(result, [0, 2, 4, 6, 8])
    
    def test_pipe_with_walrus(self):
        """Test pipe operator with walrus operator"""
        def double(x):
            return x * 2
        
        # (value := 5) |> double
        result = (value := 5) |> double
        self.assertEqual(result, 10)
        self.assertEqual(value, 5)
    
    def test_pipe_with_ternary(self):
        """Test pipe operator with ternary expressions"""
        def double(x):
            return x * 2
        
        # (5 if True else 10) |> double
        result = (5 if True else 10) |> double
        self.assertEqual(result, 10)
    
    def test_pipe_with_star_expressions(self):
        """Test pipe with unpacking"""
        def sum_all(*args):
            return sum(args)
        
        items = [1, 2, 3]
        # Can't directly pipe to *args, but can wrap it
        result = items |> (lambda lst: sum_all(*lst))
        self.assertEqual(result, 6)
    
    # Performance and optimization tests
    
    def test_pipe_chain_evaluation_order(self):
        """Test that pipe chains evaluate left to right"""
        calls = []
        
        def track_call(name):
            def inner(x):
                calls.append(name)
                return x
            return inner
        
        result = 1 |> track_call('a') |> track_call('b') |> track_call('c')
        
        self.assertEqual(calls, ['a', 'b', 'c'])


class TestPipeSyntaxErrors(unittest.TestCase):
    """Test syntax error cases for pipe operator"""
    
    def test_pipe_with_no_right_operand(self):
        """Test that pipe requires a right operand"""
        check_syntax_error(self, "5 |>", "invalid syntax")
    
    def test_pipe_with_no_left_operand(self):
        """Test that pipe requires a left operand"""
        check_syntax_error(self, "|> func", "invalid syntax")
    
    def test_pipe_cannot_be_statement(self):
        """Test that pipe expression as statement is handled"""
        # This should compile but might not do anything useful
        # Actually, it should work fine as an expression statement
        try:
            compile("5 |> int", "<test>", "exec")
        except SyntaxError:
            self.fail("Pipe as expression statement should be valid")


class TestPipeWithBuiltins(unittest.TestCase):
    """Test pipe operator with various built-in functions"""
    
    def test_pipe_with_map(self):
        """Test piping with map"""
        result = [1, 2, 3] |> (lambda lst: map(lambda x: x * 2, lst)) |> list
        self.assertEqual(result, [2, 4, 6])
    
    def test_pipe_with_filter(self):
        """Test piping with filter"""
        result = range(10) |> (lambda r: filter(lambda x: x % 2 == 0, r)) |> list
        self.assertEqual(result, [0, 2, 4, 6, 8])
    
    def test_pipe_with_sorted(self):
        """Test piping with sorted"""
        result = [3, 1, 4, 1, 5, 9, 2, 6] |> sorted
        self.assertEqual(result, [1, 1, 2, 3, 4, 5, 6, 9])
    
    def test_pipe_with_reversed(self):
        """Test piping with reversed"""
        result = [1, 2, 3] |> reversed |> list
        self.assertEqual(result, [3, 2, 1])
    
    def test_pipe_with_enumerate(self):
        """Test piping with enumerate"""
        result = ['a', 'b', 'c'] |> enumerate |> list
        self.assertEqual(result, [(0, 'a'), (1, 'b'), (2, 'c')])
    
    def test_pipe_with_zip(self):
        """Test piping to functions that need multiple args via lambda"""
        list1 = [1, 2, 3]
        list2 = ['a', 'b', 'c']
        
        result = list1 |> (lambda x: zip(x, list2)) |> list
        self.assertEqual(result, [(1, 'a'), (2, 'b'), (3, 'c')])


if __name__ == '__main__':
    unittest.main()