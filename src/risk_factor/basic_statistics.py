#!/usr/bin/env python
# -*- coding: utf-8 -*-

import csv
import os
from os import sys
import numpy as np
from utils import *

header, samples = get_risk_matrix(sys.argv[1])
header, samples, target = get_target(header, samples)

for idh, h in enumerate(header):
    print idh, h

plot_histogram(header, samples, target,
               os.path.join("results", "risk", "histograms"))
