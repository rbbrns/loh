import unittest

class TestWalrusLoopsComprehensions(unittest.TestCase):

    def test_list_comp_implicit_single(self):
        numbers = [1, 2, 3, 4]
        doubled = [$ * 2 := numbers]
        self.assertEqual(doubled, [2, 4, 6, 8])

    def test_list_comp_implicit_pair(self):
        prices = [10, 20, 30]
        quantities = [2, 3, 4]
        totals = [$ * $$ := zip(prices, quantities)]
        self.assertEqual(totals, [20, 60, 120])

    def test_list_comp_enumerate(self):
        names = ["Alice", "Bob"]
        labels = [f"#{$ + 1}: {$$}" := enumerate(names)]
        self.assertEqual(labels, ["#1: Alice", "#2: Bob"])

    def test_dict_comp_invert(self):
        orig = {"a": 1, "b": 2}
        swapped = {$$: $ := orig.items()}
        self.assertEqual(swapped, {1: "a", 2: "b"})

    def test_generator_call(self):
        class Item:
            def __init__(self, price):
                self.price = price

        items = [Item(10), Item(20), Item(30)]
        total = sum($.price := items)
        self.assertEqual(total, 60)

    def test_statement_loop_pair(self):
        pairs = []
        $$ := {"x": 100, "y": 200}.items():
            pairs.append(($, $$))
        self.assertEqual(pairs, [("x", 100), ("y", 200)])

    def test_statement_loop_triple(self):
        triples = []
        $$$ := zip(["a"], [10], [True]):
            triples.append(($, $$, $$$))
        self.assertEqual(triples, [("a", 10, True)])

    def test_statement_while_walrus(self):
        vals = [3, 2, 1, 0]
        iter_vals = iter(vals)
        res = []
        $? := next(iter_vals, None):
            res.append($)
        self.assertEqual(res, [3, 2, 1])

if __name__ == "__main__":
    unittest.main()
