import unittest
import sys
import os
import tempfile
import shutil
import subprocess

class RelativeImportTests(unittest.TestCase):
    def test_relative_imports(self):
        temp_dir = tempfile.mkdtemp()
        self.addCleanup(shutil.rmtree, temp_dir)

        # Create a package structure
        pkg_dir = os.path.join(temp_dir, 'pkg')
        subpkg_dir = os.path.join(pkg_dir, 'subpkg')
        os.makedirs(subpkg_dir)

        with open(os.path.join(pkg_dir, '__init__.py'), 'w') as f:
            f.write('')
        with open(os.path.join(pkg_dir, 'helper.py'), 'w') as f:
            f.write('def four(): return 4')
        with open(os.path.join(subpkg_dir, '__init__.py'), 'w') as f:
            f.write('')

        # Test single dot import
        test_code_single = '/ . / helper; assert helper.four() == 4'
        test_file_single = os.path.join(pkg_dir, 'test_single.py')
        with open(test_file_single, 'w') as f:
            f.write(test_code_single)
        
        env = os.environ.copy()
        env['PYTHONPATH'] = temp_dir
        result = subprocess.run([sys.executable, '-m', 'pkg.test_single'], env=env, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)

        # Test double dot import
        test_code_double = '/ .. / helper; assert helper.four() == 4'
        test_file_double = os.path.join(subpkg_dir, 'test_double.py')
        with open(test_file_double, 'w') as f:
            f.write(test_code_double)

        result = subprocess.run([sys.executable, '-m', 'pkg.subpkg.test_double'], env=env, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)
