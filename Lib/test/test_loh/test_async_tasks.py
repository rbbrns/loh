import unittest
import asyncio

class TestAsyncTasks(unittest.TestCase):
    def test_async_fn_declaration_and_await(self):
        code = """
import asyncio

% add(a, b):
    -> a + b

% main():
    t = % add(5, 7)
    res = %% t
    -> res

res = asyncio.run(main())
"""
        scope = {}
        exec(code, scope)
        self.assertEqual(scope["res"], 12)

    def test_multi_task_gather_list(self):
        code = """
import asyncio

% task1():
    await asyncio.sleep(0.01)
    -> 10

% task2():
    await asyncio.sleep(0.01)
    -> 20

% main():
    t1 = % task1()
    t2 = % task2()
    r1, r2 = %% [t1, t2]
    -> r1 + r2

res = asyncio.run(main())
"""
        scope = {}
        exec(code, scope)
        self.assertEqual(scope["res"], 30)

    def test_multi_task_gather_starred(self):
        code = """
import asyncio

% worker(x):
    await asyncio.sleep(0.01)
    -> x * 2

% main():
    tasks = [% worker(1), % worker(2), % worker(3)]
    results = %% *tasks
    -> results

res = asyncio.run(main())
"""
        scope = {}
        exec(code, scope)
        self.assertEqual(scope["res"], [2, 4, 6])

    def test_async_for_loop(self):
        code = """
import asyncio

class AsyncGen:
    def __init__(self):
        self.items = [1, 2, 3]
    def __aiter__(self):
        return self
    async def __anext__(self):
        if not self.items:
            raise StopAsyncIteration
        await asyncio.sleep(0.001)
        return self.items.pop(0)

% main():
    acc = []
    % $ := AsyncGen():
        acc.append($)
    -> acc

res = asyncio.run(main())
"""
        scope = {}
        exec(code, scope)
        self.assertEqual(scope["res"], [1, 2, 3])

    def test_async_with_statement(self):
        code = """
import asyncio

class AsyncLockContext:
    def __init__(self):
        self.entered = False
        self.exited = False
    async def __aenter__(self):
        self.entered = True
        return self
    async def __aexit__(self, exc_type, exc_val, exc_tb):
        self.exited = True

% main():
    ctx = AsyncLockContext()
    % & ctx:
        pass
    -> (ctx.entered, ctx.exited)

res = asyncio.run(main())
"""
        scope = {}
        exec(code, scope)
        self.assertEqual(scope["res"], (True, True))

if __name__ == "__main__":
    unittest.main()
