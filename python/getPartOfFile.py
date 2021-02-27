#!/usr/bin/python3

import re
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--filename", "-f", type=str, required=True)
args = parser.parse_args()

file = open(args.filename, 'r')
text = file.read();
file.close()
out = re.findall(r'SRC_URI[^"]*"[^"]*"', text, re.MULTILINE)

for o in out:
	print (o)
	print ("")
