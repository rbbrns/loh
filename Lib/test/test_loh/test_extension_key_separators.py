import unittest
import pickle
import operator

class TestExtensionKeySeparators(unittest.TestCase):
    def test_append_separator(self):
        patch = {"tags" +: ["v2"], "count" +: 5}
        self.assertEqual(repr(patch), "{'tags': <+ ['v2']>, 'count': <+ 5>}")

        base = {"tags": ["v1"], "count": 10}
        res = operator.or_(base, patch)
        self.assertEqual(res, {"tags": ["v1", "v2"], "count": 15})

    def test_union_separator(self):
        patch = {"headers" |: {"Authorization": "Bearer token"}, "nums" |: {3, 4}}
        base = {"headers": {"Accept": "application/json"}, "nums": {1, 2}}
        res = operator.or_(base, patch)
        self.assertEqual(res, {
            "headers": {"Accept": "application/json", "Authorization": "Bearer token"},
            "nums": {1, 2, 3, 4}
        })

    def test_fallback_separator(self):
        patch = {"timeout" ?: 60, "max_retries" ?: 3, "null_key" ?: "default"}
        base = {"timeout": 30, "null_key": None}
        res = operator.or_(base, patch)
        self.assertEqual(res, {
            "timeout": 30,
            "max_retries": 3,
            "null_key": "default"
        })

    def test_delete_separator(self):
        patch = {"legacy_mode": <>, "deprecated_key": <>}
        base = {"host": "localhost", "legacy_mode": True, "active": 1}
        res = operator.or_(base, patch)
        self.assertEqual(res, {
            "host": "localhost",
            "active": 1
        })

    def test_in_place_update(self):
        base = {"tags": ["v1"], "legacy": True}
        patch = {"tags" +: ["v2"], "legacy": <>}
        base.update(patch)
        self.assertEqual(base, {"tags": ["v1", "v2"]})

    def test_pickling_patch(self):
        patch = {"tags" +: ["v2"], "legacy": <>}
        data = pickle.dumps(patch)
        restored = pickle.loads(data)
        
        base = {"tags": ["v1"], "legacy": True}
        res = operator.or_(base, restored)
        self.assertEqual(res, {"tags": ["v1", "v2"]})

if __name__ == "__main__":
    unittest.main()
