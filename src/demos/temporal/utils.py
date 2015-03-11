#!/usr/bin/python
# -*- coding: utf-8 -*-

import json
import os


def load_config(filename):
    f = open(filename)
    ret = json.loads(f.read())
    f.close()
    return ret


def media(config, s):
    return os.path.join(config["media"], s + ".png")
