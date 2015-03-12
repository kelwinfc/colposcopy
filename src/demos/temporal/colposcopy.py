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


class ColposcopyResults(wx.Panel):
    def __init__(self, parent=None,
                 config_file="src/demos/temporal/config.json"):
        super(ColposcopyResults, self).__init__(parent,)

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

        self.diagnosis_first = wx.BoxSizer(wx.HORIZONTAL)
        self.diagnosis_second = wx.BoxSizer(wx.HORIZONTAL)
        self.diagnosis_third = wx.GridSizer(3, 2, 0, 0)


        ls = (0.08 * self.width, -1)
        ts = (0.10 * self.width, -1)

        self.title = wx.StaticText(self, wx.ID_ANY, label="Colposcopy")
        font = wx.Font(pointSize=12, family=wx.FONTFAMILY_DECORATIVE,
                       style=wx.NORMAL, weight=wx.FONTWEIGHT_BOLD)
        self.title.SetFont(font)

        self.sizer.Add(self.title, 0)
        self.sizer.Add(wx.StaticLine(self, wx.ID_ANY,
                                     size=(0.56 * self.width, -1)), 0)

        self.colposcopyBottomSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.colposcopyLeftSizer = wx.BoxSizer(wx.VERTICAL)
        self.colposcopyRightSizer = wx.BoxSizer(wx.VERTICAL)
        self.colposcopyBottomSizer.Add(self.colposcopyLeftSizer, 0)
        self.colposcopyBottomSizer.Add(self.colposcopyRightSizer, 0)
        self.sizer.Add(self.colposcopyBottomSizer)

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
                                              label="Vaginal discharge")
        self.limitationsWalls = wx.CheckBox(self, -1,
                                            label="Prolapsed vaginal walls")
        self.limitationsOtherLabel = wx.StaticText(self, label="Other")
        self.limitationsOtherInput = wx.TextCtrl(self, value="")

        self.diagnosis_first.Add(self.satisfactory, 0, wx.TOP, 0)
        self.diagnosis_second.Add(self.tr_zoneLabel, 0, wx.TOP, 10)
        self.diagnosis_second.Add(self.transformation_zone, 0, wx.LEFT, 5)

        self.diagnosis_third.Add(self.limitationsLabel, 0)
        self.diagnosis_third.Add(wx.BoxSizer(wx.VERTICAL), 0)  # Dummy space
        self.diagnosis_third.Add(self.limitationsVaginal, 0)
        self.diagnosis_third.Add(self.limitationsWalls, 0)
        self.diagnosis_third.Add(self.limitationsOtherLabel, 0, wx.TOP, 5)
        self.diagnosis_third.Add(self.limitationsOtherInput, 0, wx.TOP, 5)

        self.stagesSizer = wx.BoxSizer(wx.VERTICAL)
        self.macroscopic = MacroscopicResults(self)
        self.green = GreenResults(self)
        self.hinselmann = HinselmannResults(self)
        self.schiller = SchillerResults(self)

        self.colposcopyLeftSizer.Add(self.diagnosis_first, 0, wx.TOP, 10)
        self.colposcopyLeftSizer.Add(self.diagnosis_second, 0, wx.TOP, 0)
        self.colposcopyLeftSizer.Add(self.diagnosis_third, 0, wx.TOP, 20)

        self.stageSelectorLabel = wx.StaticText(self, label="Phase")
        self.stageSelectorChoice = wx.Choice(self,
                                             choices = ["Macroscopic View",
                                                        "Green light",
                                                        "Hinselmann",
                                                        "Schiller"])
        self.stageSelectorSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.stageSelectorSizer.Add(self.stageSelectorLabel, 0, wx.ALL, 0)
        self.stageSelectorSizer.Add(self.stageSelectorChoice, 0, wx.LEFT, 5)
        self.stagesSizer.Add(self.stageSelectorSizer, 0, wx.TOP | wx.CENTER,
                             10)
        
        self.stagesSizer.Add(self.macroscopic, 0, wx.LEFT, 20)
        self.stagesSizer.Add(self.green, 0, wx.LEFT, 20)
        self.stagesSizer.Add(self.hinselmann, 0, wx.LEFT, 20)
        self.stagesSizer.Add(self.schiller, 0, wx.LEFT, 20)
        self.colposcopyRightSizer.Add(self.stagesSizer, 0, wx.TOP, 10)

        self.SetSizer(self.sizer)
        
        self.show_hide_phases()

    def addTooltips(self):
        pass

    def bindControls(self):
        self.Bind(wx.EVT_CHOICE, self.show_hide_phases,
                  self.stageSelectorChoice)
        self.Bind(wx.EVT_RADIOBUTTON, self.showTRzone,
                  self.satisfactory.noRadio)
        self.Bind(wx.EVT_RADIOBUTTON, self.hideTRzone,
                  self.satisfactory.yesRadio)

    def showTRzone(self, event):
        self.tr_zoneLabel.Show()
        self.transformation_zone.Show()
        self.Layout()
        self.parent.Layout()

    def hideTRzone(self, event):
        self.tr_zoneLabel.Hide()
        self.transformation_zone.Hide()
        self.Layout()
        self.parent.Layout()

    def show_hide_phases(self, event=None):
        stages = [self.macroscopic, self.green, self.hinselmann,
                  self.schiller]
        for s in stages:
            s.Hide()

        stages[self.stageSelectorChoice.GetCurrentSelection()].Show()
        self.Layout()