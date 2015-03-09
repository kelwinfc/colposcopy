#!/usr/bin/python
# -*- coding: utf-8 -*-

from matplotlib import pyplot as plt
from threading import *
import cv2.cv as cv
import numpy as np
import tempfile
import wx.grid
import shutil
import utils
import time
import cv2
import os
import wx

class TextField(wx.Panel):
    def __init__(self, parent, id, label="", default="",
                 label_size=(20, -1), text_size=(20, -1), style=wx.TE_LEFT):
        wx.Panel.__init__(self, parent, id)
        
        self.txt_label = label
        self.label_size = label_size
        self.text_size = text_size
        self.style = style
        self.default = default
        self.content = None
        self.setLayout()

    def setLayout(self, extra_values=[]):
        self.sizer = wx.BoxSizer(wx.HORIZONTAL)
        self.label = wx.StaticText(self, label=self.txt_label,
                                   size=self.label_size)
        self.content = wx.TextCtrl(self, value=self.default,
                                   size=self.text_size, style=self.style)
        self.sizer.Add(self.label, 0, wx.ALL, 5)
        self.sizer.Add(self.content, 0, wx.ALL, 5)
        self.SetSizer(self.sizer)

class YesNoField(wx.Panel):
    def __init__(self, parent, id, label="", default=0, label_size=(20, -1),
                 option_size=(20, -1)):
        wx.Panel.__init__(self, parent, id)
        
        self.txt_label = label
        self.label_size = label_size
        self.option_size = option_size
        self.default = default
        self.setLayout()

    def setLayout(self, extra_values=[]):
        self.sizer = wx.BoxSizer(wx.HORIZONTAL)
        self.label = wx.StaticText(self, label=self.txt_label,
                                   size=self.label_size)

        self.yesRadio = wx.RadioButton(self, label="Yes", style = wx.RB_GROUP,
                                       size=self.option_size)
        self.noRadio = wx.RadioButton(self, label="No", size=self.option_size)

        self.sizer.Add(self.label, 0, wx.ALL, 5)
        self.sizer.Add(self.yesRadio, 0, wx.ALL, 5)
        self.sizer.Add(self.noRadio, 0, wx.ALL, 5)
        self.SetSizer(self.sizer)
        