import os
from os import sys
import numpy as np
import matplotlib.pyplot as plt
import math

def total_matrix(m):
    return sum([sum(x) for x in m])

def normalize_matrix(m):
    total = total_matrix(m)
    return [[float(y) / total for y in x] for x in m]

def remove_method(m, i):
    m = [x[:i] + x[i + 1:] for x in m]
    return m[:i] + m[i + 1:]

def get_binary_matrix(m, i):
    ret = [[0.0, 0.0], [0.0, 0.0]]
    for idx, x in enumerate(m):
        for idy, y in enumerate(x):
            if idx == i:
                if idy == i:
                    ret[1][1] += y
                else:
                    ret[1][0] += y
            else:
                if idy == i:
                    ret[0][1] += y
                else:
                    ret[0][0] += y
    return ret

def accuracy(m):
    total = total_matrix(m)
    return (m[0][0] + m[1][1]) / total

def precision(m):
    tp = m[1][1]
    fp = m[0][1]
    
    if tp + fp == 0.0:
        return 1.0
    
    return tp / (tp + fp)

def recall(m):
    tp = m[1][1]
    fn = m[1][0]
    
    if tp + fn == 0.0:
        return 1.0
    
    return tp / (tp + fn)

def f_measure(m):
    p = precision(m)
    r = recall(m)
    
    if p + r == 0:
        return 1.0

    return 2.0 * p * r / (p + r  + 1e-10)

def merge(a, b, n):
    nn = 1.0 / n
    return map(lambda (x, y): x + nn * y, zip(a, b))

f = open(sys.argv[1])
lines = f.readlines()[1:]
num_alg = int(sys.argv[2])
num_videos = int(sys.argv[3])

lines_per_alg = 8
lines_per_video = 2 + lines_per_alg * num_alg
lines_per_exp = 1 + num_videos * lines_per_video
methods = ["        Macro",
           "        Green",
           "         Hins",
           "          Sch",
           "\\textbf{Avg.}",
           "                              "]

num_experiments = len(lines) / lines_per_exp
num_indexed_frames = []
results_per_experiment = []

alg_names = []
num_frames = [[] for _ in xrange(5)]
num_relative_frames = [[] for _ in xrange(5)]
sizes = []
fps_per_experiment = []

print num_experiments, "experiments"
for e in xrange(num_experiments):
    
    exp_lines = lines[lines_per_exp * e: lines_per_exp * (e + 1)]
    
    statement = lines[lines_per_exp * e]
    indexed_frames = int(statement.split(' - ')[1].split(' ')[0])
    
    num_indexed_frames.append(indexed_frames)
    
    results_per_video = []
    fps_per_video = []
    
    for v in xrange(num_videos):
        video_lines = exp_lines[1 + v * lines_per_video:
                                1 + (v + 1) * lines_per_video]
        results_per_alg = []
        
        fps_per_alg = []
        
        for alg in xrange(num_alg):
            alg_lines = video_lines[2 + alg * lines_per_alg:
                                    2 + (alg + 1) * lines_per_alg]
            
            fps_per_alg.append(float(alg_lines[-2].split(' ')[-2]))
        
            if len(alg_names) < num_alg:
                alg_names.append(alg_lines[-1].split(':')[0])
            
            matrix = [filter(lambda x: len(x) > 0,
                             x.replace('\n','').split(' '))[1:] \
                        for x in alg_lines[:-2]]
            matrix = [[int(y) for y in x] for x in matrix]
            matrix = remove_method(matrix, 0)
            
            results_per_class = []
            
            if alg == 0 and e == 0:
                total_video = 0
                for cl in xrange(5):
                    total_sum = 0
                    for cl2 in xrange(5):
                        total_sum += matrix[cl][cl2]
                    total_video += total_sum
                    num_frames[cl].append(total_sum)
                sizes.append(total_video)

            matrix = normalize_matrix(matrix)
            
            if alg == 0 and e == 0:
                for cl in xrange(5):
                    total_sum = 0
                    for cl2 in xrange(5):
                        total_sum += matrix[cl][cl2]
                    num_relative_frames[cl].append(total_sum)

            for cl in xrange(4):
                b = get_binary_matrix(matrix, cl + 1)
                b2 = get_binary_matrix(remove_method(matrix, 0), cl)
                
                results_per_class.append([accuracy(b),
                                          precision(b),
                                          recall(b),
                                          f_measure(b),
                                          accuracy(b2),
                                          precision(b2),
                                          recall(b2),
                                          f_measure(b2)
                                         ])
            results_per_alg.append(results_per_class)
        results_per_video.append(results_per_alg)
        fps_per_video.append(fps_per_alg)

    results_per_experiment.append(results_per_video)
    fps_per_experiment.append(fps_per_video)

