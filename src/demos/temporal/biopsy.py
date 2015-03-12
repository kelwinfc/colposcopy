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


class BiopsyResults(wx.Panel):
    def __init__(self, parent=None,
                 config_file="src/demos/temporal/config.json"):
        super(BiopsyResults, self).__init__(parent,)

        self.config = utils.load_config(config_file)
        self.width = 0
        self.height = 0
        self.width, self.height = wx.DisplaySize()
        self.parent = parent

        self.setLayout()
        self.addTooltips()
        self.bindControls()

    def setLayout(self, extra_values=[]):
        font = wx.Font(pointSize=10, family=wx.FONTFAMILY_DECORATIVE,
                       style=wx.NORMAL, weight=wx.FONTWEIGHT_BOLD)

        self.sizer = wx.BoxSizer(wx.VERTICAL)
        self.top = wx.BoxSizer(wx.HORIZONTAL)
        self.top_left = wx.BoxSizer(wx.VERTICAL)
        self.top_right = wx.BoxSizer(wx.VERTICAL)
        self.bottom = wx.BoxSizer(wx.VERTICAL)
        
        self.title = wx.StaticText(self, wx.ID_ANY, label="Biopsy")
        title_font = wx.Font(pointSize=12, family=wx.FONTFAMILY_DECORATIVE,
                             style=wx.NORMAL, weight=wx.FONTWEIGHT_BOLD)
        self.title.SetFont(title_font)

        self.sizer.Add(self.title, 0)
        self.sizer.Add(wx.StaticLine(self, wx.ID_ANY,
                                     size=(0.56 * self.width, -1)), 0)
        
        self.cervix = Cervix(self, name="biopsy")
        self.cervix.enable(True)

        self.assessedAreaLabel = wx.StaticText(self, wx.ID_ANY,
                                               label="Assessed Area")
        self.assessedAreaLabel.SetFont(font)
        self.top_left.Add(self.assessedAreaLabel)
        self.top_left.Add(self.cervix, 0)

        self.incisional = wx.CheckBox(self, label="Incisional biopsy",
                                      style=wx.RB_GROUP)
        self.excisional = wx.CheckBox(self, label="Excisional biopsy")
        self.letz = wx.CheckBox(self, label="LETZ")
        self.cone = wx.CheckBox(self, label="Cone biopsy")
        self.endocervical = wx.CheckBox(self, label="endocervical curettage")

        self.procedureSizer = wx.GridSizer(3, 2, 0, 10)

        for x in [self.incisional, self.excisional, self.letz, self.cone,
                  self.endocervical]:
            self.procedureSizer.Add(x, 0)

        self.procedureLabel = wx.StaticText(self, label="Procedure")
        self.procedureLabel.SetFont(font)

        self.top_right.Add(self.procedureLabel, 0, wx.LEFT, 0)
        self.top_right.Add(self.procedureSizer, 0, wx.LEFT, 0)

        self.resultsSizer = wx.GridSizer(2, 2, 0, 0)
        self.lesionLabel = wx.StaticText(self, label="Description")
        self.lesionResults = wx.Choice(self, wx.ID_ANY,
                                       choices=["No lesions",
                                                "Low grade lesion",
                                                "High grade lesion",
                                                "Microinvasive cancer",
                                                "Invasive carcinoma"
                                               ])
        self.resultsSizer.Add(self.lesionLabel, 0)
        self.resultsSizer.Add(self.lesionResults, 0)

        self.marginLabel = wx.StaticText(self, label="Margin")
        self.marginResults = wx.Choice(self, wx.ID_ANY,
                                       choices=["Free",
                                                "Taken"])
        self.resultsSizer.Add(self.marginLabel, 0)
        self.resultsSizer.Add(self.marginResults, 0)

        self.resultsLabel = wx.StaticText(self, label="Results")
        self.resultsLabel.SetFont(font)

        self.bottom.Add(self.resultsLabel, 0, wx.LEFT, 0)
        self.bottom.Add(self.resultsSizer, 0, wx.LEFT, 0)
        
        self.sizer.Add(self.top, 0, wx.TOP, 10)
        self.top.Add(self.top_left, 0, wx.LEFT, 0)
        self.top.Add(self.top_right, 0, wx.LEFT, 50)
        self.sizer.Add(self.bottom, 0, wx.CENTER, 0)
        
        self.SetSizer(self.sizer)

    def addTooltips(self):
        pass

    def bindControls(self):
        pass
