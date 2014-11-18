from os import sys
from utils import *

header, samples = get_risk_matrix(sys.argv[1])
header, samples, target = get_target(header, samples)

target = [0 if x == 0.0 else 1 for x in target]

print "@RELATION company"
for ida, attr in enumerate(header):
    print "@ATTRIBUTE", str(ida) + "_" + attr, "NUMERIC"
print "@ATTRIBUTE class {0, 1}"

print "@DATA"
for s, l in zip(samples, target):
    print ','.join(map(lambda x: "?" if x is None else str(x), s)) +\
          ',' + str(int(l))
