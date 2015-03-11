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


class SchillerResults(wx.Panel):
    def __init__(self, parent=None,
                 config_file="src/demos/temporal/config.json"):
        super(SchillerResults, self).__init__(parent,)

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
        self.bottom = wx.BoxSizer(wx.HORIZONTAL)
        self.bottomRight = wx.BoxSizer(wx.VERTICAL)

        self.sizer.Add(self.top, 0, wx.TOP, 0)
        self.sizer.Add(self.bottom, 0, wx.TOP, 10)

        self.title = wx.StaticText(self, wx.ID_ANY, label="Schiller")
        font = wx.Font(pointSize=10, family=wx.FONTFAMILY_DECORATIVE,
                       style=wx.NORMAL, weight=wx.FONTWEIGHT_BOLD)

        self.title.SetFont(font)
        self.top.Add(self.title, 0)
        self.top.Add(wx.StaticLine(self, wx.ID_ANY,
                                   size=(0.28 * self.width, -1)), 0)

        self.label = wx.StaticText(self, wx.ID_ANY, label="Diagnosis",
                                   size=(0.06 * self.width, -1))
        self.state = wx.Choice(self, wx.ID_ANY,
                               choices=["Stained", "Non-stained"],
                               size=(0.16 * self.width, -1))

        self.lfirst = wx.BoxSizer(wx.HORIZONTAL)
        self.lfirst.Add(self.label, 0, wx.TOP, 0)
        self.lfirst.Add(self.state, 0, wx.ALL, 0)
        self.top.Add(self.lfirst, 0, wx.TOP, 10)

        self.cervix = Cervix(self, name="schiller",
                             big_color="brown", small_color="black")
        self.bottom.Add(self.cervix, 0, wx.ALL, 0)
        self.bottom.Add(self.bottomRight, 0, wx.LEFT, 10)

        self.hipo = YesNoField(self, wx.ID_ANY, label="Generalized cold",
                               default=0,
                               label_size=(0.08 * self.width, -1),
                               option_size=(0.045 * self.width, -1))
        self.alergic = YesNoField(self, wx.ID_ANY, label="Allergic to iodine",
                                  default=0,
                                  label_size=(0.08 * self.width, -1),
                                  option_size=(0.045 * self.width, -1))

        self.bottomRight.Add(self.hipo, 0, wx.ALL, 0)
        self.bottomRight.Add(self.alergic, 0, wx.ALL, 0)

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
            self.cervix.enable(True)

        self.Layout()
        if self.parent is not None:
            self.parent.Layout()

    def hide_lesion(self):
        self.cervix.clear_angles()
