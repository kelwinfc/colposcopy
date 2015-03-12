#!/usr/bin/python
# -*- coding: utf-8 -*-

from matplotlib import pyplot as plt
from threading import *
import cv2.cv as cv
import numpy as np
import tempfile
import datetime
import wx.calendar
import wx.grid
import shutil
import utils
import time
import cv2
import os
import wx

from custom_fields import *
from colposcopy import *
from citology import *
from biopsy import *
from export import *


class DiagnosisPanel(wx.Panel):
    def __init__(self, parent=None,
                 config_file="src/demos/temporal/config.json"):
        super(DiagnosisPanel, self).__init__(parent,)

        self.config = utils.load_config(config_file)
        self.width = 0
        self.height = 0
        self.width, self.height = wx.DisplaySize()

        self.setLayout()
        self.addTooltips()
        self.bindControls()

    def setLayout(self, extra_values=[]):
        self.globalSizer = wx.BoxSizer(wx.VERTICAL)
        self.sizer = wx.BoxSizer(wx.HORIZONTAL)

        self.globalSizer.AddStretchSpacer(1)
        self.globalSizer.Add(self.sizer, 0, wx.ALIGN_CENTER, 0)
        self.globalSizer.AddStretchSpacer(1)
        
        self.colposcopy = ColposcopyResults(self)
        self.citology = CitologyResults(self)
        self.biopsy = BiopsyResults(self)
        self.export = ExportData(self)
        
        self.leftSizer = wx.BoxSizer(wx.VERTICAL)
        self.leftSizer.Add(self.colposcopy, 0, wx.TOP, 0)
        self.leftSizer.Add(self.citology, 0, wx.TOP, 30)
        self.leftSizer.Add(self.biopsy, 0, wx.TOP, 30)

        self.rightSizer = wx.BoxSizer(wx.VERTICAL)
        self.rightSizer.Add(self.export, 0, wx.LEFT, 10)

        self.sizer.Add(self.leftSizer, 0)
        self.sizer.Add(self.rightSizer, 0, wx.LEFT, 30)

        self.SetSizer(self.globalSizer)

    def addTooltips(self):
        pass

    def bindControls(self):
        pass
