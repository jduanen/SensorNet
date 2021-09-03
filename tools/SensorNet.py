#!/usr/bin/env python3
"""
SensorNet Utilities Library
"""

import re
import yaml
from yaml import Loader


PREFIX = "/sensors"


class SensorNet():
    def __init__(self, path):
        """????
        """
        self.path = path
        with open(path, "r") as f:
            self.devices = yaml.load(f, Loader=Loader)
            #### TODO validate file contents

    def getDevices(self):
        """????
        """
        return self.devices

    def getDeviceInfo(self, nickname):
        """????
        """
        return self.devices[nickname]

    def buildDataTopic(self, nickname):
        """????
        """
        applName = self.devices[nickname]['application']
        macAddr = self.devices[nickname]['MACaddress']
        return f"{PREFIX}/{applName}/{macAddr}/data"

    def buildCommandTopic(self, nickname, command, value=None):
        """????
        """
        applName = self.devices[nickname]['application']
        macAddr = self.devices[nickname]['MACaddress']
        val = f"{command}={value}" if value else f"{command}"
        return (f"{PREFIX}/{applName}/{macAddr}/cmd", val)

    def parseEvent(self, event):
        """????
        """
        #### FIXME
        return {
            'timestamp': None,
            'topic': None,
            'application': None,
            'macAddr': None,
            'type': None,
            'values': None
        }

#
# TESTS
#
if __name__ == '__main__':
    sn = SensorNet("./exampleDevices.yml")
    devs = sn.getDevices()
    assert len(devs) == 4, f"Wrong number of devices"
    b2 = sn.getDeviceInfo('b2')
    assert set(b2.keys()) == set(['application', 'MACaddress', 'location']), f"Mismatched device fields"
    dataTopic = sn.buildDataTopic('b1')
    assert dataTopic == "/sensors/sensorB/12:34:56:78:9a:de/data", f"Incorrect data topic: {dataTopic}"
    cmdTopic = sn.buildCommandTopic('c', "RSSI")
    assert cmdTopic == ("/sensors/sensorC/12:34:56:78:9a:12/cmd", "RSSI"), f"Incorrect command topic: {cmdTopic}"
    cmdTopic = sn.buildCommandTopic('a', "rate=12345")
    assert cmdTopic == ("/sensors/sensorA/12:34:56:78:9a:bc/cmd", "rate=12345"), f"Incorrect command topic: {cmdTopic}"
    print("All tests: PASSED")
