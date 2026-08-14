import unittest

class TestMatchUnified(unittest.TestCase):

    def test_single_pattern_if(self):
        res = []
        val = {"status": 200, "data": 42}

        ? val => {"status": 200, "data": d}:
            res.append(d)

        self.assertEqual(res, [42])

    def test_single_pattern_if_else(self):
        val = {"status": 404}
        msg = "none"

        ? val => {"status": 200, "data": d}:
            msg = f"ok:{d}"
        ??:
            msg = "error"

        self.assertEqual(msg, "error")

    def test_multi_line_statement_block(self):
        status = 404
        msg = ""

        status =>:
            200:
                msg = "Success"
            404:
                msg = "Not Found"
            _:
                msg = "Error"

        self.assertEqual(msg, "Not Found")

    def test_subject_match_expression(self):
        status = 200
        msg = status => (
            200 -> "Success",
            404 -> "Not Found",
            _   -> "Error"
        )
        self.assertEqual(msg, "Success")

        val = 404
        msg2 = val => (
            200 -> "Success",
            404 -> "Not Found",
            _   -> "Error"
        )
        self.assertEqual(msg2, "Not Found")

if __name__ == "__main__":
    unittest.main()
