import scipy as sp
import sklearn as skl
from sklearn.naive_bayes import GaussianNB
from sklearn import cross_validation, svm, datasets, neighbors
from sklearn.decomposition import PCA
import copy
import matplotlib.pyplot as plt
import random
from os import sys
import numpy as np
from sklearn.ensemble import RandomForestClassifier

def split_feedback(videos, feedback, v):
    training = []
    test = []
    
    for f in feedback:
        if videos[f[0]] == v or videos[f[1]] == v:
            test.append(f)
        else:
            training.append(f)
    
    return training, test

def features_to_comp_features(a, b):
    return [1.0 if ax < bx else 0.0 for (ax, bx) in zip(a, b)]

def feedback_to_features(samples, feedback):
    features = []
    labels = []
    
    for f in feedback:
        s0 = samples[f[0]]
        s1 = samples[f[1]]
        features.append(features_to_comp_features(s0, s1))
        features.append(features_to_comp_features(s1, s0))
        labels.append(1)
        labels.append(0)
    return features, labels

lines = open(sys.argv[1]).readlines()

num_samples = int(lines[0])
samples = [x.replace('\n','').split(',') for x in  lines[1:num_samples + 1]]
print "Num samples", num_samples
videos = map(lambda x: int(x[0]), samples)
samples = [map(float, x[1:]) for x in samples]

sts_by_video = {}
for v in videos:
    sts_by_video[v] = sts_by_video.get(v, 0) + 1
    
frames_per_video = [sts_by_video[k] for k in sts_by_video]
print "Frames per video",\
      np.mean(frames_per_video), np.std(frames_per_video)

feedback = int(lines[num_samples + 1])
print "Feedback", feedback

feedback = [map(int, x.replace('\n','').split(' '))\
            for x in lines[num_samples + 2:]]

feedback_per_video = []

avg_accuracy = 0.0

useful_features = [2, 4, 9, 11, 25, 30, 38, 42, 45, 58, 65, 66]
samples = [zip(xrange(len(x)), x) for x in samples]
samples = [filter(lambda (x, s): x in useful_features, n) for n in samples]
samples = [map(lambda x: x[1], n) for n in samples]

for idv, v in enumerate(sts_by_video):
    training, test = split_feedback(videos, feedback, v)
    tr_features, tr_labels = feedback_to_features(samples, training)
    ts_features, ts_labels = feedback_to_features(samples, test)
    
    feedback_per_video.append(len(test))
    
    svm_cl = svm.SVC(C=20.0, kernel='rbf', degree=1, gamma=0.00, coef0=0.0,
                 shrinking=True, probability=True, tol=0.01, cache_size=200,
                 verbose=False, max_iter=-1,
                 random_state=None)
    
    rf_cl = RandomForestClassifier(n_estimators=100, criterion='gini',
                                   max_depth=None, min_samples_split=2,
                                   min_samples_leaf=1, max_features='auto')
    
    conf_matrix = [[0, 0], [0, 0]]
    svm_cl.fit(tr_features, tr_labels)
    for p, t in zip(svm_cl.predict(ts_features), ts_labels):
        conf_matrix[1 if t > 0.0 else 0][1 if p > 0.0 else 0] += 1
    next_acc = (conf_matrix[0][0] + conf_matrix[1][1]) / float(len(ts_labels))
    avg_accuracy += next_acc
    print "%0.4f %0.4f" % (next_acc, avg_accuracy / (idv + 1))

print "Feedback per video",\
      np.mean(feedback_per_video), np.std(feedback_per_video)
