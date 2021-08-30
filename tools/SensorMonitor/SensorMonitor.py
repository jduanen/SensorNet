#!/usr/bin/env python3
'''
SensorMonitor -- tool for monitoring SensorNet sensors
'''

import argparse
from datetime import datetime
import logging
import os
from queue import Queue
import re
import sys
import time

import paho.mqtt.client as mqtt


DEFAULTS = {
    'logLevel': "INFO",  #"DEBUG"  #"WARNING",
    'mqttBroker': "localhost",
}

MQTT_TOPIC_BASE = "/sensors/#"


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
        self.msgQ = Queue()
        self.running = True
        self.client.loop_start()

    def _onMessage(self, client, userData, message):
        logging.debug(f"msg: {message}")
        msg = str(message.payload.decode("utf-8"))
        self.msgQ.put(message.topic)

    def run(self):
        #### TODO make this run in a separate thread
        logging.info("Starting")
        while self.running:
            msg = self.msgQ.get(block=True)
            now = datetime.now().isoformat()
            logging.info(msg)
            parts = msg.split('/')
            if len(parts) < 5 or len(parts) > 6 or parts[1] != 'sensors' or parts[-1] not in ('cmd', 'data'):
                logging.warning(f"Unrecognized message: {msg}")
                continue
            self.appls[parts[2]] = now
            self.devices[parts[-2]] = now
            print("APPLS:", self.appls)
            print("DEVS:", self.devices)
        logging.info("Stopped")

    def stop(self):
        logging.info("Exiting")
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
    mon = SensorMonitor(options.mqttBroker)
    mon.run()
    input()
    mon.lastDeviceReports()
    mon.lastApplicationReports()
    mon.stop()


def getOpts():
    usage = f"Usage: {sys.argv[0]} [-v] [-L <logLevel>] [-l <logFile>] " + \
      "[-m <mqttHost>]"
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "-a", "--append", action="store_true", default=False,
        help="Append to existing samples file")
    ap.add_argument(
        "-L", "--logLevel", action="store", type=str,
        default=DEFAULTS['logLevel'],
        choices=["DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"],
        help="Logging level")
    ap.add_argument(
        "-l", "--logFile", action="store", type=str,
        help="Path to location of logfile (create it if it doesn't exist)")
    ap.add_argument(
        "-m", "--mqttHost", action="store", type=str,
        default=DEFAULTS['mqttBroker'],
        help="Hostname for where the MQTT broker is running")
    ap.add_argument(
        "-v", "--verbose", action="count", default=0,
        help="Enable printing of debug info")
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

    opts.mqttBroker = DEFAULTS['mqttBroker']

    if opts.verbose:
        print(f"    MQTT Broker:     {opts.mqttBroker}")
    return opts


if __name__ == '__main__':
    opts = getOpts()
    r = run(opts)
    sys.exit(r)
