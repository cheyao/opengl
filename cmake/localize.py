#!/usr/bin/env python3
import csv
import sys
import os

langs = []
strings = []

if len(sys.argv) != 2:
    print(f"Usage: {sys.argv[0]} [file]");

    exit(1);

with open(sys.argv[1]) as input: 
    r = csv.reader(input);
    
    langs = next(r);
    
    assert(langs[0] == "id");
    assert(langs[1] == "comment");

    for row in r:
        strings.append(row);

outDir = os.path.dirname(sys.argv[1]);

for i in range(len(langs[2:])):
    with open(outDir + langs[i] + ".json") as output:
        continue;

