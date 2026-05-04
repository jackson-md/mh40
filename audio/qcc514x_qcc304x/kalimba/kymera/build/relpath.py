############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2021 Qualcomm Technologies International, Ltd.
#
############################################################################
"""
Python script to generate the relative paths (relative to the CWD) from an input
file containing whitespace separated absolute/relative paths. Output is
on stdio.
"""
import os
import sys

def rel_path(file_in_name):
    path_out_list = []
    num_duplicates = 0

    # Get the current working directory (need paths relative to this!)
    cur = os.getcwd()
    #sys.stderr.write("CWD = {0}\n".format(cur))

    # Read paths from input file
    with open(file_in_name, 'r') as file_in_paths:    
        # Read the whole file of space separated paths
        paths = file_in_paths.read()

    # Create a list from the paths (space separated)    
    path_in_list = paths.split()

    # Look through list and convert to relative path where possible
    for path in path_in_list:
        try:
            com = os.path.commonpath([os.path.abspath(cur), os.path.abspath(path)])        
            newpath = os.path.relpath(os.path.abspath(path), os.path.abspath(cur))

            #sys.stderr.write("Common:   {0}\n".format(com))
            #sys.stderr.write("Relative: {0}\n".format(newpath))

        except:
            # If no common path just use absolute path
            newpath = os.path.abspath(path)

            #sys.stderr.write("Absolute: {0}\n".format(newpath))

        # Don't add duplicates
        if newpath not in path_out_list:
            path_out_list.append(newpath)
        else:
            num_duplicates += 1
            #sys.stderr.write("Duplicate: {0}\n".format(newpath))

    # Report number of duplicate paths
    #sys.stderr.write("Number of duplicate header search paths = {0}\n".format(num_duplicates))

    # Write paths to file for debug
    with open(file_in_name+"_result", 'w') as file_out_paths:
        for path in path_out_list:
             file_out_paths.write("{0}\n".format(path))

    # Deliver space separated output paths to stdio    
    for path in path_out_list:
         sys.stdout.write("{0} ".format(path))
     
if __name__ == '__main__':

    if sys.argv[1] == "":
        sys.stderr.write("Usage: {0} <input file of paths>\n".format(sys.argv[0]))
        exit()

    # Input file path
    file_in_name = sys.argv[1]
    
    rel_path(file_in_name)