fps = []
for idx_exp, exp in enumerate(fps_per_experiment):
    fps_per_alg = reduce(lambda pr, nw: [x + y for (x, y) in zip(pr, nw)],
                         exp, [0.0 for _ in xrange(num_alg)])
    fps_per_alg = [x / float(num_videos) for x in fps_per_alg]
    fps.append(fps_per_alg)
    print ' '.join(["%0.4f" % x for x in fps_per_alg])

fig, ax = plt.subplots()

plt.plot(num_indexed_frames,
         [x[1-1] for x in fps], 'b',
         label=alg_names[0])
plt.plot(num_indexed_frames,
         [x[3-1] for x in fps], 'r',
         label=alg_names[2])

ax.set_xticks(num_indexed_frames)
plt.legend(loc='upper right')

axis_font = {'size':'18'}
plt.xlabel('Indexed Frames (log2)', **axis_font)
plt.ylabel('fps', **axis_font)
plt.xlim([min(num_indexed_frames), max(num_indexed_frames)])

f = os.path.join("results", "phase_timeline", "fps_indexed_frames.png")
plt.savefig(f)
plt.clf()

max_length = max(map(len, alg_names))
alg_names = [x + " " * (max_length - len(x)) for x in alg_names]

accuracies = []
for id_exp, exp in enumerate(results_per_experiment):
    print id_exp, num_indexed_frames[id_exp]
    results_per_alg = [[[0.0] * 8 for cl in xrange(5)] \
                        for alg in xrange(num_alg)]
    
    for id_v, v in enumerate(exp):
        for alg in xrange(num_alg):
            for cl in xrange(4):
                results_per_alg[alg][cl] = merge(results_per_alg[alg][cl],
                                                 v[alg][cl], num_videos)

    next_accuracies = []
    for alg in xrange(num_alg):
        for cl in xrange(4):
            results_per_alg[alg][4] = merge(results_per_alg[alg][4],
                                            results_per_alg[alg][cl], 4.0)
        next_accuracies.append(results_per_alg[alg][4][4])
    accuracies.append(next_accuracies)

    for cl in xrange(5):
        for alg in xrange(num_alg):
            print ("\multirow{" + str(num_alg) + "}{*}{" + methods[cl] + "}" \
                    if alg == 0 else methods[-1]), "&",
            print alg_names[alg] + " & " + \
                  " & ".join(map(lambda x: "%0.4f" % round(x, 4),
                                 results_per_alg[alg][cl])) + "\\\\"
        print "\\hline"
    print

if num_experiments > 1:
    fig, ax = plt.subplots()

    plt.plot(num_indexed_frames,
             [x[1] for x in accuracies], 'b',
             label=alg_names[1])
    plt.plot(num_indexed_frames,
             [x[3] for x in accuracies], 'r',
             label=alg_names[3])
    ax.set_xticks(num_indexed_frames)
    #plt.axis([num_indexed_frames[0],
              #num_indexed_frames[-1],
              #0,  # min(tr_acc_ks + ts_acc_ks) - 0.01,
              #0.6])
    plt.legend(loc='lower right')

    axis_font = {'size':'18'}
    plt.xlabel('Indexed Frames (log2)', **axis_font)
    plt.ylabel('Accuracy', **axis_font)
    plt.xlim([min(num_indexed_frames), max(num_indexed_frames)])

    f = os.path.join("results", "phase_timeline", "knn_indexed_frames.png")
    plt.savefig(f)
    plt.clf()

fmean = np.mean(num_frames, axis=1)
fmin = np.min(num_frames, axis=1)
fmax = np.max(num_frames, axis=1)
frmean = np.mean(num_relative_frames, axis=1)
frmin = np.min(num_relative_frames, axis=1)
frmax = np.max(num_relative_frames, axis=1)

classes = ["Transition      ",
           "Macroscopic     ",
           "Green           ",
           "Hinselmann      ",
           "Schiller        "
          ]

print "Num frames:", np.sum(num_frames)
print
print "\\hline"
for id_cl, cl in enumerate(classes):
    print "%s & %4d & %5d & %4d & %6.2f & %5.2f\\\\" % \
        (cl,
         fmin[id_cl],
         fmax[id_cl], fmean[id_cl],
         frmax[id_cl] * 100., frmean[id_cl] * 100.)

print "\\hline"
print "\\textbf{Video}   & %4d & %5d & %4d &   --   &  --  \\\\" % \
            (np.min(sizes), np.max(sizes), np.mean(sizes))
print "\\hline"
