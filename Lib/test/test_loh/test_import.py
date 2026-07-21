import unittest

from itertools import *

class Tests(unittest.TestCase):
    def test_import(self):
        /math
        assert math.sqrt(4) == 2

    def test_from(self):
        /math/sqrt
        assert sqrt(4) == 2

    def test_as(self):
        /math => m
        assert m.sqrt(4) == 2

        /math/sqrt => s
        assert s(4) == 2
    
    def test_import_all(self):
        #intertools imported above at module level
        assert isinstance(chain(), chain)

    def test_import_multiple(self):
        / math / sqrt, floor
        assert sqrt(4) == 2
        assert floor(4.5) == 4

        / math, datetime
        assert sqrt(4) == 2 
        assert datetime.datetime.now()

        / os / (path, listdir)
        assert path
        assert listdir

    def test_import_multiple_as(self):
        / math => m, datetime => d
        assert m.sqrt(4) == 2 
        assert d.datetime.now()

    def test_import_submodule(self):
        / xml.etree.ElementTree
        assert xml.etree.ElementTree

        / xml.etree / ElementTree
        assert ElementTree

        / xml.etree / ElementTree => ET
        assert ET

    def test_loh_extension_import(self):
        import tempfile
        import sys
        import os
        import shutil
        import importlib
        import importlib.machinery

        self.assertIn('.loh', importlib.machinery.SOURCE_SUFFIXES)

        temp_dir = tempfile.mkdtemp()
        try:
            module_name = "test_loh_import_helper"
            module_file = os.path.join(temp_dir, f"{module_name}.loh")
            with open(module_file, "w") as f:
                f.write("def get_value(obj):\n    return obj~.x\n")

            sys.path.insert(0, temp_dir)
            try:
                mod = importlib.import_module(module_name)
                
                class Foo:
                    x = 123
                
                self.assertEqual(mod.get_value(Foo()), 123)
                self.assertIsNone(mod.get_value(None))
                
                if module_name in sys.modules:
                    del sys.modules[module_name]
                
                # Test using Loh import syntax
                scope = {}
                exec(f"/{module_name}\nresult = {module_name}.get_value(None)", {}, scope)
                self.assertIsNone(scope["result"])
            finally:
                sys.path.remove(temp_dir)
                if module_name in sys.modules:
                    del sys.modules[module_name]
        finally:
            shutil.rmtree(temp_dir)



    