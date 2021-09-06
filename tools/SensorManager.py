#!/usr/bin/env python3
'''
SensorManager -- tool for managing SensorNet sensors
'''

import argparse
import json
import logging
import os
from queue import Queue
import re
import sys
import time

import paho.mqtt.client as mqtt

from SensorNet import SensorNet, PubType, SUB_TOPICS


DEFAULTS = {
    'logLevel': "INFO",  #"DEBUG"  #"WARNING",
    'mqttBroker': "localhost",
    'deviceFile': "./devices.yml"
}

COMMANDS = ('version', 'RSSI', 'rate')

MQTT_TOPIC_BASE = "/sensors/#"

running = True

class SensorManager():
    """????
    """
    def __init__(self, mqttBroker):
        self.mqttBroker = mqttBroker
        self.msgQ = Queue()
        self.client = mqtt.Client("SensorMonitor listener")
        self.client.enable_logger(logging.getLogger(__name__))
        if self.client.connect(mqttBroker):
            logging.error(f"Failed to connect to MQTT broker '{mqttBroker}'")
            raise Exception("Failed to connect to MQTT broker")
        self.client.on_message = self._onMessage
        self.client.loop_start()

    def _onMessage(self, client, userData, message):
        parts = message.topic.split('/')
        if len(parts) < 5 or len(parts) > 6 or parts[1] != 'sensors' or parts[-1] not in SUB_TOPICS:
            logging.warning(f"Unrecognized message: {message}")
        topic = "/".join(parts)
        self.msgQ.put((topic, message.payload.decode("utf-8"), message.timestamp))

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.client.loop_stop()

    def _getResponse(self, topic, retries=2, timeout=3):
        """????
        """
        tmp = topic.split('/')
        tmp[-1] = "response"
        responseTopic = "/".join(tmp)
        result, msgId = self.client.subscribe(responseTopic)
        if result:
            logging.error(f"Failed to subscribe to MQTT topic '{responseTopic}'")
            raise Exception("Failed to subscribe to MQTT topic")
        logging.debug(f"Subscribed to: {responseTopic}")

        payload = None
        for _ in range(retries):
            response = None
            if self.msgQ.empty():
                try:
                    response = self.msgQ.get(block=True, timeout=timeout)
                    if response[0] != responseTopic:
                        logging.warning(f"Response mismatch: {response}")
                        continue
                except:
                    logging.warning(f"Get MQTT topic '{responseTopic}' timed out, retrying...")
                    time.sleep(timeout)
                    continue
            else:
                response = self.msgQ.get()
                if response[0] != responseTopic:
                    logging.warning(f"Mismatched response: {response}")
                    continue
            payload = response[1] if response else None
        result, msgId = self.client.unsubscribe(responseTopic)
        if result:
            logging.warning(f"Failed to unsubscribe to MQTT topic '{responseTopic}'")
        else:
            logging.debug(f"Unsubscribed from: {responseTopic}")
        return payload

    def issueCommand(self, cmdTopic, msg, retries=3, timeout=2):
        """????
        """
        self.client.publish(cmdTopic, payload=msg)
        response = None
        for _ in range(retries):
            response = self._getResponse(cmdTopic, retries, timeout)
            if not response:
                continue
            break
        return response


def run(options):
    sn = SensorNet(options.deviceFile)
    results = {}
    with SensorManager(options.mqttBroker) as mgr:
        msg = f"{options.cmd}={options.val}" if options.val else options.cmd
        for devName in options.deviceNames:
            results[devName] = {}
            cmdTopic = sn.buildTopic(PubType.COMMAND, devName)
            print(f"{devName}: {cmdTopic}, {msg}")
            results[devName] = mgr.issueCommand(cmdTopic, msg)
    json.dump(results, sys.stdout, indent=4)
    print("")


def getOpts():
    usage = f"Usage: {sys.argv[0]} [-v] [-L <logLevel>] [-l <logFile>] " + \
      "[-m <mqttHost>] [-d <deviceFile>] <cmd>{=<val>} {<devNickname>}+"
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "-d", "--deviceFile", action="store", type=str,
        default=DEFAULTS['deviceFile'],
        help="Path to file containing device information")
    ap.add_argument(
        "-L", "--logLevel", action="store", type=str,
        default=DEFAULTS['logLevel'],
        choices=["DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"],
        help="Logging level")
    ap.add_argument(
        "-l", "--logFile", action="store", type=str,
        help="Path to location of logfile (create it if it doesn't exist)")
    ap.add_argument(
        "-m", "--mqttBroker", action="store", type=str,
        default=DEFAULTS['mqttBroker'],
        help="Hostname for where the MQTT broker is running")
    ap.add_argument(
        "-v", "--verbose", action="count", default=0,
        help="Enable printing of debug info")
    ap.add_argument(
        "command", action="store", type=str,
        help="Command to execute (with optional value -- e.g., <cmd>=<val>)")
    ap.add_argument(
        "deviceNames", action="store", type=str, nargs='+',
        help="Nickname of a device to be managed")
    opts = ap.parse_args()

    if opts.logFile:
        logging.basicConfig(filename=opts.logFile,
                            format='%(asctime)s %(levelname)-8s %(message)s',
                            datefmt='%Y-%m-%d %H:%M:%S',
                            level=opts.logLevel)
    else:
        logging.basicConfig(level=opts.logLevel,
                            format='%(asctime)s %(levelname)-8s %(message)s',
                            datefmt='%Y-%m-%d %H:%M:%S')
    if not opts.command:
        logging.error("Must give command using '-c'")
        sys.exit(1)

    if not os.path.exists(opts.deviceFile):
        logging.error(f"Invalid device info file: {opts.deviceFile}")
        sys.exit(1)

    sn = SensorNet(opts.deviceFile)
    missingDevices = set(opts.deviceNames) - set(sn.getDevices().keys())
    if missingDevices:
        logging.error(f"Devices {missingDevices} not found in {opts.deviceFile}")
        sys.exit(1)

    parts = opts.command.split('=')
    opts.cmd = parts[0]
    if opts.cmd not in COMMANDS:
        logging.error(f"Invalid command '{opts.cmd}'")
        sys.exit(1)
    opts.val = parts[1] if len(parts) > 1 else None

    if opts.verbose:
        print(f"    MQTT Broker:  {opts.mqttBroker}")
        print(f"    Devices File: {opts.deviceFile}")
        print(f"    Command:      {opts.cmd}")
        if opts.val:
            print(f"    Value:        {opts.val}")
        if opts.verbose > 1:
            devs = {name: sn.getDeviceInfo(name) for name in opts.deviceNames}
            print(f"    Devices:      {json.dumps(devs, indent=4, sort_keys=True)}")
        else:
            print(f"    Devices:      {opts.deviceNames}")
    return opts


if __name__ == '__main__':
    opts = getOpts()
    r = run(opts)
    sys.exit(r)
