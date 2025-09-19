import asyncio
import time
import unittest
from test.support import import_helper

class Tests(unittest.TestCase):
    def test_async_def(self):
        % def foo():
            pass

    def test_await(self):
        
        % def foo():
            % asyncio.sleep(.1)
        
        asyncio.run(foo())

    def test_async_for(self):
        % def async_generator():
            for i in range(5):
                await asyncio.sleep(.1) # Simulate an async operation
                yield i
        
        seq = []
        % def foo():
            % for item in async_generator():
                seq.append(item)
        asyncio.run(foo())
        self.assertEqual(seq, list(range(5)))


        def test_async_with():
            % def foo():
                % with asyncio.Lock():
                    lock_acquired = True

            asyncio.run(foo())
            self.assertTrue(lock_acquired)
        
