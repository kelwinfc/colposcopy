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
from macroscopic import *
from green import *
from hinselmann import *
from schiller import *

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
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        self.diagnosis_first = wx.BoxSizer(wx.HORIZONTAL)
        self.diagnosis_second = wx.BoxSizer(wx.HORIZONTAL)

        self.globalSizer.AddStretchSpacer(1)
        self.globalSizer.Add(self.sizer, 0, wx.ALIGN_CENTER, 0)
        self.globalSizer.AddStretchSpacer(1)

        ls = (0.08 * self.width, -1)
        ts = (0.10 * self.width, -1)
        
        self.satisfactory = YesNoField(self, wx.ID_ANY,
                                       label="Satisfactory colposcopy",
                                       default=0,
                                       label_size=(0.14 * self.width, -1),
                                       option_size=(0.045 * self.width, -1))
        self.tr_zoneLabel = wx.StaticText(self, label="Transformation zone",
                                          size=(0.16 * self.width, -1))
        self.transformation_zone = wx.Choice(self, wx.ID_ANY,
                                             choices=["Type 2", "Type 3"])

        self.limitationsLabel = wx.StaticText(self, label="Limitations",
                                              size=ls)
        self.limitationsVaginal = wx.CheckBox(self, -1,
                                              label="Vaginal fluid")
        self.limitationsWalls = wx.CheckBox(self, -1,
                                            label="Prolapsed vaginal walls")
        self.limitationsOther = TextField(self, wx.ID_ANY,
                                          label="Other", default="",
                                          label_size=(0.04 * self.width, -1),
                                                      text_size=ts)

        self.diagnosis_first.Add(self.satisfactory, 0, wx.TOP, 0)
        self.diagnosis_first.Add(self.tr_zoneLabel, 0, wx.TOP, 10)
        self.diagnosis_first.Add(self.transformation_zone, 0, wx.LEFT, 5)

        self.diagnosis_second.Add(self.limitationsLabel, 0, wx.ALL, 5)
        self.diagnosis_second.Add(self.limitationsVaginal, 0, wx.TOP, 2)
        self.diagnosis_second.Add(self.limitationsWalls, 0, wx.ALL, 2)
        self.diagnosis_second.Add(self.limitationsOther, 0, wx.LEFT, 10)

        self.stagesSizer = wx.GridSizer(4, 1, 0, 0) 
        self.macroscopic = MacroscopicResults(self)
        self.green = GreenResults(self)
        self.hinselmann = HinselmannResults(self)
        self.schiller = SchillerResults(self)
        
        self.sizer.Add(self.diagnosis_first, 0, wx.ALL, 0)
        self.sizer.Add(self.diagnosis_second, 0, wx.ALL, 0)

        self.stagesSizer.Add(self.macroscopic, 0, wx.LEFT, 5)
        self.stagesSizer.Add(self.green, 0, wx.LEFT, 5)
        self.stagesSizer.Add(self.hinselmann, 0, wx.LEFT, 5)
        self.stagesSizer.Add(self.schiller, 0, wx.LEFT, 5)
        self.sizer.Add(self.stagesSizer, 0)

        self.SetSizer(self.globalSizer)

    def addTooltips(self):
        pass

    def bindControls(self):
        self.Bind(wx.EVT_RADIOBUTTON, self.showTRzone,
                  self.satisfactory.noRadio)
        self.Bind(wx.EVT_RADIOBUTTON, self.hideTRzone,
                  self.satisfactory.yesRadio)

    def showTRzone(self, event):
        self.tr_zoneLabel.Show()
        self.transformation_zone.Show()
        self.Layout()

    def hideTRzone(self, event):
        self.tr_zoneLabel.Hide()
        self.transformation_zone.Hide()
        self.Layout()
