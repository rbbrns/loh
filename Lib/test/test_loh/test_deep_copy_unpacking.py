import unittest

class TestDeepCopyUnpacking(unittest.TestCase):
    def test_list_deep_copy_unpacking(self):
        orig = [[1, 2], [3, 4]]
        copy_list = [***orig]
        
        self.assertEqual(copy_list, [[1, 2], [3, 4]])
        
        # Modify copy to ensure orig is unaffected
        copy_list[0].append(99)
        self.assertEqual(orig, [[1, 2], [3, 4]])
        self.assertEqual(copy_list, [[1, 2, 99], [3, 4]])

    def test_dict_deep_copy_unpacking(self):
        orig = {"meta": {"user": "alice"}, "scores": [10, 20]}
        copy_dict = {***orig}

        self.assertEqual(copy_dict, {"meta": {"user": "alice"}, "scores": [10, 20]})

        # Modify copy to ensure orig is unaffected
        copy_dict["meta"]["user"] = "bob"
        copy_dict["scores"].append(30)
        self.assertEqual(orig, {"meta": {"user": "alice"}, "scores": [10, 20]})
        self.assertEqual(copy_dict, {"meta": {"user": "bob"}, "scores": [10, 20, 30]})

    def test_set_deep_copy_unpacking(self):
        orig = ((1, 2), (3, 4))
        copy_set = {'first', ***orig}

        self.assertEqual(copy_set, {'first', (1, 2), (3, 4)})

    def test_function_positional_deep_copy_unpacking(self):
        def mutate(a, b):
            a.append(99)
            b["val"] = 100

        orig = [[1], {"val": 2}]
        mutate(***orig)

        # Original objects must remain untouched
        self.assertEqual(orig, [[1], {"val": 2}])

if __name__ == "__main__":
    unittest.main()
