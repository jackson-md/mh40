############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
"""
Convert a list of filenames to local format

Typically needed when passing make targets etc to Windows tools
The Kymera build system invokes this via the "ospath" function
"""

import os
import sys

if __name__ == '__main__':
    params = sys.argv[1:]
    output = ""
    if params[0] == '-f':
        with open(params[1], 'r') as infile:
            for line in infile:
                for path_string in line.split():
                    output += os.path.normpath(path_string) + " "
    else:
        for path_string in params:
            output += os.path.normpath(path_string) + " "
    print(output.rstrip())
