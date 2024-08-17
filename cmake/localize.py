#!/usr/bin/env python3
import csv
import json
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
    data = {"version": 100};

    for row in strings:
        data[row[0]] = row[i+2];

    with open(outDir + "/" + langs[i+2] + ".json", "w") as output:
        json.dump(data, output);

