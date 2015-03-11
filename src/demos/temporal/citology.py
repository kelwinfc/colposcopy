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


class CitologyResults(wx.Panel):
    def __init__(self, parent=None,
                 config_file="src/demos/temporal/config.json"):
        super(CitologyResults, self).__init__(parent,)

        self.config = utils.load_config(config_file)
        self.width = 0
        self.height = 0
        self.width, self.height = wx.DisplaySize()
        self.parent = parent

        self.setLayout()
        self.addTooltips()
        self.bindControls()

    def setLayout(self, extra_values=[]):
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        self.top = wx.BoxSizer(wx.VERTICAL)
        self.lfirst = wx.BoxSizer(wx.HORIZONTAL)

        self.title = wx.StaticText(self, wx.ID_ANY, label="Citology")
        font = wx.Font(pointSize=12, family=wx.FONTFAMILY_DECORATIVE,
                       style=wx.NORMAL, weight=wx.FONTWEIGHT_BOLD)
        self.title.SetFont(font)

        self.top.Add(self.title, 0)
        self.top.Add(wx.StaticLine(self, wx.ID_ANY,
                                   size=(0.56 * self.width, -1)), 0)

        self.resultSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.resultLabel = wx.StaticText(self, label="Transformation Zone")
        self.resultChoice = \
            wx.Choice(self, wx.ID_ANY,
                      choices=["Presence",
                               "Absence"])

        self.resultSizer.Add(self.resultLabel, 0)
        self.resultSizer.Add(self.resultChoice, 0, wx.LEFT, 5)

        self.sampleLabel = wx.StaticText(self, label="Sample status")
        self.sampleChoice = \
            wx.Choice(self, wx.ID_ANY,
                      choices=["Satisfactory",
                               "Unsatisfactory - insufficient",
                               "Unsatisfactory - inflammatory",
                               "Unsatisfactory - hematic"])

        self.resultSizer.Add(self.sampleLabel, 0, wx.LEFT, 10)
        self.resultSizer.Add(self.sampleChoice, 0, wx.LEFT, 5)
        self.top.Add(self.resultSizer, 0, wx.TOP, 10)

        self.noEvidence = wx.RadioButton(self, label="No evidence",
                                         style=wx.RB_GROUP)
        self.lowGrade = wx.RadioButton(self, label="Low grade lesion")
        self.highGrade = wx.RadioButton(self, label="High grade lesion")
        self.carcinoma = wx.RadioButton(self, label="Invasive carcinoma")
        self.ascus = wx.RadioButton(self, label="ASCUS")
        self.asgus = wx.RadioButton(self, label="ASGUS")
        self.squamous = wx.RadioButton(self, label="Atypical squamous cells")
        self.asch = wx.RadioButton(self, label="ASC-H")
        self.glandular = wx.RadioButton(self, label="Atypical glandular cells")
        
        self.selectionSizer = wx.GridSizer(3, 3, 0, 10)
        
        for x in [self.noEvidence, self.lowGrade, self.highGrade,
                  self.carcinoma, self.ascus, self.asgus, self.squamous,
                  self.asch, self.glandular]:
            self.selectionSizer.Add(x, 0)
        
        self.top.Add(self.selectionSizer, 0, wx.TOP, 10)
        
        self.sizer.Add(self.top, 0, wx.LEFT, 0)

        self.SetSizer(self.sizer)

    def addTooltips(self):
        pass

    def bindControls(self):
        pass
