#!/usr/bin/env python3
'''
SensorManager -- tool for managing SensorNet sensors
'''

import argparse
import logging
import os
import re
import sys
import time

import paho.mqtt.client as mqtt


DEFAULTS = {
    'logLevel': "INFO",  #"DEBUG"  #"WARNING",
    'mqttBroker': "localhost",
}

COMMANDS = ('version', 'RSSI', 'rate')

MQTT_TOPIC_BASE = "/sensors/#"

running = True

class SensorManager():
    """
    """
    def __init__(self, mqttBroker):
        self.client = mqtt.Client("SensorMonitor listener")
        self.client.enable_logger(logging.getLogger(__name__))
        if self.client.connect(mqttBroker):
            logging.error(f"Failed to connect to MQTT broker '{mqttBroker}'")
            raise Exception("Failed to connect to MQTT broker")
        self.client.on_message = self._onMessage
        _, _ = self.client.subscribe(MQTT_TOPIC_BASE)
        self.client.loop_start()

    def _onMessage(self, client, userData, message):
        print(message.topic, client, userData, dir(message))
        print(message.info)
        print(message.state)
        print(message.timestamp)
        parts = message.topic.split('/')
        if len(parts) < 5 or len(parts) > 6 or parts[1] != 'sensors' or parts[-1] not in ('cmd', 'data'):
            logging.warning(f"Unrecognized message: {message}")
        else:
            print("TBD")

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.client.loop_stop()

    def _issueCommand(self, addr, cmd):
        topic = "????"  #### FIXME
        result, msgId = self.client.subscribe(topic)
        if result:
            logging.error(f"Failed to subscribe to MQTT topic '{topic}'")
            raise Exception("Failed to subscribe to MQTT topic")
        return msgId

    def getCommand(self, cmd):
        time.sleep(15)
        return "TBD"

    def setCommand(self, cmd, val):
        time.sleep(15)
        return "TBD"


def run(options):
    with SensorManager(options.mqttBroker) as mgr:
        for devAddr in options.deviceAddrs:
            if options.val:
                res = mgr.setCommand(options.cmd, options.val)
            else:
                res = mgr.getCommand(options.cmd)
            print(f"{devAddr}: {options.cmd} = {res}")

def getOpts():
    usage = f"Usage: {sys.argv[0]} [-v] [-L <logLevel>] [-l <logFile>] " + \
      "[-m <mqttHost>] <cmd>{=<val>} {<deviceAddr>}+"
    ap = argparse.ArgumentParser()
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
        "deviceAddrs", action="store", type=str, nargs='+',
        help="WiFi MAC address of the device to be managed")
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

    parts = opts.command.split('=')
    opts.cmd = parts[0]
    if opts.cmd not in COMMANDS:
        logging.error(f"Invalid command '{opts.cmd}'")
        sys.exit(1)
    opts.val = parts[1] if len(parts) > 1 else None

    for addr in opts.deviceAddrs:
        if not re.match("[0-9a-f]{2}([-:]?)[0-9a-f]{2}(\\1[0-9a-f]{2}){4}$", addr.lower()):
            logging.error(f"Invalid device address: {addr}")
            sys.exit(1)

    if opts.verbose:
        print(f"    MQTT Broker: {opts.mqttBroker}")
        print(f"    Command:     {opts.cmd}")
        if opts.val:
            print(f"    Value:       {opts.val}")
        print(f"    Devices:     {opts.deviceAddrs}")
    return opts


if __name__ == '__main__':
    opts = getOpts()
    r = run(opts)
    sys.exit(r)
