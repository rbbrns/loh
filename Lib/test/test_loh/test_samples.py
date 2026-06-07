# Test suite to automatically discover and run all samples in the LohSamples/ directory.

/unittest
/subprocess
/sys
/os

TestLohSamples:unittest.TestCase:
    .test_samples():
        # Resolve the LohSamples directory relative to this test file
        dir_path = os.path.dirname(__file__)
        samples_dir = os.path.abspath(os.path.join(dir_path, "../../../LohSamples"))
        
        files_to_test = [
            "logic.loh",
            "restaurant.loh",
            "test_restaurant.loh",
            "smart_home.loh",
            "test_smart_home.loh"
        ]
        
        $ f <~ files_to_test:
            file_path = os.path.join(samples_dir, f)
            with .subTest(file=f):
                result = subprocess.run([sys.executable, file_path], capture_output=True, text=True)
                .assertEqual(
                    result.returncode, 
                    0, 
                    f"Sample {f} failed with exit code {result.returncode}.\nStderr:\n{result.stderr}\nStdout:\n{result.stdout}"
                )

? __name__ == "__main__":
    unittest.main()
