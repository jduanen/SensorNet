#!/usr/bin/env python3
"""
SensorNet Utilities Library
"""

from datetime import datetime
import json
import logging
import os
import yaml
from yaml import Loader


class SensorNet():
    def __init__(self, path):
        """
        """
        self.path = path
        with open(path, "r") as f:
            self.devices = yaml.load(f, Loader=Loader)
            #### TODO validate file contents

    def getDevices(self):
        return self.devices

    def getDeviceInfo(self, nickname):
        return self.devices[nickname]

    def buildDataTopic(self, nickname):
        pass

    def buildCommandTopic(self, nickname, command, value=None):
        pass

    def parseEvent(self, event):
        pass
