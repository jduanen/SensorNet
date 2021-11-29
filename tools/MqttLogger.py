#!/usr/bin/env python3
'''
MqttLogger
'''

import argparse
from datetime import datetime
import logging
import os
import sys
import time

import paho.mqtt.client as mqtt
from queue import Queue

from RotatingFile import RotatingFile


DEFAULTS = {
    'logLevel': "INFO",  #"DEBUG"  #"WARNING",
    'samplesPath': "/home/jdn/Data/SensorNet",
    'samplesFilename': "sensornet.csv",
    'maxNumFiles': 10,
    'maxFileSize': 10000000,
    'mqttBroker': "localhost",
    'mqttTopic': "/sensors/#"
}

running = True
msgQ = Queue()

def onMessage(client, userData, message):
    logging.debug(f"msg: {message}")
    #### TODO figure out how to deal with TZ properly
    try:
        msg = str(message.payload.decode("utf-8"))
    except Exception as ex:
        logging.warning(f"Message receive error: {ex}")
        return
    #### TODO use message.timestamp instead?
    msgQ.put(f"{datetime.now().isoformat()},{message.topic},{msg}")


def run(options):
    client = mqtt.Client("SensorNet listener")
    client.enable_logger(logging.getLogger(__name__))
    if client.connect(options.mqttBroker):
        logging.error(f"Failed to connect to MQTT broker '{options.mqttBroker}'")
        sys.exit(1)
    client.on_message = onMessage
    result, msgId = client.subscribe(options.mqttTopic)
    if result:
        logging.error(f"Failed to subscribe to topic '{options.mqttTopic}'")
        sys.exit(1)
    client.loop_start()

    logging.info("Starting")
    with RotatingFile(options.samplesPath,
                      os.path.basename(options.samplesFile),
                      options.maxNumFiles,
                      options.maxFileSize,
                      os.path.join(options.samplesPath, "archive"),
                      options.append) as f:
        while running:
            msg = msgQ.get(block=True)
            f.write(msg + "\n")
    client.loop_stop()
    logging.info("Exiting")


def getOpts():
    usage = f"Usage: {sys.argv[0]} [-v] [-L <logLevel>] [-l <logFile>] " + \
      "[-s <samplesPath>]"
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
        "-v", "--verbose", action="count", default=0,
        help="Enable printing of debug info")
    ap.add_argument(
        "-s", "--samplesPath", action="store",
        type=str, default=DEFAULTS['samplesPath'],
        help="Path to directory where samples files are written")
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

    if not os.path.isdir(opts.samplesPath):
        logging.error(f"Samples files path '{opts.samplesPath}' doesn't exist")
        sys.exit(1)

    opts.samplesFile = os.path.join(opts.samplesPath, DEFAULTS['samplesFilename'])
    if os.path.exists(opts.samplesFile):
        if opts.append:
            logging.warning(f"Appending to existing samples file '{opts.samplesFile}'")
        else:
            logging.error(f"Samples file '{opts.samplesFile}' exists, use '-a' to append to it")
            sys.exit(1)


    opts.maxFileSize = DEFAULTS['maxFileSize']
    opts.maxNumFiles = DEFAULTS['maxNumFiles']

    opts.mqttBroker = DEFAULTS['mqttBroker']
    opts.mqttTopic = DEFAULTS['mqttTopic']

    if opts.verbose:
        print(f"    Samples File: {opts.samplesFile}")
        print(f"    MQTT Broker:  {opts.mqttBroker}")
        print(f"    MQTT Topic:   {opts.mqttTopic}")
    return opts


if __name__ == '__main__':
    opts = getOpts()
    r = run(opts)
    sys.exit(r)
