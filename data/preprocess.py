#!/usr/bin/env python3

# Copyright 2021 Wieger Wesselink.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or http://www.boost.org/LICENSE_1_0.txt)

# This script converts .csv files into .data files

import numpy as np
import os
from pathlib import Path
from prep import get_data, get_stats, standardize_data

# N.B. The functions in prep.py expect the .csv files to be in a subdirectory 'data'.
os.chdir('..')

# Create the output directory 'data/output' if it does not yet exist
outputdir = Path('data') / 'output'
outputdir.mkdir(parents=True, exist_ok=True)

for csvfile in sorted(Path('data').glob('*.csv')):
    dataset = csvfile.stem
    print('dataset', dataset)
    datafile = outputdir / (csvfile.stem + '.data')
    D, ncat = get_data(dataset)
    ncat = list(map(int, ncat.tolist()))
    ncat = [x if x != 1 else 0 for x in ncat]
    ncat = map(str, ncat)
    header = 'dataset: 1.0\ncategory_counts: {}\n'.format(' '.join(ncat))
    features = ''
    with open(csvfile) as f:
        first_line = f.readline().strip()
        if first_line.startswith('"'):
            first_line = first_line.replace('"', '')
            first_line = first_line.replace(',', ' ')
            features = 'features: ' + first_line + '\n'
    with open(datafile, "w") as f:
        f.write(header)
        if features:
            f.write(features)
        np.savetxt(f, D, fmt='%1.6g')
