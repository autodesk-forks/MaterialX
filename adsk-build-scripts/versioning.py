import re
import os
import sys
import json
import requests
import argparse
import tempfile
from requests.auth import HTTPBasicAuth

VERSION_FILE_DIR = ""

def increment_one_version(filename, key):
    with open(filename, 'rU') as fin, tempfile.NamedTemporaryFile('w', dir=os.path.dirname(filename), delete=False) as fout:
        for line in fin.readlines():
            if line.startswith(key):
                prevversion = line.split('=')[1].split('.')
                newversion = prevversion[0] + "." + prevversion[1] + "." +  str(int(prevversion[2])+1)
                line = '='.join((line.split('=')[0], '{}'.format(newversion))) + "\n"
            fout.write(line)
        fin.close()
        fout.close()
    # remove old version
    os.unlink(filename)
    # rename new version
    os.rename(fout.name, filename)
    return
    
def increase_component_version():
    increment_one_version("adsk-contrib\\" + os.environ['branch'] + ".properties", "version")
    return 
                                
if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-u",
                        "--update",
                        action="store_true",
                        help="Increase version in branch properties file.",
                        default=False)                  
    parser.add_argument("-b",
                        "--branch",
                        nargs=1,
                        help="Specify branch name for properties file",
                        required=True)
    parsed_args = parser.parse_args()
    
    os.environ['branch'] = parsed_args.branch[0]    
    
    if ("release" in os.environ['branch']):
        increase_component_version()
