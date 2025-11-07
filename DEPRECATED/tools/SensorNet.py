#!/usr/bin/env python3
"""
SensorNet Utilities Library
"""

from enum import Enum
import logging
import re
import sys
import yaml
from yaml import Loader


PREFIX = "/sensors"


class SubTopic(Enum):
    DATA = 0
    COMMAND = 1
    RESPONSE = 2
    ERROR = 3
    STARTUP = 4


SUB_TOPICS = {
    SubTopic.DATA: "data",
    SubTopic.COMMAND: "cmd",
    SubTopic.RESPONSE: "response",
    SubTopic.ERROR: "error",
    SubTopic.STARTUP: "startup",
}

SUB_TOPICS_MAP = {v: k for k, v in SUB_TOPICS.items()}


class SensorNet():
    def __init__(self, path):
        """????
        """
        self.path = path
        with open(path, "r") as f:
            self.devices = yaml.load(f, Loader=Loader)
            #### TODO validate file contents

        '''
        for addr in opts.deviceAddrs:
            if not re.match("[0-9a-f]{2}([-:]?)[0-9a-f]{2}(\\1[0-9a-f]{2}){4}$", addr.lower()):
                logging.error(f"Invalid device address: {addr}")
                sys.exit(1)
        '''
        self.nicknames = {info['MACaddress']: name for name, info in self.devices.items()}

    def getDevices(self):
        """Return ????
        """
        return self.devices

    def getDeviceInfo(self, nickname):
        """Return information about a given device.

        Inputs
          nickname: short string uniquely identifying a device
        Returns
          dict with information about the given device
        """
        return self.devices[nickname]

    def getNickname(self, macAddr):
        """Return the device nickname associated with a given WiFi MAC address

        Inputs
          macAddr: string in form of six hex bytes separated by colons
        Returns
          short string identifier for the device with the given MAC address
        """
        return self.nicknames[macAddr]

    def buildTopic(self, subTopic, nickname):
        """Create a topic of a given type for a given device

        Inputs
          subTopic: member of SubTopic enum class
          nickname: short string uniquely identifying a device
        Returns
          string containing desired topic
        """
        assert isinstance(subTopic, SubTopic), "subTopic arg must be a member of the SubTopic enum class"
        applName = self.devices[nickname]['application']
        macAddr = self.devices[nickname]['MACaddress']
        return f"{PREFIX}/{applName}/{macAddr}/{SUB_TOPICS[subTopic]}"

    def parseSample(self, sampleParts):
        """Parse an sample/event string logged by a SensorNet device

        Inputs
          sampleParts: list of the comma-separated parts of a sample line
           (i.e., timestamp, topic, payload)
        Returns
          dict with each of the components of the given sample broken out
        """
        timestamp = sampleParts[0]
        topic = sampleParts[1]
        subparts = topic.split('/')
        if subparts[0] != "" or subparts[1] != PREFIX.strip('/'):
            logging.error(f"Unrecognized topic: {sampleParts}")
            raise Exception("Unrecognized topic")
        macAddr = subparts[-2]
        appl = "/".join(subparts[2:-2])
        values = None
        if subparts[-1] == "data":
            sampleType = "data"
            subType = None
            values = sampleParts[2:]
        elif subparts[-1] == "cmd":
            sampleType = "cmd"
            if sampleParts[2] == "Startup":
                subType = "startup"
                values = {
                    'HW': sampleParts[3],
                    'applName': sampleParts[4],
                    'version': sampleParts[5],
                    'schema': sampleParts[6:-1],
                    'RSSI': sampleParts[-1]
                }
            elif sampleParts[2].find('=') > 1:
                subType = "setCmd"
                assignment = sampleParts[2].split('=')
                values = {'cmd': assignment[0], 'value': assignment[1]}
            else:
                subType = "getCmd"
                values = {'cmd': sampleParts[2]}
        else:
            logging.error(f"Unrecognized sample type: {sampleParts}")
            raise Exception("Unrecognized sample type")
        return {
            'timestamp': timestamp,
            'topic': topic,
            'application': appl,
            'macAddr': macAddr,
            'nickname': self.nicknames[macAddr],
            'type': sampleType,
            'subtype': subType,
            'values': values
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
    dataTopic = sn.buildTopic(SubTopic.DATA, 'b1')
    assert dataTopic == "/sensors/sensorB/12:34:56:78:9a:de/data", f"Incorrect data topic: {dataTopic}"
    cmdTopic = sn.buildTopic(SubTopic.COMMAND, 'c')
    assert cmdTopic == "/sensors/sensorC/12:34:56:78:9a:12/cmd", f"Incorrect command topic: {cmdTopic}"
    print("All tests: PASSED")
