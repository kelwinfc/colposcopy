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

class HinselmannResults(wx.Panel):
    def __init__(self, parent=None,
                 config_file="src/demos/temporal/config.json"):
        super(HinselmannResults, self).__init__(parent,)

        self.config = utils.load_config(config_file)
        self.width = 0
        self.height = 0
        self.width, self.height = wx.DisplaySize()
        self.parent = parent
        
        self.setLayout()
        self.addTooltips()
        self.bindControls()

    def setLayout(self, extra_values=[]):
        self.sizer = wx.BoxSizer(wx.HORIZONTAL)
        self.left = wx.BoxSizer(wx.VERTICAL)
        self.middle = wx.BoxSizer(wx.VERTICAL)
        self.right = wx.BoxSizer(wx.HORIZONTAL)

        self.sizer.Add(self.left, 0, wx.LEFT, 0)
        self.sizer.Add(self.middle, 0, wx.LEFT, 10)
        self.sizer.Add(self.right, 0, wx.LEFT|wx.CENTER, 10)

        self.title = wx.StaticText(self, wx.ID_ANY, label="Hinselmann")
        font = wx.Font(pointSize=12, family=wx.FONTFAMILY_DECORATIVE,
                       style=wx.NORMAL, weight=wx.FONTWEIGHT_BOLD)

        self.title.SetFont(font)
        self.left.Add(self.title, 0)
        self.left.Add(wx.StaticLine(self, wx.ID_ANY,
                                    size=(0.16 * self.width, -1)), 0)

        self.label = wx.StaticText(self, wx.ID_ANY, label="Diagnosis",
                                   size=(0.06 * self.width, -1))
        self.state = wx.Choice(self, wx.ID_ANY,
                               choices=["Healthy", "Lesions"],
                               size=(0.10 * self.width, -1))

        self.lfirst = wx.BoxSizer(wx.HORIZONTAL)
        self.lfirst.Add(self.label, 0, wx.ALL, 0)
        self.lfirst.Add(self.state, 0, wx.ALL, 0)
        self.left.Add(self.lfirst, 0, wx.TOP, 10)

        self.cervix = Cervix(self, name="hinselmann")
        self.middle.Add(self.cervix, 0, wx.ALL, 0)

        self.lesionLabel = wx.StaticText(self, wx.ID_ANY, label="Description",
                                         size=(0.08 * self.width, -1))
        self.lesionState = wx.Choice(self, wx.ID_ANY,
                                     choices=["White pattern",
                                              "Fine mosaic",
                                              "Dotted fine",
                                              "Dotted thick"
                                             ],
                                     size=(0.10 * self.width, -1))
        self.right.Add(self.lesionLabel, 0, wx.ALL, 0)
        self.right.Add(self.lesionState, 0, wx.ALL, 0)
        
        self.SetSizer(self.sizer)
        self.hide_lesion()

    def addTooltips(self):
        pass

    def bindControls(self):
        self.Bind(wx.EVT_CHOICE, self.OnChangeDiagnosis, self.state)
    
    def OnChangeDiagnosis(self, event):
        if self.state.GetCurrentSelection() == 0:
            self.hide_lesion()
            self.cervix.enable(False)
        else:
            self.lesionLabel.Show()
            self.lesionState.Show()
            self.cervix.enable(True)

        self.Layout()
        if self.parent is not None:
            self.parent.Layout()

    def hide_lesion(self):
        self.lesionLabel.Hide()
        self.lesionState.Hide()
        self.cervix.clear_angles()