#!/usr/bin/python3

import unittest
import subprocess
import tempfile
import os
import shutil
import json

ZC_BIN = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "build", "zc"))

class TestZCCli(unittest.TestCase):
    def setUp(self):
        # Create a temporary directory for our tests to avoid messing with the user's files
        self.test_dir = tempfile.mkdtemp()
        self.original_cwd = os.getcwd()
        os.chdir(self.test_dir)

        # Override HOME to avoid reading the real global config
        self.env = os.environ.copy()
        self.env["HOME"] = self.test_dir
        # We also unset ZC_REGISTRY if we want to isolate network requests (optional)

    def tearDown(self):
        os.chdir(self.original_cwd)
        shutil.rmtree(self.test_dir)

    def run_zc(self, *args, input_data=None):
        """Helper to run the zc command and return (stdout, stderr, returncode)"""
        cmd = [ZC_BIN] + list(args)
        process = subprocess.Popen(
            cmd,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            env=self.env
        )
        stdout, stderr = process.communicate(input=input_data)
        return stdout, stderr, process.returncode

    def test_init_and_build(self):
        # 1. Test init
        os.mkdir("test_project")
        os.chdir("test_project")

        # Pass newlines to accept default values for interactive prompts
        stdout, stderr, code = self.run_zc("init", input_data="\n\n\n\n\n")
        self.assertEqual(code, 0, f"Init failed: {stderr}")
        self.assertTrue(os.path.exists("zc.json"))
        self.assertTrue(os.path.exists("src"))

        # 2. Test build
        stdout, stderr, code = self.run_zc("build")
        self.assertEqual(code, 0, f"Build failed: {stderr}")
        self.assertTrue(os.path.exists("build/test_project"), "Binary was not compiled!")

    def test_config_command(self):
        # Test modifying the global config
        stdout, stderr, code = self.run_zc("config", "editor", "nano")
        self.assertEqual(code, 0, f"Config failed: {stderr}")

        # Verify the file was created and modified
        config_path = os.path.join(self.test_dir, ".zc", "config.json")
        self.assertTrue(os.path.exists(config_path), "Global config file not created")

        with open(config_path, "r") as f:
            data = json.load(f)
            self.assertEqual(data.get("editor"), "nano")

    def test_invalid_command(self):
        stdout, stderr, code = self.run_zc("nocommand")
        self.assertNotEqual(code, 0, "Invalid command should return non-zero exit code")

if __name__ == "__main__":
    if not os.path.exists(ZC_BIN):
        print(f"Error: Could not find ZC binary at {ZC_BIN}")
        print("Please compile the project (make -C build) before running tests.")
        exit(1)
    unittest.main(verbosity=2)
