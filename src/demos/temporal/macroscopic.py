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


class MacroscopicResults(wx.Panel):
    def __init__(self, parent=None,
                 config_file="src/demos/temporal/config.json"):
        super(MacroscopicResults, self).__init__(parent,)

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
        
        self.lfirst = wx.BoxSizer(wx.HORIZONTAL)
        self.lsecond = wx.BoxSizer(wx.HORIZONTAL)
        self.rfirst = wx.BoxSizer(wx.HORIZONTAL)
        self.rsecond = wx.BoxSizer(wx.HORIZONTAL)
        self.rthird = wx.BoxSizer(wx.HORIZONTAL)

        self.sizer.Add(self.top, 0, wx.LEFT, 0)
        self.sizer.Add(self.bottom, 0, wx.TOP, 10)

        self.label = wx.StaticText(self, wx.ID_ANY, label="Diagnosis",
                                   size=(0.06 * self.width, -1))
        self.state = wx.Choice(self, wx.ID_ANY,
                               choices=["Macroscopically healthy",
                                        "Architectural distortion"],
                               size=(0.16 * self.width, -1))

        self.top.Add(self.lsecond, 0, wx.TOP, 10)
        self.lsecond.Add(self.label, 0, wx.ALL, 0)
        self.lsecond.Add(self.state, 0, wx.ALL, 0)

        self.lesionLabel = wx.StaticText(self, wx.ID_ANY, label="Lesion",
                                         size=(0.08 * self.width, -1))
        self.lesionState = wx.Choice(self, wx.ID_ANY,
                                     choices=["Exophytic", "Endophytic"],
                                     size=(0.10 * self.width, -1))
        self.rfirst.Add(self.lesionLabel, 0, wx.ALL, 0)
        self.rfirst.Add(self.lesionState, 0, wx.ALL, 0)

        self.regionLabel = wx.StaticText(self, wx.ID_ANY, label="Reach",
                                         size=(0.08 * self.width, -1))
        self.regionState = wx.Choice(self, wx.ID_ANY,
                                     choices=["Cervix", "Vagina"],
                                     size=(0.10 * self.width, -1))

        self.rsecond.Add(self.regionLabel, 0, wx.ALL, 0)
        self.rsecond.Add(self.regionState, 0, wx.ALL, 0)

        self.length = TextField(self, wx.ID_ANY, label="Diameter (cm)",
                                default="0",
                                label_size=(0.08 * self.width - 10, -1),
                                text_size=(0.10 * self.width, -1),
                                style=wx.TE_RIGHT)
        self.rthird.Add(self.length, 0, wx.LEFT, 0)

        self.cervix = Cervix(self, name="macroscopic")

        self.bottom.Add(self.cervix, 0, wx.LEFT, 0)
        self.bottom.Add(self.bottomRight, 0, wx.LEFT, 10)
        self.bottomRight.Add(self.rfirst, 0, wx.TOP, -1)
        self.bottomRight.Add(self.rsecond, 0, wx.TOP, -1)
        self.bottomRight.Add(self.rthird, 0, wx.TOP, -1)

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
            self.regionLabel.Show()
            self.regionState.Show()
            self.length.Show()
            self.cervix.enable(True)

        self.Layout()

        parent = self.parent
        try:
            while parent is not None:
                parent.Layout()
                parent = parent.parent
        except:
            pass  # Finished layout

    def hide_lesion(self):
        self.lesionLabel.Hide()
        self.lesionState.Hide()
        self.regionLabel.Hide()
        self.regionState.Hide()
        self.length.Hide()
        self.cervix.clear_angles()
