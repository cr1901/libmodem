#!/usr/bin/env python3

import os.path
import sys

if __name__ == "__main__":
    print(os.path.relpath(sys.argv[1], sys.argv[2]))
