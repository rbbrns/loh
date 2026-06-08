import unittest

class Tests(unittest.TestCase):
    def test_value_presence_postfix(self):
        # x !~ checks value inequality with None (x != None)
        x = 42
        self.assertTrue(x !~)
        
        x = None
        self.assertFalse(x !~)

        x = [1, 2]
        self.assertTrue(x !~)

    def test_identity_presence_postfix(self):
        # x !~~ checks identity inequality with None (x is not None)
        x = 42
        self.assertTrue(x !~~)
        
        x = None
        self.assertFalse(x !~~)

        x = [1, 2]
        self.assertTrue(x !~~)

    def test_difference_between_value_and_identity(self):
        # A class that overrides __eq__ to equal None
        class EqualNone:
            def __eq__(self, other):
                return other is None

        obj = EqualNone()

        # obj == None is True, so obj != None (obj !~) is False
        self.assertFalse(obj !~)

        # obj is not None (obj !~~) is True
        self.assertTrue(obj !~~)
