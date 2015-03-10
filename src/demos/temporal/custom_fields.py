#!/usr/bin/python
# -*- coding: utf-8 -*-

from matplotlib import pyplot as plt
from matplotlib.patches import Wedge
import numpy as np
import utils
import math
import os
import wx

class TextField(wx.Panel):
    def __init__(self, parent, id, label="", default="",
                 label_size=(20, -1), text_size=(20, -1), style=wx.TE_LEFT):
        wx.Panel.__init__(self, parent, id)
        
        self.txt_label = label
        self.label_size = label_size
        self.text_size = text_size
        self.style = style
        self.default = default
        self.content = None
        self.setLayout()

    def setLayout(self, extra_values=[]):
        self.sizer = wx.BoxSizer(wx.HORIZONTAL)
        self.label = wx.StaticText(self, label=self.txt_label,
                                   size=self.label_size)
        self.content = wx.TextCtrl(self, value=self.default,
                                   size=self.text_size, style=self.style)
        self.sizer.Add(self.label, 0, wx.ALL, 5)
        self.sizer.Add(self.content, 0, wx.ALL, 5)
        self.SetSizer(self.sizer)

class YesNoField(wx.Panel):
    def __init__(self, parent, id, label="", default=0, label_size=(20, -1),
                 option_size=(20, -1)):
        wx.Panel.__init__(self, parent, id)
        
        self.txt_label = label
        self.label_size = label_size
        self.option_size = option_size
        self.default = default
        self.setLayout()

    def setLayout(self, extra_values=[]):
        self.sizer = wx.BoxSizer(wx.HORIZONTAL)
        self.label = wx.StaticText(self, label=self.txt_label,
                                   size=self.label_size)

        self.yesRadio = wx.RadioButton(self, label="Yes", style = wx.RB_GROUP,
                                       size=self.option_size)
        self.noRadio = wx.RadioButton(self, label="No", size=self.option_size)
        if self.default == 0:
            self.noRadio.SetValue(1)
            self.yesRadio.SetValue(0)
        else:
            self.yesRadio.SetValue(1)
            self.noRadio.SetValue(0)

        self.sizer.Add(self.label, 0, wx.ALL, 5)
        self.sizer.Add(self.yesRadio, 0, wx.ALL, 5)
        self.sizer.Add(self.noRadio, 0, wx.ALL, 5)
        self.SetSizer(self.sizer)


class Cervix(wx.Panel):
    def __init__(self, parent=None, size=(100, 100),
                 config_file="src/demos/temporal/config.json",
                 name="cervix", big_color="pink", small_color="brown"):
        super(Cervix, self).__init__(parent,)

        self.parent = parent
        self.config = utils.load_config(config_file)
        self.width = size[0]
        self.height = size[1]
        self.angle_start = None
        self.angle_end = None
        self.name = name
        self.path = os.path.join(self.config["tmp"], self.name + ".png")
        self.img = None
        self.enabled = False
        self.big_color = big_color
        self.small_color = small_color

        self.setLayout()
        self.addTooltips()
        self.bindControls()

    def setLayout(self):
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        self.draw_image()
        self.load_image()
        
        self.sizer.Add(self.img, 0, wx.ALL, 0)
        self.img.Bind(wx.EVT_LEFT_DOWN, self.OnMouseDown)
        self.img.Bind(wx.EVT_MOTION, self.OnMouseDrag)
        self.img.Bind(wx.EVT_LEFT_UP, self.OnMouseUp)

        self.SetSizer(self.sizer)

    def get_angle(self, event):
        (x, y) = event.GetPositionTuple()
        y = self.height - y
        x /= float(self.width)
        y /= float(self.height)
        x -= 0.5
        y -= 0.5

        ret = math.atan2(x, y) * 6.0 / np.pi
        
        if ret < 0:
            ret = 12.0 + ret

        return ret

    def OnMouseDrag(self, event):
        def angle_dist(a, b):
            return min(abs(a - b), 12.0 - abs(a - b))

        if not self.enabled:
            return

        if event.LeftIsDown():
            new_end = self.get_angle(event)

            if self.angle_end is None or \
                    angle_dist(new_end, self.last_update_end) > 0.5:
                self.last_update_end = new_end
                self.draw_image()
                self.load_image()
                self.Layout()
            self.angle_end = new_end

    def OnMouseDown(self, event):
        if not self.enabled:
            return

        self.angle_start = self.get_angle(event)
        self.angle_end = self.angle_start
        self.last_update_end = self.angle_end
        self.draw_image()
        self.load_image()
        self.Layout()

    def OnMouseUp(self, event):
        if not self.enabled:
            return

        self.draw_image()
        self.load_image()
        self.Layout()

    def addTooltips(self):
        pass

    def bindControls(self):
        pass

    def draw_image(self):
        plt.clf()
        center = (0.5,0.5)
        r = .5
        circle = plt.Circle(center, r, color=self.big_color, alpha=1.0)
        circle_edge = plt.Circle(center, r, color='k', fill=False)
        circle2 = plt.Circle(center, 0.1 * r, color=self.small_color,
                             alpha=1.0)

        area = None
        if self.angle_start is not None and self.angle_end is not None:
            area = Wedge(center, r,
                         90 - 30 * self.angle_end,
                         90 - 30 * self.angle_start,
                         fc='white', alpha=0.5)
        elif self.angle_start is not None and self.angle_end is None:
            area = Wedge(center, r,
                         90 - 30 * (self.angle_start + 1e-3),
                         90 - 30 * (self.angle_start - 1e-3),
                         fc='white', alpha=0.5)

        plt.axis('off')
        fig = plt.gcf()
        fig.gca().add_artist(circle)
        fig.gca().add_artist(circle_edge)

        if area is not None:
            fig.gca().add_artist(area)

        fig.gca().add_artist(circle2)
        plt.savefig(self.path, transparent=True, bbox_inches='tight')

    def load_image(self):
        circle_image = wx.Bitmap(self.path)
        circle_image = wx.ImageFromBitmap(circle_image)
        circle_image = circle_image.Scale(self.width, self.height,
                                          wx.IMAGE_QUALITY_HIGH)
        circle_image = wx.BitmapFromImage(circle_image)
        
        if self.img is None:
            self.img = wx.StaticBitmap(self, -1, bitmap=circle_image)
        else:
            self.img.SetBitmap(circle_image)

    def clear_angles(self):
        self.angle_start = None
        self.angle_end = None
        self.draw_image()
        self.load_image()

    def enable(self, v):
        if v:
            self.enabled = True
        else:
            self.clear_angles()
            self.enabled = False
