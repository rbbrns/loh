import unittest

class TestImplicitDictUnpacking(unittest.TestCase):
    def test_bare_dict_unpacking(self):
        res = {"a": 1, {"b": 2, "c": 3}, "d": 4}
        self.assertEqual(res, {"a": 1, "b": 2, "c": 3, "d": 4})

    def test_bare_dictcomp_unpacking(self):
        res = {"env": "prod", {f"node_{i}": f"http://10.0.0.{i}" for i in range(1, 4)}}
        self.assertEqual(res, {
            "env": "prod",
            "node_1": "http://10.0.0.1",
            "node_2": "http://10.0.0.2",
            "node_3": "http://10.0.0.3"
        })

    def test_multiple_bare_dicts_and_dictcomps(self):
        res = {
            {"x": 10},
            {"y": 20},
            {f"k_{i}": i * 2 for i in range(2)}
        }
        self.assertEqual(res, {"x": 10, "y": 20, "k_0": 0, "k_1": 2})

    def test_extension_key_separators_with_implicit_unpacking(self):
        base = {"tags": ["v1"], "port": 8000}
        patch = base | {
            "tags" +: ["v2"],
            {"port": 8443, "status": "active"}
        }
        self.assertEqual(patch, {"tags": ["v1", "v2"], "port": 8443, "status": "active"})

if __name__ == "__main__":
    unittest.main()
