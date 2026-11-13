#!/usr/bin/env python3
"""Verify app API isolation from an original-name shared library."""
import sys
import unittest
from pathlib import Path

BIN_TEST_PLATFORM = "linux"
APP_DIR = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(APP_DIR.parents[1] / "framework" / "testfw" / "bin_internal"))
from shared_namespace_probe import verify_namespace

@unittest.skipUnless(sys.platform.startswith("linux"), "[Linux] ELF namespace probe requires Linux")
class NamespaceCoexistenceTest(unittest.TestCase):
    def test_both_global_load_orders(self):
        verify_namespace(APP_DIR, 'zlib_app', 'compress', '#include <zlib.h>\n#include <string.h>\nint namespace_probe(void) { const Bytef input[]="namespace"; Bytef packed[128], output[128]; uLongf n=sizeof(packed), m=sizeof(output); if(zlib_app_compress(packed,&n,input,sizeof(input))!=Z_OK) return 10; if(zlib_app_uncompress(output,&m,packed,n)!=Z_OK) return 11; return m!=sizeof(input) || memcmp(input,output,m)!=0; }\n')

if __name__ == "__main__":
    unittest.main()
