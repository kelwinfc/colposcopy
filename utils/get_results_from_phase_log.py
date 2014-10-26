import os
from os import sys
import numpy as np
import matplotlib.pyplot as plt
import numpy


def accuracy_per_class(global_matrix):
    acc = [0.0 for _ in xrange(len(global_matrix))]
    n = len(global_matrix)

    for label in xrange(n):
        for rc in xrange(n):
            if rc == label:
                continue
            acc[label] += global_matrix[rc][label]
            acc[label] += global_matrix[label][rc]

    return [1.0 - acc[label] for label in xrange(n)]


def precision_per_class(global_matrix):
    tp_fp = [0.0 for _ in xrange(len(global_matrix))]
    n = len(global_matrix)

    for idx, x in enumerate(global_matrix):
        for idy, y in enumerate(x):
            tp_fp[idy] += y

    return [global_matrix[label][label] / tp_fp[label] for label in xrange(n)]


def recall_per_class(global_matrix):
    tp_fn = [0.0 for _ in xrange(len(global_matrix))]
    n = len(global_matrix)

    for idx, x in enumerate(global_matrix):
        for idy, y in enumerate(x):
            tp_fn[idx] += y

    return [global_matrix[label][label] / tp_fn[label] for label in xrange(n)]


def print_array(a):
    for idv, v in enumerate(a):
        if idv != 0:
            print "&",
        print "%.4f" % v,
    print

num_videos = int(sys.argv[1])
num_algorithms = int(sys.argv[2])
lines = open(sys.argv[3]).readlines()[2 + num_videos:]
algorithm = int(sys.argv[4])

total_accuracy = 0.0
accuracies = []
times = []

global_matrix = [[0.0 for _ in xrange(5)] for _ in xrange(5)]

base_lines = 2
lines_per_algorithm = 8
shift_lines = base_lines + lines_per_algorithm * num_algorithms

nvideos = 0
for i in xrange(num_videos):
    base = shift_lines * i + base_lines
    info = lines[base + lines_per_algorithm * algorithm:
                 base + lines_per_algorithm * (algorithm + 1)]

    matrix = [filter(lambda y: len(y) > 0, x.split(' '))[2: -1] \
                for x in info[1:-2]][0:]

    #print int(info[-2].split(' ')[0])
    times.append(int(info[-2].split(' ')[0]))

    total = 0.0
    for row in matrix:
        for col in row:
            total += float(col)

    for idx, row in enumerate(matrix):
        for idy, col in enumerate(row):
            global_matrix[idx][idy] += float(col) / total
    error = float(info[-1].split(' ')[-1])

    accuracies.append(1.0 - error)
    total_accuracy += 1.0 - error

    nvideos += 1

for idx, x in enumerate(global_matrix):
    for idy, y in enumerate(x):
        global_matrix[idx][idy] /= nvideos
        #print "%.4f" % (y / nvideos),
    #print
#print

print "Accuracy ",
acc = accuracy_per_class(global_matrix)
print_array(acc)

print "Precision",
prec = precision_per_class(global_matrix)
print_array(prec)

print "Recall   ",
rec = recall_per_class(global_matrix)
print_array(rec)

plt.boxplot(accuracies, notch=False, sym='+', vert=True, whis=1.5)
#plt.show()

print
print "Accuracy", "%.4f" % numpy.mean(numpy.array(accuracies), axis=0)
print "Avg. Accuracy", "%.4f" % numpy.mean(numpy.array(acc), axis=0)
print "Avg. Precision", "%.4f" % numpy.mean(numpy.array(prec), axis=0)
print "Avg. Recall", "%.4f" % numpy.mean(numpy.array(rec), axis=0)
print "Avg. Time", sum(times)
#print "stddev", numpy.std(numpy.array(accuracies), axis=0)
