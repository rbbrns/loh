import sys
import unittest
from test.support import import_helper

class Tests(unittest.TestCase):

    def test_break(self):
        while True:
            $>>
            ^^^ AssertionError
        else:
            ^^^ AssertionError

        for i in range(10):
            $>>
            ^^^ AssertionError
        else:
            ^^^ AssertionError

    
    def test_continue(self):
       i = 0
       while i<10:
            i += 1
            $<<
            ^^^ AssertionError
       else:
            while_else_ran = True
       self.assertTrue(while_else_ran)

       for i in range(10):
            $<<
            ^^^ AssertionError
       else:
            for_else_ran = True
       self.assertTrue(for_else_ran)

    def test_for(self):
        $ i in range(10):
            pass
        self.assertTrue(i == 9)

        $ i := range(10):
            pass
        self.assertTrue(i == 9)

        $i:=range(10):
            pass
        self.assertTrue(i == 9)
        
        $ i :=  range(10):
            pass
        self.assertTrue(i == 9)

    def test_for_else(self):
        $ i := range(10): $>>
        ?!$>>: ^^^ AssertionError

        $ i := range(10): $>>
        ?! $>>: ^^^ AssertionError

        $ i := range(10): $>>
        else: ^^^ AssertionError

        $ i := range(10): $>>
        if not break: ^^^ AssertionError

        $ i := range(10): $>>
        ? ! $>>: ^^^ AssertionError

        $ i := range(10): $>>
        ?!$>>: ^^^ AssertionError

    def test_for_comprehension(self):
        self.assertEqual([ i $ i in range(10) ], list(range(10)))
        self.assertEqual([ i $ i <~ range(10) ], list(range(10)))

        self.assertEqual([i$i in range(10)], list(range(10)))
        self.assertEqual([i$i<~range(10)], list(range(10)))

        self.assertEqual([ i $ i <~ range(10) if i < 5], list(range(5)))
        self.assertEqual([ i $ i <~ range(10) ? i < 5], list(range(5)))

        self.assertEqual([ i+1 $ i <~ range(10) ], list(range(1,11)))

        data = [(1, 2, 3), (4, 5, 6, 7)]
        first = [first for first, *rest in data]
        self.assertEqual(first, [1, 4])

        first = [first $ first, *rest <~ data]
        self.assertEqual(first, [1, 4])

    def test_for_if_comprehension(self):
        self.assertEqual([i $ i in range(10) if i < 5], list(range(5)))
        self.assertEqual([i $ i <~ range(10) ? i < 5], list(range(5)))

    def test_while(self):
        i = 0
        $? True:
            i += 1
            if i == 10:
                break
        self.assertTrue(i == 10)

        $? x:=0:
            x += 1
            if x == 10:
                i = x
                break
        self.assertTrue(i == 10)

        j = 0
        $? j < 10:
            j += 1
        self.assertTrue(j == 10)

    def test_while_else(self):
        i = 0
        $? i<10:
            i += 1
        ?! $>>:
            else_ran = True
        self.assertTrue(i == 10)
        self.assertTrue(else_ran)

        $? ++: $>>
        ?!$>>:
            ^^^ AssertionError
        
        $?++:$>>
        ?! break:
            ^^^ AssertionError

        $?++:$>>
        if not break:
            ^^^ AssertionError

        $?++:$>>
        if not $>>:
            ^^^ AssertionError

        $?++:$>>
        ?!$>>:
            ^^^ AssertionError

        $?++:$>>
        ? ! $>>:
            ^^^ AssertionError

        $?++:$>>
        ?!$>>:
            ^^^ AssertionError
