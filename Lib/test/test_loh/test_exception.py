import sys
import unittest
from test.support import import_helper

class Tests(unittest.TestCase):
    def test_assert(self):
        ^?! True

        with self.assertRaises(AssertionError):
            ^?! False
        
        # assert true
        ^?! ++

        # assert not false
        ^?! ! --

    def test_raise(self):
        with self.assertRaises(ValueError):
            ^^^ ValueError

    def test_try(self):
        ~^: 
            ^^^ ValueError
            raise AssertionError 
        except ValueError:
            except_ran = True
        self.assertTrue(except_ran)
            
    def test_except(self):
        ~^: 
            ^^^ ValueError
            ^^^ AssertionError
        ?^ ValueError:
            except_ran = True
        ?^: 
            ^^^ AssertionError 
        self.assertTrue(except_ran)

        ~^: 
            ^^^ ValueError
            ^^^ AssertionError
        ? ^ ValueError:
            except_ran = True
        ? ^: 
            ^^^ AssertionError 
        self.assertTrue(except_ran)

        ~^: 
            try_ran = True
        ?^ValueError:
            ^^^ AssertionError
        ?^: 
            ^^^ AssertionError 
        self.assertTrue(try_ran)

        ~^: 
            ^^^ AssertionError
        ?^ValueError:
            ^^^ AssertionError
        ?^: 
            bare_except_ran = True
        self.assertTrue(bare_except_ran)

        ~^: 
            ^^^ AssertionError
        ?^* Exception: 
            bare_except_star_ran = True
        self.assertTrue(bare_except_star_ran)

        ~^: 
            ^^^ AssertionError
        ?^ * Exception: 
            bare_except_star2_ran = True
        self.assertTrue(bare_except_star2_ran)

    def test_else(self):
        ~^:
            try_ran = True
        ?^:
            ^^^ AssertionError
        ?!^:
            else_ran = True
        self.assertTrue(try_ran)
        self.assertTrue(else_ran)

        ~^:
            ^^^ Exception
        ?^:
            except_ran = True
        ?!^:
            ^^^ AssertionError
        self.assertTrue(except_ran)

    def test_finally(self):
        seq = []
        ~^:
            seq.append('try')
        ?^:
            seq.append('except')
        ?!^:
            seq.append('else')
        ?*:
            seq.append('finally')
        self.assertEqual(seq, ['try', 'else', 'finally'])

        seq = []
        ~^:
            ^^^ Exception
            seq.append('try')
        ?^:
            seq.append('except')
        ?!^:
            seq.append('else')
        ?*:
            seq.append('finally')
        self.assertEqual(seq, ['except', 'finally'])

        seq = []
        ~^:
            seq.append('try')
        ?*:
            seq.append('finally')
        self.assertEqual(seq, ['try', 'finally'])

    def test_as(self):
        seq = []
        ~^:
            ^^^ Exception
            seq.append('try')
        ?^ Exception => e:
            seq.append('except')
            self.assertIsInstance(e, Exception)
        self.assertEqual(seq, ['except'])

        seq = []
        ~^:
            ^^^ Exception
            seq.append('try')
        ?^ Exception => e:
            seq.append('except')
            self.assertIsInstance(e, Exception)
        self.assertEqual(seq, ['except'])

    def test_raise_string(self):
        with self.assertRaisesRegex(Exception, "Test1"):
            ^^^ "Test1"

        with self.assertRaisesRegex(Exception, "Test2"):
            ^^^ "Test" "2"

        with self.assertRaisesRegex(Exception, "Test3"):
            ^^^ 'Test3'
        
        with self.assertRaisesRegex(Exception, "Test4"):
            ^^^ f'Test{4}'

        with self.assertRaisesRegex(Exception, "Test5"):
            ^^^ str('Test5')

        with self.assertRaisesRegex(BaseException, "exceptions must derive from BaseException"):
            ^^^ 100

    def test_raise_string_from(self):
        with self.assertRaisesRegex(Exception, "Level 2") as cm:
            try:
                ^^^ "Level 1"
            except Exception as e:
                assert("Level 1" in str(e))
                ^^^ "Level 2" from e




if __name__ == "__main__":
    unittest.main()
