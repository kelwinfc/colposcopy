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

EVT_RESULT_ID = wx.NewId()

class ResultEvent(wx.PyEvent):
    def __init__(self, data):
        wx.PyEvent.__init__(self)
        self.SetEventType(EVT_RESULT_ID)
        self.data = data

class WorkerThread(Thread):
    def __init__(self, notify_window, parent, index):
        Thread.__init__(self)
        self.parent = parent
        self.index = index
        self._notify_window = notify_window
        self._want_abort = 0
        self.start()

    def run(self):
        b = [0] + self.parent.boundaries + [self.parent.num_of_frames]
        num_frames = b[self.index + 1] - b[self.index]
        for frame in range(b[self.index], b[self.index + 1]):
            wx.PostEvent(self._notify_window, ResultEvent(frame))
            time.sleep(1.0 / self.parent.fps)

            if self._want_abort:
                wx.PostEvent(self._notify_window, ResultEvent(None))
                break

        wx.PostEvent(self._notify_window, ResultEvent(None))

    def abort(self):
        self._want_abort = 1

class FrameWidget(wx.Panel):
    def __init__(self, parent, name, index, id):
        wx.Panel.__init__(self, parent, id)
        
        self.index = index
        self.parent = parent
        self.config = self.parent.config
        self.name = name
        
        self.is_running = False
        self.previous_representative = None
        self.worker = None

        self.setLayout()
        self.addTooltips()
        self.bindControls()

    def addTooltips(self):
        pass

    def bindControls(self):
        self.play.Bind(wx.EVT_BUTTON, self.OnPlay)
        self.Connect(-1, -1, EVT_RESULT_ID, self.OnPlayingResult)

    def setLayout(self, extra_values=[]):
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        self.headerSizer = wx.BoxSizer(wx.HORIZONTAL)
        
        self.label = wx.StaticText(self, label=self.name)
        self.play = wx.BitmapButton(self, id=wx.ID_ANY,
                                    bitmap=wx.Bitmap(utils.media(self.config,
                                                                 "play")),
                                                     style=wx.NO_BORDER,
                                                     pos=(0, 0))

        self.headerSizer.Add(self.label, 0, wx.ALL, 0)
        self.headerSizer.Add(self.play, 0, wx.ALL, -1)

        self.img = wx.Bitmap(self.config["default"])
        self.img = self.parent.scale_image_size(self.img,
                                                0.24 * self.parent.width,
                                                0.24 * self.parent.height)
        self.img_bitmap = wx.StaticBitmap(self, -1, self.img)

        self.sizer.Add(self.headerSizer, 0, wx.ALL, 0)
        self.sizer.Add(self.img_bitmap, 0, wx.ALL, 0)
        self.SetSizer(self.sizer)
    
    def setImage(self, filename):
        self.img = wx.Bitmap(filename)
        self.img = self.parent.scale_image_size(self.img,
                                                0.24 * self.parent.width,
                                                0.24 * self.parent.height)
        self.img_bitmap.SetBitmap(self.img)
        self.Update()
        self.Refresh()

    def OnPlay(self, event):
        if self.parent.num_of_frames == 0:
            return
        
        if self.worker is None:
            self.is_running = True
            self.previous_representative = \
                self.parent.representatives[self.index]
            self.play.SetBitmapLabel(wx.Bitmap(utils.media(self.config,
                                                           "stop")))
            self.worker = WorkerThread(self, self.parent, self.index)
        else:
            self.OnStop(event)

    def OnPlayingResult(self, event):
        if event.data is not None:
            self.parent.representatives[self.index] = event.data
            self.parent.update_representatives(False)
        else:
            self.OnStop(event)

    def OnStop(self, event=None):
        self.is_running = False
        self.parent.representatives[self.index] = \
            self.previous_representative
        self.parent.update_representatives(False)
        self.play.SetBitmapLabel(wx.Bitmap(utils.media(self.config,
                                                  "play")))
        if self.worker is not None:
            self.worker.abort()
            self.worker = None


