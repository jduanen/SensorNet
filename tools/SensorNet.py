#!/usr/bin/env python3
"""
SensorNet Utilities Library
"""

import re
import sys
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

        '''
        for addr in opts.deviceAddrs:
            if not re.match("[0-9a-f]{2}([-:]?)[0-9a-f]{2}(\\1[0-9a-f]{2}){4}$", addr.lower()):
                logging.error(f"Invalid device address: {addr}")
                sys.exit(1)
        '''
        self.nicknames = {info['MACaddress']: name for name, info in self.devices.items()}

    def getDevices(self):
        """????
        """
        return self.devices

    def getDeviceInfo(self, nickname):
        """????
        """
        return self.devices[nickname]

    def getNickname(self, macAddr):
        """????
        """
        return self.nicknames[macAddr]

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

    def parseSample(self, sampleParts):
        """????
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
    dataTopic = sn.buildDataTopic('b1')
    assert dataTopic == "/sensors/sensorB/12:34:56:78:9a:de/data", f"Incorrect data topic: {dataTopic}"
    cmdTopic = sn.buildCommandTopic('c', "RSSI")
    assert cmdTopic == ("/sensors/sensorC/12:34:56:78:9a:12/cmd", "RSSI"), f"Incorrect command topic: {cmdTopic}"
    cmdTopic = sn.buildCommandTopic('a', "rate=12345")
    assert cmdTopic == ("/sensors/sensorA/12:34:56:78:9a:bc/cmd", "rate=12345"), f"Incorrect command topic: {cmdTopic}"
    print("All tests: PASSED")
