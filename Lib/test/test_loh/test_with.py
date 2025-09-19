import unittest
import asyncio

class Tests(unittest.TestCase):
    class WithMock:
        def __init__(self):
            self.entered = False
            self.exited = False

        def __enter__(self):
            self.entered = True
            return self

        def __exit__(self, exc_type, exc_val, exc_tb):
            self.exited = True

    class AsyncWithMock:
        def __init__(self):
            self.entered = False
            self.exited = False

        % def __aenter__(self):
            self.entered = True
            return self

        % def __aexit__(self, exc_type, exc_val, exc_tb):
            self.exited = True
    
    def test_with(self):
        with self.WithMock() as mock:
            assert mock.entered
            assert not mock.exited
        assert mock.exited

        & self.WithMock() as mock:
            assert mock.entered
            assert not mock.exited
        assert mock.exited

        % def foo():
            % & self.AsyncWithMock() => mock:
                assert mock.entered
                assert not mock.exited
            assert mock.exited

            %&self.AsyncWithMock()=>mock:
                assert mock.entered
                assert not mock.exited
            assert mock.exited
        asyncio.run(foo())


if __name__ == '__main__':
    unittest.main()
