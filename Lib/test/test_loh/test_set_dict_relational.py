import unittest

class TestSetDictRelational(unittest.TestCase):
    def test_set_relational_with_list(self):
        self.assertTrue({'a', 'b'} <= ['a', 'b', 'c'])
        self.assertFalse({'a', 'z'} <= ['a', 'b', 'c'])
        self.assertTrue({'a', 'b'} < ['a', 'b', 'c'])
        self.assertFalse({'a', 'b', 'c'} < ['a', 'b', 'c'])

    def test_set_relational_with_dict(self):
        d = {"env": "prod", "port": 8080}
        self.assertTrue({'env'} <= d)
        self.assertTrue({'env', 'port'} <= d)
        self.assertFalse({'env', 'missing'} <= d)

    def test_dict_submapping_containment(self):
        target = {"env": "prod", "port": 8080, "status": "active"}
        
        # Sub-mapping checks
        self.assertTrue({"port": 8080} <= target)
        self.assertTrue({"env": "prod", "port": 8080} <= target)
        self.assertFalse({"port": 9000} <= target)
        self.assertFalse({"missing": 1} <= target)
        self.assertFalse({"env": "prod", "missing": 1} <= target)

        # Strict subset/superset checks
        self.assertTrue({"port": 8080} < target)
        self.assertFalse(target < target)
        self.assertTrue(target >= {"port": 8080})
        self.assertTrue(target > {"port": 8080})

if __name__ == "__main__":
    unittest.main()
