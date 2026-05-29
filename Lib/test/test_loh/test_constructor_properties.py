import unittest

class Tests(unittest.TestCase):
    def test_constructor_shorthand_basic(self):
        C::
            .(x, y):
                .x = x
                .y = y
        
        c = C(10, 20)
        self.assertEqual(c.x, 10)
        self.assertEqual(c.y, 20)

    def test_constructor_parameter_properties(self):
        Account::
            .(.owner, .balance, email):
                .email_domain = email.split('@')[1]

        acc = Account("Alice", 1000, "alice@example.com")
        self.assertEqual(acc.owner, "Alice")
        self.assertEqual(acc.balance, 1000)
        self.assertEqual(acc.email_domain, "example.com")

    def test_method_parameter_properties(self):
        User::
            .(.name):
                .status = "active"

            .update_status(.status, details):
                .details = details

        u = User("Alice")
        self.assertEqual(u.name, "Alice")
        self.assertEqual(u.status, "active")

        u.update_status("inactive", "logged out")
        self.assertEqual(u.status, "inactive")
        self.assertEqual(u.details, "logged out")

    def test_parameter_properties_with_annotations_and_defaults(self):
        C::
            .set_data(.val: int = 42, .flag: bool = True):
                pass

        c = C()
        c.set_data()
        self.assertEqual(c.val, 42)
        self.assertEqual(c.flag, True)

        c.set_data(100, False)
        self.assertEqual(c.val, 100)
        self.assertEqual(c.flag, False)

if __name__ == "__main__":
    unittest.main()
