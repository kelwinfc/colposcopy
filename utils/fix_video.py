import os
from os import sys

path = sys.argv[1]
for root, dirs, files in os.walk(path):
    for idx, f in enumerate(files):
        v = os.path.join(path, f).replace(' ', '\ ')
        os.system("avconv -threads 4 -i " + v + \
                  " -vcodec copy -acodec copy __0.mpg; cp __0.mpg " + v +\
                  "; rm __0.mpg")
        print "\n\n\n", str(idx + 1) + "/" + str(len(files)), v, "\n\n\n"
    break
