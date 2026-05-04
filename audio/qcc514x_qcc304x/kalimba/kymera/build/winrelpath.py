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
    curdir = os.curdir
    filename = None
    index = 0
    while index <= len(sys.argv)-2 and params[index][0] == '-':
        if params[index][1] == 'r':
            index = index + 1
            curdir = params[index]
        elif params[index][1] == 'f':
            index = index + 1
            filename = params[index]
        else:
            print('Unknown option %s' % params[index])
            sys.exit(1)
        index = index + 1

    if filename:
        with open(filename, 'r') as infile:
            for line in infile:
                for path_string in line.split():
                    try:
                        output += os.path.relpath(path_string, curdir) + " "
                    except ValueError:
                        output += os.path.abspath(path_string) + " "
    else:
        for path_string in params[index:]:
            try:
                output += os.path.relpath(path_string, curdir) + " "
            except ValueError:
                output += os.path.abspath(path_string) + " "
    print(output.rstrip())
