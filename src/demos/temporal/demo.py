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

from video import *
from patient_info import *
from diagnosis import *
from about import *

class ColposcopicScreening(wx.Frame):

    def __init__(self, parent):
        super(ColposcopicScreening, self).__init__(parent,
            style=wx.MAXIMIZE_BOX | wx.RESIZE_BORDER | wx.SYSTEM_MENU | \
                       wx.CAPTION | wx.CLOSE_BOX,
            pos=(0, 0), size=wx.DisplaySize())
        self.InitUI()

    def InitUI(self):
        pnl = wx.Panel(self, -1)

        sizer = wx.BoxSizer(wx.VERTICAL)

        nestedNotebook = wx.Notebook(pnl, wx.NewId())
        self.patientTab = PatientPanel(nestedNotebook)
        self.videoTab = VideoPanel(nestedNotebook)
        self.diagnosisTab = DiagnosisPanel(nestedNotebook)
        self.aboutTab = AboutPanel(nestedNotebook)
        
        nestedNotebook.AddPage(self.patientTab, "Patient")
        nestedNotebook.AddPage(self.videoTab, "Video")
        nestedNotebook.AddPage(self.diagnosisTab, "Diagnosis")
        nestedNotebook.AddPage(self.aboutTab, "About")

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(nestedNotebook, 1, wx.ALL | wx.EXPAND, 5)

        pnl.SetSizer(sizer)

        self.SetTitle('Colposcopic Screening')
        self.Centre()
        self.Show(True)
        self.Maximize(True)

    def OnClose(self, e):
        self.Close(True)

    def OnQuit(self, e):
        self.Close()

app = wx.App()
ColposcopicScreening(None)
app.MainLoop()
