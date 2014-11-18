import os
from os import sys
import numpy as np
import matplotlib.pyplot as plt

f = open(sys.argv[1])
lines = f.readlines()[1:]
num_k = int(sys.argv[2])
lines_per_exp = 1 + 3 * num_k + 1
n = len(lines) / (1 + 3 * num_k + 1)

tests = []
for l in xrange(n):
    tests.append({'lines': lines[lines_per_exp * l: lines_per_exp * (l + 1)]})

for idt in xrange(n):
    t = tests[idt]
    t['accuracies'] = []
    t['precision'] = []
    t['recall'] = []
    t['f'] = []
    t['wprecision'] = []
    t['wrecall'] = []
    t['wf'] = []
    
    for idk in xrange(num_k):
        cm = t['lines'][1 + 3 * idk + 1: 1 + 3 * idk + 3]
        cm = [filter(lambda y: len(y) > 0, x.split(' '))[1: -1] for x in cm]
        cm = [map(float, x) for x in cm]
        
        num = [cm[0][0] + cm[0][1], cm[1][0] + cm[1][1]]
        
        tp = cm[1][1]
        tn = cm[0][0]
        fp = cm[0][1]
        fn = cm[1][0]
        
        accuracy = tp + tn
        p = [tn / (tn + fn + 1e-6), tp / (tp + fp + 1e-6)]
        r = [tn / (tn + fp + 1e-6), tp / (tp + fn + 1e-6)]
        f = [2 * p[i] * r[i] / (p[i] + r[i] + 1e-6) for i in range(2)]
        
        wp = num[0] * p[0] + num[1] * p[1]
        wr = num[0] * r[0] + num[1] * r[1]
        wf = num[0] * f[0] + num[1] * f[1]

        t['accuracies'].append(accuracy)
        
        t['precision'].append(p)
        t['recall'].append(r)
        t['f'].append(f)
        
        t['wprecision'].append(wp)
        t['wrecall'].append(wr)
        t['wf'].append(wf)
    
accuracies = []
precisions = []
recalls = []
fs = []
wprecisions = []
wrecalls = []
wfs = []

for idt in xrange(n):
    new_tests = tests[:idt] + tests[idt + 1:]

    acc_k = [0.0] * num_k

    for k in xrange(num_k):
        for i in xrange(n - 1):
            acc_k[k] += new_tests[i]['accuracies'][k]
        acc_k[k] /= n - 1

    max_v = max(acc_k)
    best_k = acc_k.index(max_v)
    
    accuracies.append(tests[idt]['accuracies'][best_k])
    precisions.append(tests[idt]['precision'][best_k])
    recalls.append(tests[idt]['recall'][best_k])
    fs.append(tests[idt]['f'][best_k])
    wprecisions.append(tests[idt]['wprecision'][best_k])
    wrecalls.append(tests[idt]['wrecall'][best_k])
    wfs.append(tests[idt]['wf'][best_k])

acc = np.mean(accuracies)
p = np.mean(precisions, axis=0)
r = np.mean(recalls, axis=0)
f = np.mean(fs, axis=0)
wp = np.mean(wprecisions, axis=0)
wr = np.mean(wrecalls, axis=0)
wf = np.mean(wfs, axis=0)

print "Accuracy:", acc
print "\\begin{tabular}{cccc}"
print "\\hline"
print " & ".join(["\\textbf{" + x + "}" for x in \
                        ["Class","Precision","Recall","F-measure"]]) + "\\\\"
print "\\hline"
print "non-transition & %0.4f & %0.4f & %0.4f\\\\" % (p[0], r[0], f[0])
print "    transition & %0.4f & %0.4f & %0.4f\\\\" % (p[1], r[1], f[1])
print "\\hline"
print "\\textbf{Weighted Avg.} & %0.4f & %0.4f & %0.4f\\\\" % (wp, wr, wf)
print "\\hline"
print "\\end{tabular}"

ts_acc_ks = [0.0] * num_k
tr_acc_ks = [0.0] * num_k
accs = [[] for _ in xrange(num_k)]

for k in xrange(num_k):
    for idt in xrange(n):
        new_tests = tests[:idt] + tests[idt + 1:]
        
        accs[k].append(tests[idt]['accuracies'][k])
        ts_acc_ks[k] += tests[idt]['accuracies'][k]
        tr_acc_ks[k] += np.mean([s['accuracies'][k] for s in new_tests])

ts_acc_ks = [1.0 - x / n for x in ts_acc_ks]
tr_acc_ks = [1.0 - x / n for x in tr_acc_ks]

plt.figure(figsize=(8, 6), dpi=80)
#plt.plot(range(num_k), tr_acc_ks, 'r', label='Training')
plt.plot(range(num_k), ts_acc_ks, 'b', label='Test')

plt.axis([0,
          num_k - 1,
          0,  # min(tr_acc_ks + ts_acc_ks) - 0.01,
          0.6])

axis_font = {'size':'18'}
plt.xlabel('K - Neighborhood size', **axis_font)
plt.ylabel('Error rate', **axis_font)

f = os.path.join("results", "phase_timeline", "motion_K.png")
plt.savefig(f)
plt.clf()


accs = [[1.0 - y for y in x] for x in accs]
plt.boxplot(accs, notch=False, sym='b+',
            vert=True, whis=1.5,
            positions=None, widths=None, patch_artist=False,
            bootstrap=None, usermedians=None, conf_intervals=None)
plt.xticks(range(num_k + 1), [""] + map(str, range(num_k + 1)))
plt.xlabel('K - Neighborhood size', **axis_font)
plt.ylabel('Error rate', **axis_font)
f = os.path.join("results", "phase_timeline", "motion_K_boxplots.png")
plt.savefig(f)
plt.clf()