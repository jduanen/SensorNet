#!/usr/bin/env python3
'''
SensorMonitor -- tool for monitoring SensorNet sensors
'''

#### TODO add events for late devices or values out of range
#### TODO add feature to allow executing script on events

import argparse
from datetime import datetime
import json
import logging
import os
import signal
import subprocess
import sys
import time
import yaml
from yaml import Loader

import paho.mqtt.client as mqtt


DEFAULTS = {
    'logLevel': "INFO",  #"DEBUG"  #"WARNING",
    'mqttBroker': "localhost",
    'reportInterval': 60*60  # one report per hour
}

MQTT_TOPIC_BASE = "/sensors/#"

running = True

class SensorMonitor():
    """
    """
    def __init__(self, mqttBroker):
        self.client = mqtt.Client("SensorMonitor listener")
        self.client.enable_logger(logging.getLogger(__name__))
        if self.client.connect(mqttBroker):
            logging.error(f"Failed to connect to MQTT broker '{mqttBroker}'")
            raise Exception("Failed to connect to MQTT broker")
        self.client.on_message = self._onMessage
        result, msgId = self.client.subscribe(MQTT_TOPIC_BASE)
        if result:
            logging.error(f"Failed to subscribe to MQTT topic '{MQTT_TOPIC_BASE}'")
            raise Exception("Failed to subscribe to MQTT topic")
        self.devices = {}
        self.appls = {}
        self.client.loop_start()

    def _onMessage(self, client, userData, message):
        now = datetime.now().isoformat()
        print(message.topic)
        parts = message.topic.split('/')
        if len(parts) < 5 or len(parts) > 6 or parts[1] != 'sensors' or parts[-1] not in ('cmd', 'data'):
            logging.warning(f"Unrecognized message: {message}")
        else:
            self.appls[parts[2]] = now
            self.devices[parts[-2]] = now
        print("APPLS:", self.appls)
        print("DEVS:", self.devices)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.client.loop_stop()

    def lastDeviceReports(self):
        return self.devices

    def lastDeviceReport(self, device):
        return self.devices[device]

    def lastApplicationReports(self):
        return self.appls

    def lastApplicationReport(self, appl):
        return self.applis[appl]


def run(options):
    if options.status:
        lastSeen = {d['MACaddress']: None for d in options.devs}
        proc = subprocess.Popen(['tac', options.status], stdout=subprocess.PIPE)
        for line in proc.stdout.readlines():
            parts = line.decode('utf-8').split(',')
            timestamp = parts[0]
            macAddr = parts[1].split('/')[-2]
            if macAddr in lastSeen.keys() and not lastSeen[macAddr]:
                lastSeen[macAddr] = timestamp
            if all([t for t in lastSeen.values()]):
                break
        if options.verbose:
            print("    Last sample time:")
        json.dump(lastSeen, sys.stdout, indent=4)
        print("")
    else:
        with SensorMonitor(options.mqttBroker) as mon:
            def signalHandler(sig, frame):
                print("SIG", mon.lastDeviceReports())
            signal.signal(signal.SIGUSR1, signalHandler)
            while running:
                now = datetime.now().isoformat()
                lastSamples = mon.lastDeviceReports()
                for devArgs in options.devs:
                    print(lastSamples[devArgs['MACaddress']], now)
                time.sleep(options.reportInterval)


def getOpts():
    usage = f"Usage: {sys.argv[0]} [-v] [-L <logLevel>] [-l <logFile>] " + \
      "[-m <mqttHost>] [-r <reportInterval>] [-s <samplesFile>] <deviceFile>"
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
        "-r", "--reportInterval", action="store", type=int,
        default=DEFAULTS['reportInterval'],
        help="Number of seconds in between reports")
    ap.add_argument(
        "-v", "--verbose", action="count", default=0,
        help="Enable printing of debug info")
    ap.add_argument(
        "-s", "--status", action="store", type=str,
        help="Print time of last sample for each device found in the given samples file")
    ap.add_argument(
        "deviceFile", action="store", type=str,
        help="Path to YAML file containing the devices to monitor (tuples with MAC address and sample interval in seconds)")
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

    with open(opts.deviceFile, "r") as f:
        opts.devs = yaml.load(f, Loader=Loader)
        #### TODO validate file contents

    if opts.status and not os.path.exists(opts.status):
        logging.error("Invalid path to samples file: {opts.status}")
        sys.exit(1)

    if opts.verbose:
        if opts.status:
            print(f"    Report last sample time for each device")
            print(f"    Samples file: {opts.status}")
        else:
            print(f"    Monitor devices")
            print(f"    MQTT Broker:       {opts.mqttBroker}")
            print(f"    Report Interval:   {opts.reportInterval}")
        print(f"    Monitored Devices: {[d['MACaddress'] for d in opts.devs]}")
    return opts


if __name__ == '__main__':
    opts = getOpts()
    r = run(opts)
    sys.exit(r)
