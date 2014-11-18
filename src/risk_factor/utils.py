#!/usr/bin/env python
# -*- coding: utf-8 -*-

import csv
import os
from os import sys
import numpy as np
import math
from matplotlib import pyplot as plt


def unicode_csv_reader(data, dialect=csv.excel, **kwargs):
    csv_reader = csv.reader(data, dialect=dialect, **kwargs)
    for row in csv_reader:
        yield [unicode(cell, 'utf-8') for cell in row]

def get_known_data(s):
    return filter(lambda x: x is not None, s)

def get_risk_matrix(filename):
    def noop(v):
        return v

    def get_int(v, default=None):
        if v.isnumeric():
            return int(v)
        else:
            return default

    def get_float(v, default=None):
        try:
            return float(v.replace('"',''))
        except ValueError:
            return default
    
    def not_zero(v):
        if len(v) == 0 or v[0] == '0':
            return 0.0
        else:
            return 1.0
    
    def has_value(v):
        if len(v) == 0:
            return 0.0
        else:
            return 1.0

    csvfile = open(filename, 'rb')
    samples = list(unicode_csv_reader(csvfile, delimiter=',', quotechar='|'))

    header = [x.replace('"','') for x in samples[0]]
    header[0] = u"id"
    
    samples = samples[1:]

    processors = [get_int] * 6 +\
                 [lambda x: get_float(x, 0.0)] * 2 +\
                 [get_int] +\
                 [lambda x: get_float(x, 0.0)] +\
                 [get_int, get_float, not_zero, has_value] +\
                 [noop] * 8

    samples = map(lambda x: [p(x) for p, x in zip(processors, x)], samples)
    return header, samples

def get_target(header, samples):
    def pr_schiller(x):
        if x == u'"Schiller negativo"' or len(x) == 0:
            return 0.0
        return 1.0
    
    def pr_colposcopy(x):
        if x in [u'"Negativa"', u'"Sin alteraciones"'] or len(x) == 0:
            return 0.0
        return 1.0
    
    def pr_citology(x):
        return pr_colposcopy(x)
    
    def pr_biopsy(x):
        if x == '"Sin alteraciones"' or len(x) == 0:
            return 0.0
        return 1.0

    processors = [pr_schiller, pr_colposcopy, pr_citology, pr_biopsy]
    target = [x[-5:-1] for x in samples]
    target = map(lambda x: [p(x) for p, x in zip(processors, x)], target)
    target = map(lambda x: 1.0 if sum(x) > 0 else 0.0, target)
    samples = [x[:-5] for x in samples]
    header = header[:-5]
    
    return header, samples, target

def plot_histogram(attributes, samples, labels, path):
    pos, neg = split_data(samples, labels)
    
    for idx in xrange(len(attributes)):
        pos_idx = get_known_data([s[idx] for s in pos])
        neg_idx = get_known_data([s[idx] for s in neg])
        all_idx = pos_idx + neg_idx

        IQR = all_idx
        IQR.sort()
        IQR = IQR[int(0.75 * len(IQR))] - IQR[int(0.25 * len(IQR))]

        n = len(samples)
        h = 3.5 * np.std(all_idx) / (n ** (1.0/3.0))
        #h = 2.0 * IQR / (n ** (1.0 / 3.0))

        if h == 0:
            h = 1

        num_bins = (max(all_idx) - min(all_idx)) / h
        num_bins = math.ceil(num_bins)
        
        bins = np.linspace(min(all_idx), max(all_idx), num_bins)

        plt.title(attributes[idx])
        plt.hist(neg_idx, bins, alpha=0.5, label='-1', color='r', normed=True)
        plt.hist(pos_idx, bins, alpha=0.5, label='+1', color='b', normed=True)
        plt.legend(loc='upper right')

        f = os.path.join(path, str(idx) + "_" + attributes[idx] + '.png')
        plt.savefig(f)
        plt.clf()

def split_data(samples, labels):
    z = zip(samples, labels)
    pos = [x[0] for x in filter(lambda (s, l): l >= 0.5, z)]
    neg = [x[0] for x in filter(lambda (s, l): l < 1.0, z)]
    return pos, neg
