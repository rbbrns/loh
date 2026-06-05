import unittest

class Tests(unittest.TestCase):
    def test_rhs_standalone_star(self):
        # Dict star
        d = {'a': 1, 'b': 2}
        res = *d
        self.assertEqual(res, ['a', 'b'])

        # List/Tuple star
        lst = [10, 20]
        res = *lst
        self.assertEqual(res, [10, 20])
        tup = (30, 40)
        res = *tup
        self.assertEqual(res, [30, 40])

        # Generator/Iterable star
        def gen():
            yield 1
            yield 2
        res = *gen()
        self.assertEqual(res, [1, 2])

    def test_rhs_standalone_double_star(self):
        # Dict double star
        d = {'a': 1, 'b': 2}
        res = **d
        self.assertEqual(res, {'a': 1, 'b': 2})
        # Check that it returns a shallow copy
        self.assertIsNot(res, d)

        # List/Tuple double star (creates index-to-value mapping)
        lst = [10, 20]
        res = **lst
        self.assertEqual(res, {0: 10, 1: 20})
        tup = (30, 40)
        res = **tup
        self.assertEqual(res, {0: 30, 1: 40})

        # Other types should raise TypeError
        with self.assertRaises(TypeError):
            res = **42

    def test_rhs_subscripted_star(self):
        d = {'a': 1, 'b': 2, 'c': 3}
        # Tuple index lookup
        res = *d['a', 'c']
        self.assertEqual(res, [1, 3])
        # Single index lookup
        res = *d['b']
        self.assertEqual(res, [2])

        # Sequence slice
        lst = [10, 20, 30, 40]
        res = *lst[1:3]
        self.assertEqual(res, [20, 30])
        # Sequence tuple indices
        res = *lst[0, 2]
        self.assertEqual(res, [10, 30])
        # Sequence single index
        res = *lst[1]
        self.assertEqual(res, [20])

        # Error cases
        with self.assertRaises(KeyError):
            res = *d['x']
        with self.assertRaises(IndexError):
            res = *lst[10]

    def test_rhs_subscripted_double_star(self):
        d = {'a': 1, 'b': 2, 'c': 3}
        # Dict tuple indices
        res = **d['a', 'c']
        self.assertEqual(res, {'a': 1, 'c': 3})
        # Dict single index
        res = **d['b']
        self.assertEqual(res, {'b': 2})

        # Sequence slice
        lst = [10, 20, 30, 40]
        res = **lst[1:3]
        self.assertEqual(res, {1: 20, 2: 30})
        # Sequence tuple indices
        res = **lst[0, 2]
        self.assertEqual(res, {0: 10, 2: 30})
        # Sequence single index
        res = **lst[1]
        self.assertEqual(res, {1: 20})

    def test_rhs_empty_subscript(self):
        d = {'a': 1, 'b': 2}
        # d[] -> dict_values([1, 2])
        res1 = d[]
        self.assertEqual(list(res1), [1, 2])
        # *d[] -> [1, 2]
        res2 = *d[]
        self.assertEqual(res2, [1, 2])

        lst = [10, 20, 30]
        # lst[] -> [10, 20, 30] (shallow copy)
        res3 = lst[]
        self.assertEqual(res3, [10, 20, 30])
        self.assertIsNot(res3, lst)
        # *lst[] -> [10, 20, 30]
        res4 = *lst[]
        self.assertEqual(res4, [10, 20, 30])

    def test_lhs_standalone_star(self):
        # List target
        lst = [1, 2, 3]
        *lst = [10, 20]
        self.assertEqual(lst, [10, 20])

        # Dict target (positional assignment to existing keys)
        d = {'a': 1, 'b': 2}
        *d = [10, 20]
        self.assertEqual(d, {'a': 10, 'b': 20})

        # Error case: Dict size mismatch
        with self.assertRaises(ValueError):
            *d = [100]
        with self.assertRaises(ValueError):
            *d = [100, 200, 300]

        # Error case: Invalid target type
        with self.assertRaises(TypeError):
            tup = (1, 2)
            *tup = [10, 20]

    def test_lhs_standalone_double_star(self):
        # Dict target (update/merge)
        d = {'a': 1, 'b': 2}
        **d = {'b': 20, 'c': 3}
        self.assertEqual(d, {'a': 1, 'b': 20, 'c': 3})

        # List target (key-value mapping to indices)
        lst = [10, 20, 30]
        **lst = {0: 100, 2: 300}
        self.assertEqual(lst, [100, 20, 300])

        # List target: out of bounds or invalid key
        with self.assertRaises(IndexError):
            **lst = {5: 500}
        with self.assertRaises(TypeError):
            **lst = {'a': 1}

    def test_lhs_subscripted_star(self):
        # Dict subscripted (tuple keys)
        d = {'a': 1, 'b': 2, 'c': 3}
        *d['a', 'c'] = [10, 30]
        self.assertEqual(d, {'a': 10, 'b': 2, 'c': 30})

        # Dict subscripted (single key)
        *d['b'] = [20]
        self.assertEqual(d, {'a': 10, 'b': 20, 'c': 30})

        # Dict size mismatch
        with self.assertRaises(ValueError):
            *d['a', 'b'] = [100]

        # List subscripted (slice)
        lst = [10, 20, 30, 40]
        *lst[1:3] = [200, 300]
        self.assertEqual(lst, [10, 200, 300, 40])

        # List subscripted (tuple indices)
        *lst[0, 3] = [100, 400]
        self.assertEqual(lst, [100, 200, 300, 400])

        # List subscripted (single index)
        *lst[1] = [2000]
        self.assertEqual(lst, [100, 2000, 300, 400])

    def test_lhs_subscripted_double_star(self):
        # Dict subscripted (tuple keys)
        d = {'a': 1, 'b': 2, 'c': 3}
        val = {'a': 10, 'b': 20, 'c': 30}
        **d['a', 'c'] = val
        self.assertEqual(d, {'a': 10, 'b': 2, 'c': 30})

        # Dict subscripted (single key)
        **d['b'] = {'b': 20}
        self.assertEqual(d, {'a': 10, 'b': 20, 'c': 30})

        # List subscripted (slice)
        lst = [1, 2, 3, 4]
        val_list = {1: 20, 2: 30}
        **lst[1:3] = val_list
        self.assertEqual(lst, [1, 20, 30, 4])

        # List subscripted (tuple indices)
        **lst[0, 3] = {0: 10, 3: 40}
        self.assertEqual(lst, [10, 20, 30, 40])

        # List subscripted (single index)
        **lst[1] = {1: 200}
        self.assertEqual(lst, [10, 200, 30, 40])
