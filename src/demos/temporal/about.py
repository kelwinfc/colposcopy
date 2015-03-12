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


class AboutPanel(wx.Panel):
    def __init__(self, parent=None,
                 config_file="src/demos/temporal/config.json"):
        super(AboutPanel, self).__init__(parent,)

        self.config = utils.load_config(config_file)
        self.width = 0
        self.height = 0
        self.width, self.height = wx.DisplaySize()

        self.setLayout()
        self.addTooltips()
        self.bindControls()

    def setLayout(self, extra_values=[]):
        font = wx.Font(pointSize=12, family=wx.FONTFAMILY_DECORATIVE,
                       style=wx.NORMAL, weight=wx.FONTWEIGHT_BOLD)
        it_font = wx.Font(12, wx.DECORATIVE, wx.ITALIC, wx.NORMAL)

        self.sizer = wx.BoxSizer(wx.VERTICAL)

        self.globalSizer = wx.BoxSizer(wx.VERTICAL)
        self.globalSizer.AddStretchSpacer(1)
        self.globalSizer.Add(self.sizer, 0, wx.ALIGN_CENTER, 0)
        self.globalSizer.AddStretchSpacer(1)
        
        self.authorsLabel = wx.StaticText(self, label="Authors")
        self.authorsLabel.SetFont(font)
        self.sizer.Add(self.authorsLabel, 0, wx.ALIGN_CENTER | wx.TOP, 0)

        self.authorsSizer = wx.GridSizer(3, 2, 10, 0)
        authors = ["Kelwin Fernandes", "Jaime S. Cardoso", "Jessica Fernandes"]
        affiliations = ["INESC TEC, Porto, Portugal\n"
                        "Universidade do Porto",
                        "INESC TEC, Porto, Portugal\n"
                        "Universidade do Porto",
                        "Hospital Universitario de Caracas,\n"
                        "Universidad Central de Venezuela"]
        for author, affiliation in zip(authors, affiliations):
            next_author = wx.StaticText(self, label=author)
            next_affiliation = wx.StaticText(self, label=affiliation)
            next_affiliation.SetFont(it_font)
            self.authorsSizer.Add(next_author, 0, wx.LEFT | wx.ALIGN_RIGHT, 0)
            self.authorsSizer.Add(next_affiliation, 0, wx.LEFT, 10)

        self.sizer.Add(self.authorsSizer, 0, wx.ALIGN_CENTER | wx.TOP, 10)
        
        self.acknowledgements = wx.StaticText(self, label="Acknowledgements")
        self.acknowledgements.SetFont(font)
        self.sizer.Add(self.acknowledgements, 0, wx.ALIGN_CENTER | wx.TOP, 50)

        self.grant = wx.StaticText(self,
                      label="This project was financed by\n"
                            "Fundação para a Ciência e a Tecnologia (FCT),"
                            " Portugal,\n"
                            "through the grant SFRH/BD/93012/2013.",
                       style=wx.ALIGN_CENTRE_HORIZONTAL)
        self.sizer.Add(self.grant, 0, wx.ALIGN_CENTER | wx.TOP, 10)

        self.institutions= wx.StaticText(self, label="Institutions")
        self.institutions.SetFont(font)
        self.sizer.Add(self.institutions, 0, wx.ALIGN_CENTER | wx.TOP, 50)
        
        self.firstlogosSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.secondlogosSizer = wx.BoxSizer(wx.HORIZONTAL)
        
        self.inesc = None
        self.up = None
        self.fct = None
        self.huc = None
        self.ucv = None
        logos = [
                 (self.inesc, "INESC.png", 75, self.firstlogosSizer),
                 (self.up, "UP.jpg", 75, self.firstlogosSizer),
                 (self.fct, "FCT.png", 75, self.firstlogosSizer),
                 (self.huc, "HUC.png", 150, self.secondlogosSizer),
                 (self.ucv, "UCV.png", 150, self.secondlogosSizer)
                ]

        for (obj, logo, h, sizer) in logos:
            path = os.path.join(self.config["media"], "logos", logo)
            img = wx.Bitmap(path)
            size = img.GetSize()
            img = self.scale_image_size(img,
                                        size[0] * float(h) / size[1], h)

            obj = wx.StaticBitmap(self, wx.ID_ANY, img)
            sizer.Add(obj, 0, wx.LEFT|wx.ALIGN_CENTER, 30)

        self.sizer.Add(self.firstlogosSizer, 0, wx.TOP | wx.ALIGN_CENTER, 10)
        self.sizer.Add(self.secondlogosSizer, 0, wx.TOP | wx.ALIGN_CENTER, 20)

        self.SetSizer(self.globalSizer)

    def addTooltips(self):
        pass

    def bindControls(self):
        pass

    def scale_image_size(self, image, width, height):
        image = wx.ImageFromBitmap(image)
        image = image.Scale(width, height, wx.IMAGE_QUALITY_HIGH)
        return wx.BitmapFromImage(image)