class VideoPanel(wx.Panel):
    def __init__(self, parent=None,
                 config_file="src/demos/temporal/config.json"):
        super(VideoPanel, self).__init__(parent,)

        self.config = utils.load_config(config_file)
        self.num_of_frames = 0
        self.current_frame = 0
        self.boundaries = [0, 0, 0]

        self.width = 0
        self.height = 0
        self.width, self.height = wx.DisplaySize()
        
        self.setLayout()
        self.addTooltips()
        self.bindControls()


        self.Show()

    def setLayout(self, extra_values=[]):
        # Sizers
        self.sizer = wx.BoxSizer(wx.VERTICAL)
        self.subSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.leftSizer = wx.BoxSizer(wx.VERTICAL)
        self.rightSizer = wx.BoxSizer(wx.VERTICAL)
        self.fileSizer = wx.BoxSizer(wx.HORIZONTAL)

        self.framesSizer = wx.BoxSizer(wx.VERTICAL)
        self.topFramesSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.bottomFramesSizer = wx.BoxSizer(wx.HORIZONTAL)

        self.sizer.Add(self.fileSizer, 0, wx.ALL|wx.CENTER, 10)
        self.sizer.Add(self.subSizer, 0, wx.ALL|wx.CENTER, 10)

        self.subSizer.Add(self.leftSizer, 0, wx.TOP, 0)
        self.subSizer.Add(self.rightSizer, 0, wx.TOP, 0)

        # Input file
        self.fileLabel = wx.StaticText(self, label="Video:")
        self.fileInput = wx.TextCtrl(self,
                                     value="",
                                     size=(0.4 * self.width, -1),
                                     style=wx.TE_LEFT)
        self.videoFilenameButton = \
            wx.BitmapButton(self, id=wx.ID_ANY,
                            bitmap=wx.Bitmap(utils.media(self.config,
                                                         "video")),
                                             style=wx.NO_BORDER,
                                             pos=(0, 0))
                            
        self.fileSizer.Add(self.fileLabel, 0, wx.TOP, 5)
        self.fileSizer.Add(self.fileInput, 0, wx.ALL, 0)
        self.fileSizer.Add(self.videoFilenameButton, 0, wx.ALL, 0)

        # Video
        self.imageLabel = wx.StaticText(self, label="Video Preview:")
        self.image = wx.Bitmap(self.config["default"])
        self.scale_image()
        self.imageControl = wx.StaticBitmap(self, -1, self.image)
        self.leftSizer.Add(self.imageLabel, 0, wx.TOP, 2)
        self.leftSizer.Add(self.imageControl, 0, wx.TOP, 5)
        
        # Colors and Tracker
        self.colors = wx.Bitmap(self.config["default"])
        self.colors = self.scale_image_size(self.colors,
                                            int(0.4 * self.width), 20)
        self.colorsImg = wx.StaticBitmap(self, -1, self.colors)
        
        self.tracker = wx.Slider(self, id=wx.ID_ANY, value=0, minValue=0,
                                 maxValue=0, size=(int(0.4 * self.width + 10),
                                                   20),
                                 style=wx.SL_HORIZONTAL | wx.SL_AUTOTICKS)

        self.boundariesImg = self.empty_boundaries_img()
        self.topBoundariesMarkers = wx.StaticBitmap(self, -1,
                                                    self.boundariesImg)
        self.bottomBoundariesMarkers = wx.StaticBitmap(self, -1,
                                                       self.boundariesImg)

        # Controllers
        self.controllerSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.stageLabel = wx.StaticText(self, label="Stage:")
        self.stageChoice = wx.Choice(self, id=wx.ID_ANY,
                                     choices=["Macroscopic", "Green",
                                              "Hinselmann", "Schiller"])
        self.stageSetLast = wx.Button(self, wx.ID_ANY,
                                      label="Set Last Frame")
        self.stageSetRepresentative = wx.Button(self, wx.ID_ANY,
                                      label="Set Representative")

        self.controllerSizer.Add(self.stageLabel, 0, wx.ALL, 5)
        self.controllerSizer.Add(self.stageChoice, 0, wx.LEFT, 0)
        self.controllerSizer.Add(self.stageSetLast, 0, wx.LEFT, 10)
        self.controllerSizer.Add(self.stageSetRepresentative, 0, wx.LEFT, 10)

        self.leftSizer.Add(self.tracker, 0, wx.LEFT, -5)
        self.leftSizer.Add(self.topBoundariesMarkers, 0, wx.TOP, 0)
        self.leftSizer.Add(self.colorsImg, 0, wx.TOP, 2)
        self.leftSizer.Add(self.bottomBoundariesMarkers, 0, wx.TOP, 2)
        self.leftSizer.Add(self.controllerSizer, 0, wx.TOP|wx.CENTER, 2)

        # Chosen frames
        self.macro = FrameWidget(self, "Macroscopic", 0, wx.ID_ANY)
        self.green = FrameWidget(self, "Green", 1, wx.ID_ANY)
        self.hinselmann = FrameWidget(self, "Hinselmann", 2, wx.ID_ANY)
        self.schiller = FrameWidget(self, "Schiller", 3, wx.ID_ANY)
        self.frames = [self.macro, self.green, self.hinselmann, self.schiller]
        self.boundaries = [0, 0, 0]
        self.representatives = [None, None, None, None]
        self.previous_representatives = [None, None, None, None]
        self.topFramesSizer.Add(self.macro, 0, wx.LEFT, 5)
        self.topFramesSizer.Add(self.green, 0, wx.LEFT, 10)
        self.bottomFramesSizer.Add(self.hinselmann, 0, wx.LEFT, 5)
        self.bottomFramesSizer.Add(self.schiller, 0, wx.LEFT, 10)

        self.rightSizer.Add(self.framesSizer)
        self.framesSizer.Add(self.topFramesSizer)
        self.framesSizer.Add(self.bottomFramesSizer, 0, wx.TOP, 5)

        self.SetSizer(self.sizer)

    def addTooltips(self):
        self.fileInput.SetToolTip(wx.ToolTip("Input video"))
        self.videoFilenameButton.SetToolTip(wx.ToolTip("Input video"))

    def bindControls(self):
        self.videoFilenameButton.Bind(wx.EVT_BUTTON, self.OnProcessVideo)
        self.stageSetLast.Bind(wx.EVT_BUTTON, self.OnSetLast)
        self.stageSetRepresentative.Bind(wx.EVT_BUTTON,
                                         self.OnSetRepresentative)
        self.tracker.Bind(wx.EVT_SCROLL_CHANGED, self.OnTrackerChanged)

    def OnProcessVideo(self, event):
        # Select the video
        dlg = wx.FileDialog(self, message="Choose a file",
                             defaultDir=os.getcwd(),
                             defaultFile="",
                             wildcard="(*.*)" \
                                      "All files (*.*)|*.*",
                             style=wx.OPEN
                            )

        path = '.'
        if dlg.ShowModal() == wx.ID_OK:
            path = dlg.GetPaths()[0]
        self.fileInput.SetValue(path)
        dlg.Destroy()

        # Create the temporary directory to store the images
        img_path = self.config["tmp"]
        try:
            shutil.rmtree(img_path)
        except:
            pass

        if not os.path.exists(img_path):
            os.makedirs(img_path)

        # Capture the images
        cap = cv2.VideoCapture(path)
        index = 0
        keepGoing = True
        num_frames = cap.get(cv.CV_CAP_PROP_FRAME_COUNT)

        try:
            self.fps = int(round(cap.get(cv.CV_CAP_PROP_FPS)))
        except:
            self.fps = 30

        progress_dlg = wx.ProgressDialog(
                        "Please wait.",
                        "Please wait while your video is loaded.\n",
                        maximum=num_frames,
                        parent=self,
                        style=wx.PD_CAN_ABORT
                              | wx.PD_ELAPSED_TIME
                              | wx.PD_REMAINING_TIME
                              | wx.PD_ESTIMATED_TIME
                              | wx.PD_APP_MODAL
                              | wx.PD_AUTO_HIDE
                        )
        
        self.num_of_frames = num_frames
        self.colors = []
        
        if self.num_of_frames == 0:
            progress_dlg.Destroy()
            wx.MessageBox('Invalid video', 'Error', 
                          wx.ICON_ERROR)
            return

        while keepGoing and index < num_frames:
            ret, frame = cap.read()
            if not ret:
                break

            filename = os.path.join(img_path, str(index) + ".jpg")
            if not os.path.isfile(filename):
                cv2.imwrite(filename, frame)

            keepGoing, _ = progress_dlg.Update(index)
            index += 1

            if index % self.config["colors-rate"] == 0:
                self.colors.append(np.mean(frame, axis=1))

            if index % self.config["refresh-rate"] == 0:
                self.current_frame = index
                self.image = wx.Bitmap(filename)
                self.scale_image()
                self.imageControl.SetBitmap(self.image)

        self.num_of_frames = index
        self.tracker.SetMax(self.num_of_frames - 1)

        progress_dlg.Destroy()
        cap.release()

        # Main colors
        self.colors = np.asarray(self.colors, np.uint8)
        self.colors = np.transpose(self.colors, axes=(1, 0, 2))
        self.colors = cv2.resize(self.colors, (int(0.4 * self.width), 20))

        self.colors = cv2.cvtColor(self.colors, cv2.COLOR_BGR2RGB)
        self.colors = wx.BitmapFromBuffer(self.colors.shape[1],
                                          self.colors.shape[0],
                                          self.colors)
        self.colorsImg.SetBitmap(self.colors)

        # Initialize
        self.current_frame = 0
        self.go_to_frame()
        self.update_boundaries()

    def scale_image(self):
        self.image = self.scale_image_size(self.image,
                                           0.4 * self.width,
                                           0.517 * self.height)

    def scale_image_size(self, image, width, height):
        image = wx.ImageFromBitmap(image)
        image = image.Scale(width, height, wx.IMAGE_QUALITY_HIGH)
        return wx.BitmapFromImage(image)

    def go_to_frame(self):
        if self.num_of_frames == 0:
            return

        filename = os.path.join(self.config["tmp"],
                                str(self.current_frame) + ".jpg")

        self.image = wx.Bitmap(filename)
        self.scale_image()
        self.imageControl.SetBitmap(self.image)

    def empty_boundaries_img(self):
        z = cv2.cvtColor(240 - np.zeros((int(0.4 * self.width), 10, 3),
                                        np.uint8),
                         cv2.COLOR_BGR2RGB)
        zz = np.transpose(z, axes=(1, 0, 2))
        path = os.path.join(self.config["tmp"], "boundaries.jpg")
        cv2.imwrite(path, zz)
        return wx.Bitmap(path)

    def update_boundaries(self):
        if self.num_of_frames == 0:
            return

        self.boundariesImg.Destroy()
        img = cv2.cvtColor(240 - np.zeros((int(0.4 * self.width), 10, 3),
                                        np.uint8),
                           cv2.COLOR_BGR2RGB)

        boundaries = [0] + self.boundaries + [self.num_of_frames]
        colors  = [[0, 0, 255], [0, 255, 0], [255, 255, 255], [0, 64, 128]]

        for i in range(4):
            left = int(float(boundaries[i]) / self.num_of_frames * \
                       img.shape[0])
            right = int(float(boundaries[i + 1]) / self.num_of_frames * \
                       img.shape[0])
            for r in xrange(left, right):
                img[r] = colors[i]

        for b in self.representatives:
            if b is None:
                continue

            for c in xrange(10):
                for r in xrange(3):
                    img[int(float(b) / (self.num_of_frames  + 1) * \
                            img.shape[0])] = np.asarray([0, 0, 0])
        img = np.transpose(img, axes=(1, 0, 2))
        
        path = os.path.join(self.config["tmp"], "boundaries.jpg")
        cv2.imwrite(path, img)

        self.boundariesImg = wx.Bitmap(path)
        self.boundariesImg = self.scale_image_size(self.boundariesImg,
                                                   self.colors.GetSize()[0],
                                                   10)

        self.topBoundariesMarkers.SetBitmap(self.boundariesImg)
        self.bottomBoundariesMarkers.SetBitmap(self.boundariesImg)
        self.Update()
        self.Refresh()

    def OnTrackerChanged(self, event):
        self.current_frame = self.tracker.GetValue()
        self.go_to_frame()

    def OnSetLast(self, event):
        if self.num_of_frames == 0:
            return

        self.current_frame = self.tracker.GetValue()
        boundary = self.stageChoice.GetCurrentSelection()
        
        if 0 <= boundary and boundary < 3:
            self.boundaries[boundary] = self.current_frame
            for b in range(boundary + 1, 3):
                self.boundaries[b] = max(self.boundaries[b],
                                         self.current_frame)
            self.update_boundaries()

    def update_representatives(self, update=True):
        for i in range(4):
            if self.previous_representatives[i] != self.representatives[i]:
                f = self.config["default"]
                if self.representatives[i] is not None:
                    f = os.path.join(self.config["tmp"],
                                     str(self.representatives[i]) + ".jpg")
                self.frames[i].setImage(f)
                self.previous_representatives[i] = self.representatives[i]

        if update:
            self.update_boundaries()

    def OnSetRepresentative(self, event):
        f = self.tracker.GetValue()
        stage = None
        boundaries = [0] + self.boundaries + [self.num_of_frames]

        for i in range(4):
            if boundaries[i] <= f and f < boundaries[i + 1]:
                stage = i
                self.stageChoice.SetSelection(i)
                self.representatives[i] = f
                self.update_representatives()
                break
