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

from custom_fields import *

class PatientPanel(wx.Panel):
    def __init__(self, parent=None,
                 config_file="src/demos/temporal/config.json"):
        super(PatientPanel, self).__init__(parent,)

        self.config = utils.load_config(config_file)
        self.width = 0
        self.height = 0
        self.width, self.height = wx.DisplaySize()

        self.setLayout()
        self.addTooltips()
        self.bindControls()
        #self.Show()

    def setLayout(self, extra_values=[]):
        # Sizers
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        self.infoSizer = wx.BoxSizer(wx.VERTICAL)
        self.tobaccoSizer = wx.BoxSizer(wx.VERTICAL)
        self.contraceptivesSizer = wx.BoxSizer(wx.VERTICAL)
        
        self.sizer.Add(self.infoSizer, 0, wx.ALL, 20)
        self.sizer.Add(self.tobaccoSizer, 0, wx.ALL, 20)
        self.sizer.Add(self.contraceptivesSizer, 0, wx.ALL, 20)

        # Personal Info
        ls = (0.08 * self.width, -1)
        ts = (0.10 * self.width, -1)
        self.patientId = TextField(self, wx.ID_ANY,
                                   label="Patient ID", default="",
                                   label_size=ls, text_size=ts)
        self.patientDNI = TextField(self, wx.ID_ANY,
                                   label="Patient DNI", default="",
                                   label_size=ls, text_size=ts)
        self.patientFirstName = TextField(self, wx.ID_ANY,
                                          label="First Name", default="",
                                          label_size=ls, text_size=ts)
        self.patientLastName = TextField(self, wx.ID_ANY,
                                         label="Last Name", default="",
                                         label_size=ls, text_size=ts)
        self.patientTelephone = TextField(self, wx.ID_ANY,
                                          label="Phone number", default="",
                                          label_size=ls, text_size=ts)
        self.patientAge = TextField(self, wx.ID_ANY,
                                    label="Age", default="",
                                    label_size=ls, text_size=ts)

        self.infoSizer.Add(wx.StaticText(self, label="Personal Information",
                                         size=(0.18 * self.width, -1)))
        self.infoSizer.Add(wx.StaticLine(self, size=(0.40 * self.width, -1)),
                           0, wx.TOP, 5)

        self.idSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.idSizer.Add(self.patientId, 0, wx.ALL, 0)
        self.idSizer.Add(self.patientDNI, 0, wx.LEFT, 20)
        self.infoSizer.Add(self.idSizer, 0, wx.TOP, 20)
        
        self.nameSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.nameSizer.Add(self.patientFirstName, 0, wx.ALL, 0)
        self.nameSizer.Add(self.patientLastName, 0, wx.LEFT, 20)
        self.infoSizer.Add(self.nameSizer, 0, wx.TOP, 10)
        
        self.otherSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.otherSizer.Add(self.patientTelephone, 0, wx.ALL, 0)
        self.otherSizer.Add(self.patientAge, 0, wx.LEFT, 20)
        self.infoSizer.Add(self.otherSizer, 0, wx.TOP, 10)

        # Tobacco
        self.subTobaccoSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.tobaccoSizer.Add(wx.StaticText(self,
                                            label="Tobacco consumption",
                              size=(0.18 * self.width, -1)))
        self.tobaccoSizer.Add(wx.StaticLine(self,
                                            size=(0.40 * self.width, -1)),
                              0, wx.TOP, 5)
        self.tobacco = YesNoField(self, wx.ID_ANY, label="Tobacco", default=0,
                                  label_size=ls,
                                  option_size=(0.045 * self.width, -1))
        self.tobaccoYears = TextField(self, wx.ID_ANY,
                                      label="Years", default="0",
                                      label_size=ls, text_size=ts,
                                      style=wx.TE_RIGHT)

        self.subTobaccoSizer.Add(self.tobacco, 0, wx.TOP, 0)
        self.subTobaccoSizer.Add(self.tobaccoYears, 0, wx.LEFT, 20)
        self.tobaccoSizer.Add(self.subTobaccoSizer, 0, wx.TOP, 10)

        # Contraceptives
        self.contraceptivesSizer.Add(wx.StaticText(self,
                                                   label="Contraceptives",
                                     size=(0.18 * self.width, -1)))
        self.contraceptivesSizer.Add(wx.StaticLine(self,
                                                   size=(0.40 * self.width,
                                                         -1)),
                                     0, wx.TOP, 5)

        self.contraceptives_first = wx.BoxSizer(wx.HORIZONTAL)
        self.contraceptives_second = wx.BoxSizer(wx.HORIZONTAL)
        
        self.hormonal = YesNoField(self, wx.ID_ANY, label="Hormonal", default=0,
                                   label_size=ls,
                                   option_size=(0.045 * self.width, -1))
        self.hormonalYears = TextField(self, wx.ID_ANY,
                                       label="Years", default="0",
                                       label_size=ls, text_size=ts,
                                       style=wx.TE_RIGHT)
        self.contraceptives_first.Add(self.hormonal, 0, wx.TOP, 0)
        self.contraceptives_first.Add(self.hormonalYears, 0, wx.LEFT, 20)

        self.iud = YesNoField(self, wx.ID_ANY,
                              label="Intrauterine device", default=0,
                              label_size=ls,
                              option_size=(0.045 * self.width, -1))
        self.iudYears = TextField(self, wx.ID_ANY,
                                  label="Years", default="0",
                                  label_size=ls, text_size=ts,
                                  style=wx.TE_RIGHT)
        self.contraceptives_second.Add(self.iud, 0, wx.TOP, 0)
        self.contraceptives_second.Add(self.iudYears, 0, wx.LEFT, 20)

        self.contraceptivesSizer.Add(self.contraceptives_first, 0, wx.TOP, 20)
        self.contraceptivesSizer.Add(self.contraceptives_second, 0, wx.TOP, 20)

        self.SetSizer(self.sizer)

    def addTooltips(self):
        pass

    def bindControls(self):
        self.Bind(wx.EVT_RADIOBUTTON, self.restartTobacco,
                  self.tobacco.noRadio)
        self.Bind(wx.EVT_RADIOBUTTON, self.restartHormonal,
                  self.hormonal.noRadio)
        self.Bind(wx.EVT_RADIOBUTTON, self.restartIUD,
                  self.iud.noRadio)

    def restartTobacco(self, event):
        self.tobaccoYears.content.SetValue("0")

    def restartHormonal(self, event):
        self.hormonalYears.content.SetValue("0")
        
    def restartIUD(self, event):
        self.iudYears.content.SetValue("0")
