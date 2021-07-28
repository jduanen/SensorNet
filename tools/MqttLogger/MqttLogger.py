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
    #### TMP TMP TMP
    msg = str(message.payload.decode("utf-8"))
    logging.debug(f"msg: {message}")
    #### TODO figure out how to deal with TZ properly
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
                      os.path.join(options.samplesPath, "archive")) as f:
        while running:
            msg = msgQ.get(block=True)
            f.write(msg + "\n")
    client.loop_stop()
    logging.info("Exiting")


def getOpts():
    usage = f"Usage: {sys.argv[0]} [-v] [-L <logLevel>] [-l <logFile>] " + \
      "[-f] <samplesPath>"
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "-f", "--force", action="store_true", default=False,
        help="Force overwrite of samples file")
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
        if opts.force:
            logging.warning("Overwriting samples file '{opts.samplesFile}'")
        else:
            logging.error("Samples file '{opts.samplesFile}' exists, use '-f' to overwrite")
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









'''
def on_message(client, userdata, msg):
...     print(msg.topic+" "+str(msg.payload))
client = mqtt.Client()
client.subscribe("/sensors/#")
>>> client.on_message = on_message
 client.is_connected()
True
>>> client.loop_forever()
from random import randrange, uniform
import time

mqttBroker ="mqtt.eclipseprojects.io" 

client = mqtt.Client("Temperature_Inside")
client.connect(mqttBroker) 

while True:
    randNumber = uniform(20.0, 21.0)
    client.publish("TEMPERATURE", randNumber)
    print("Just published " + str(randNumber) + " to topic TEMPERATURE")
    time.sleep(1)

def on_message(client, userdata, message):
    print("received message: " ,str(message.payload.decode("utf-8")))

mqttBroker ="mqtt.eclipseprojects.io"

client = mqtt.Client("Smartphone")
client.connect(mqttBroker) 

client.loop_start()

client.subscribe("TEMPERATURE")
client.on_message=on_message 

time.sleep(30)
client.loop_stop()
'''
