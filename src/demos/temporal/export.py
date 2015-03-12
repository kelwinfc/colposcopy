#!/usr/bin/python
# -*- coding: utf-8 -*-

from matplotlib import pyplot as plt
from wx.calendar import CalendarCtrl
from threading import *
import cv2.cv as cv
import numpy as np
import wx.calendar
import webbrowser
import subprocess
import tempfile
import datetime
import wx.grid
import shutil
import utils
import time
import json
import cv2
import os
import wx

from pylatex import *
from pylatex.numpy import Matrix
from pylatex.utils import italic, escape_latex

from custom_fields import *


class ExportData(wx.Panel):
    def __init__(self, parent=None,
                 config_file="src/demos/temporal/config.json"):
        super(ExportData, self).__init__(parent,)

        self.config = utils.load_config(config_file)
        self.width = 0
        self.height = 0
        self.width, self.height = wx.DisplaySize()
        self.parent = parent

        self.setLayout()
        self.addTooltips()
        self.bindControls()

    def setLayout(self, extra_values=[]):
        lsize = (0.30 * self.width, -1)
        
        font = wx.Font(pointSize=10, family=wx.FONTFAMILY_DECORATIVE,
                       style=wx.NORMAL, weight=wx.FONTWEIGHT_BOLD)

        self.sizer = wx.BoxSizer(wx.VERTICAL)
        
        self.title = wx.StaticText(self, wx.ID_ANY, label="Export Data")
        title_font = wx.Font(pointSize=12, family=wx.FONTFAMILY_DECORATIVE,
                             style=wx.NORMAL, weight=wx.FONTWEIGHT_BOLD)
        self.title.SetFont(title_font)

        self.sizer.Add(self.title, 0)
        self.sizer.Add(wx.StaticLine(self, wx.ID_ANY, size=lsize), 0)

        # Physician ID
        s = (0.15 * self.width, -1)
        self.physician = TextField(self, wx.ID_ANY, label="Physician ID",
                                   label_size=s, text_size=s, style=wx.TE_LEFT)
        self.sizer.Add(self.physician, 0, wx.TOP, 10)

        # Date
        self.dateSizer = wx.BoxSizer(wx.VERTICAL)
        self.date = CalendarCtrl(self, size=(0.22 * self.width, -1))
        self.dateSizer.Add(wx.StaticText(self, label="Assessment date"), 0)
        self.dateSizer.Add(self.date)
        self.sizer.Add(self.dateSizer, 0, wx.CENTER, 0)
        
        self.sizer.Add(wx.StaticLine(self, wx.ID_ANY, size=lsize), 0,
                       wx.TOP, 5)

        # Selective export
        self.personal = wx.CheckBox(self, label="Personal Information",
                                      style=wx.RB_GROUP)
        self.risk = wx.CheckBox(self, label="Risk Factors")
        
        self.colposcopy = wx.CheckBox(self, label="Colposcopy")
        self.representatives = wx.CheckBox(self, label="Chosen frames")
        self.macroscopic = wx.CheckBox(self, label="Macroscopic results")
        self.green = wx.CheckBox(self, label="Green results")
        self.hinselmann = wx.CheckBox(self, label="Hinselmann results")
        self.schiller = wx.CheckBox(self, label="Schiller Results")
        
        self.citology = wx.CheckBox(self, label="Citology Results")
        self.biopsy = wx.CheckBox(self, label="Biopsy Results")


        self.selectiveSizer = wx.GridSizer(13, 2, 0, 10)

        dummy = wx.Size(0, 0)
        for x in [wx.StaticText(self, label="Patient information"), dummy,
                  self.personal, self.risk,
                  dummy, dummy,
                  wx.StaticText(self, label="Colposcopy"), dummy,
                  self.colposcopy, self.representatives, 
                  self.macroscopic, self.green,
                  self.hinselmann, self.schiller,
                  dummy, dummy,
                  wx.StaticText(self, label="Citology"), dummy,
                  self.citology, dummy,
                  dummy, dummy,
                  wx.StaticText(self, label="Biopsy"), dummy,
                  self.biopsy, dummy]:
            if type(x) == wx.CheckBox:
                x.SetValue(True)
            self.selectiveSizer.Add(x, 0)
        
        self.sizer.Add(self.selectiveSizer, 0, wx.TOP, 10)

        self.sizer.Add(wx.StaticLine(self, wx.ID_ANY, size=lsize), 0,
                       wx.TOP, 10)

        # Filename
        self.fileSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.fileLabel = wx.StaticText(self, label="Export")
        self.filenameButton = \
            wx.BitmapButton(self, bitmap=wx.Bitmap(utils.media(self.config,
                                                               "save")),
                            style=wx.NO_BORDER)

        self.fileSizer.Add(self.fileLabel, 0, wx.TOP, 5)
        self.fileSizer.Add(self.filenameButton, 0, wx.LEFT, 10)
        self.sizer.Add(self.fileSizer, 0, wx.TOP|wx.ALIGN_RIGHT, 10)

        self.SetSizer(self.sizer)

    def addTooltips(self):
        pass

    def bindControls(self):
        self.Bind(wx.EVT_BUTTON, self.OnSave, self.filenameButton)
        self.Bind(wx.EVT_CHECKBOX, self.OnCheckColposcopy, self.colposcopy)
        self.Bind(wx.EVT_CHECKBOX, self.OnCheckStage, self.macroscopic)
        self.Bind(wx.EVT_CHECKBOX, self.OnCheckStage, self.green)
        self.Bind(wx.EVT_CHECKBOX, self.OnCheckStage, self.hinselmann)
        self.Bind(wx.EVT_CHECKBOX, self.OnCheckStage, self.schiller)
        self.Bind(wx.EVT_CHECKBOX, self.OnCheckStage, self.representatives)

    def OnCheckColposcopy(self, event):
        if not self.colposcopy.GetValue():
            self.macroscopic.SetValue(False)
            self.green.SetValue(False)
            self.hinselmann.SetValue(False)
            self.schiller.SetValue(False)
            self.representatives.SetValue(False)
            self.selectiveSizer.Layout()

    def OnCheckStage(self, event):
        if not self.colposcopy.GetValue():
            phases = [self.macroscopic, self.green, self.hinselmann,
                      self.schiller, self.representatives]
            for phase in phases:
                if phase.GetValue():
                    self.colposcopy.SetValue(True)
                    self.selectiveSizer.Layout()
                    break

    def OnSave(self, event):
        wildcard = "PDF files (*.pdf)|*.pdf|json files (*.json)|*.json"

        dlg = wx.FileDialog(self, message="Choose a file",
                             defaultDir=os.getcwd(),
                             defaultFile="default.pdf",
                             wildcard=wildcard,
                             style=wx.SAVE
                            )

        path = 'default.pdf'
        if dlg.ShowModal() == wx.ID_OK:
            path = dlg.GetPaths()[0]

        json_value = self.get_json(path)
        fformat = path.split('.')[-1]

        if fformat == "pdf":
            self.save_pdf(json_value, path)
        elif fformat == "json":
            self.save_json(json_value, path)

        dlg.Destroy()

    def get_json(self, path):
        topFrame = wx.GetTopLevelParent(self.parent)

        ret = {}
        
        # Personal information
        if self.personal.GetValue():
            p = topFrame.patientTab
            
            personal = \
                {
                 "id": p.patientId.content.GetValue(),
                 "dni": p.patientDNI.content.GetValue(),
                 "first_name": p.patientFirstName.content.GetValue(),
                 "last_name": p.patientLastName.content.GetValue(),
                 "phone": p.patientTelephone.content.GetValue(),
                 "age": p.patientAge.content.GetValue(),
                }
            ret["personal"] = personal

        # Risk factors
        if self.risk.GetValue():
            p = topFrame.patientTab
            
            risk = \
                {
                    "tobacco": p.tobacco.yesRadio.GetValue(),
                    "tobacco_years": p.tobaccoYears.content.GetValue(),
                    "hormonal": p.hormonal.yesRadio.GetValue(),
                    "hormonal_years": p.hormonalYears.content.GetValue(),
                    "iud": p.iud.yesRadio.GetValue(),
                    "iud_years": p.iudYears.content.GetValue(),
                    "num_partners": p.num_partners.content.GetValue(),
                    "intercourse": p.first_intercourse.content.GetValue(),
                    "num_pregnancies": p.pregnancies.content.GetValue(),
                    "sti": p.sti.yesRadio.GetValue(),
                    "sti_info": p.stiInfo.content.GetValue(),
                    "sti_date": p.stiDate.content.GetValue(),
                    "cervical_pathology": p.pathology.yesRadio.GetValue(),
                    "cervical_pathology_info": \
                        p.pathologyInfo.content.GetValue(),
                    "cervical_pathology_date": \
                        p.pathologyDate.content.GetValue(),
                    "excisional": p.excisional.yesRadio.GetValue(),
                    "workflow": p.workflow.yesRadio.GetValue(),
                }
            ret["risk"] = risk

        
        # Colposcopy
        if self.colposcopy.GetValue():
            ret["colposcopy"] = {}

            # Chosen frames
            if self.representatives.GetValue():
                p = topFrame.videoTab
                macroscopic = p.macro
                green = p.green
                hinselmann = p.hinselmann
                schiller = p.schiller
                names = ["macroscopic", "green", "hinselmann", "schiller"]
                frames = [p.macro, p.green, p.hinselmann, p.schiller]
                representatives = {}
                
                for i, n, f in zip(range(4), names, frames):
                    next_frame = p.representatives[i]
                    if next_frame is not None:
                        new_filename = '.'.join(path.split('.')[: -1])
                        new_filename += "%s.jpg" % n
                        shutil.copy2(os.path.join(self.config["tmp"],
                                                  str(next_frame) + ".jpg"),
                                     new_filename)

                        representatives[n] = new_filename
                    else:
                        representatives[n] = self.config["default"]

                ret["colposcopy"]["frames"] = representatives

            # General info
            p = topFrame.diagnosisTab.colposcopy

            ret["colposcopy"]["satisfactory"] = \
                p.satisfactory.yesRadio.GetValue()
            if not p.satisfactory.yesRadio.GetValue():
                ret["colposcopy"]["transformation"] = \
                    p.transformation_zone.GetStringSelection()
            ret["colposcopy"]["limitations_vaginal_discharge"] = \
                p.limitationsVaginal.GetValue()
            ret["colposcopy"]["limitations_vaginal_walls"] = \
                p.limitationsWalls.GetValue()
            ret["colposcopy"]["limitations_other"] = \
                p.limitationsOtherInput.GetValue()

        # Macroscopic
        if self.macroscopic.GetValue():
            p = topFrame.diagnosisTab.colposcopy.macroscopic
            macroscopic = {}
            macroscopic["diagnosis"] = p.state.GetStringSelection()
            
            if p.state.GetCurrentSelection() == 1:
                macroscopic["lesion"] = p.lesionState.GetStringSelection()
                macroscopic["region"] = p.regionState.GetStringSelection()
                macroscopic["diameter"] = p.length.content.GetValue()
                macroscopic["angle"] = [p.cervix.angle_start,
                                        p.cervix.angle_end]

            ret["colposcopy"]["macroscopic"] = macroscopic

        # Green
        if self.green.GetValue():
            p = topFrame.diagnosisTab.colposcopy.green
            green = {}
            green["diagnosis"] = p.state.GetStringSelection()
            
            if p.state.GetCurrentSelection() == 1:
                green["angle"] = [p.cervix.angle_start, p.cervix.angle_end]

            ret["colposcopy"]["green"] = green

        # Hinselmann
        if self.hinselmann.GetValue():
            hinselmann = {}
            p = topFrame.diagnosisTab.colposcopy.hinselmann
            hinselmann["diagnosis"] = p.state.GetStringSelection()
            
            if p.state.GetCurrentSelection() == 1:
                hinselmann["angle"] = [p.cervix.angle_start,
                                       p.cervix.angle_end]
                hinselmann["description"] = p.lesionState.GetStringSelection()

            ret["colposcopy"]["hinselmann"] = hinselmann

        # Schiller
        if self.schiller.GetValue():
            schiller = {}
            p = topFrame.diagnosisTab.colposcopy.schiller
            schiller["diagnosis"] = p.state.GetStringSelection()
            schiller["angle"] = [p.cervix.angle_start,
                                 p.cervix.angle_end]
            schiller["cold"] = p.hipo.yesRadio.GetValue()
            schiller["allergic"] = p.allergic.yesRadio.GetValue()

            ret["colposcopy"]["schiller"] = schiller

        # Citology
        if self.citology.GetValue():
            citology = {}
            p = topFrame.diagnosisTab.citology

            citology["transformation_zone"] = \
                p.resultChoice.GetStringSelection()
            citology["sample"] = p.sampleChoice.GetStringSelection()
            citology["no-evidence"] = p.noEvidence.GetValue()
            citology["lowGrade"] = p.lowGrade.GetValue()
            citology["highGrade"] = p.highGrade.GetValue()
            citology["carcinoma"] = p.carcinoma.GetValue()
            citology["ascus"] = p.ascus.GetValue()
            citology["asgus"] = p.asgus.GetValue()
            citology["squamous"] = p.squamous.GetValue()
            citology["asch"] = p.asch.GetValue()
            citology["glandular"] = p.glandular.GetValue()
            
            ret["citology"] = citology

        # Biopsy
        if self.biopsy.GetValue():
            biopsy = {}
            p = topFrame.diagnosisTab.biopsy
            biopsy["angle"] = [p.cervix.angle_start, p.cervix.angle_end]
            biopsy["incisional"] = p.incisional.GetValue()
            biopsy["excisional"] = p.excisional.GetValue()
            biopsy["letz"] = p.letz.GetValue()
            biopsy["cone"] = p.cone.GetValue()
            biopsy["endocervical"] = p.endocervical.GetValue()
            biopsy["result"] = p.lesionResults.GetStringSelection()
            biopsy["margin"] = p.marginResults.GetStringSelection()
            ret["biopsy"] = biopsy

        # Physician
        print dir(self.date.GetDate)
        ret["physician"] = self.physician.content.GetValue()
        y = self.date.GetDate().GetYear()
        m = self.date.GetDate().GetMonth() + 1
        d = self.date.GetDate().GetDay()
        ret["date"] = "%4d/%02d/%02d" % (y, m, d)

        return ret

    def save_pdf(self, json_value, filename):
        def get_new_command(k, v):
            vv = str(v)
            if type(v) == bool:
                vv = "$\surd$" if v else "$\\times$"
            elif type(v) == list:
                vv = ' - '.join(["%0.2f" % x for x in v if x is not None])

            if len(vv) == 0:
                vv = "\\color{white}$N$\\color{black}"

            return "\\newcommand{%c%s}{%s}\n" % \
                ("\\", k.replace('_','').replace('-',''),
                       vv.replace('_','').replace('-', ''))

        def get_commands(j, prefix):
            ret = []
            print j
            for k, v in j.items():
                if v is None:
                    continue
                elif type(v) == dict:
                    ret += get_commands(v, prefix + k)
                else:
                    ret.append(get_new_command(prefix + k, v))
            return ret

        prev_dir = os.getcwd()
        os.chdir(self.config["pdf-dir"])
        
        f = open("macros.tex", 'w')
        lines = get_commands(json_value, "demo")
        f.writelines(lines)
        f.close()

        for _ in range(3):
            proc = subprocess.Popen(['pdflatex', 'main.tex'])
        
        os.chdir(prev_dir)
        shutil.move(os.path.join(self.config["pdf-dir"], "main.pdf"),
                    filename)

        webbrowser.open_new(filename)

    def save_json(self, json_value, filename):
        f = open(filename, 'w')
        json.dump(json_value, f, indent=2)
        f.close()
