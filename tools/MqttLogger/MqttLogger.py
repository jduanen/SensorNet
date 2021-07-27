#!/usr/bin/env python3
'''
MqttLogger
'''

import argparse
import logging
import os
import sys
import time

import paho.mqtt.client as mqtt

from RotatingFile import RotatingFile


DEFAULTS = {
    'logLevel': "INFO",  #"DEBUG"  #"WARNING",
    'samplesPath': "/home/jdn/Data/SensorNet",
    'samplesFilename': "sensornet.csv",
    'maxNumFiles': 10,
    'maxFileSize': 10000000
}


def run(options):
    pass

def getOpts():
    usage = f"Usage: {sys.argv[0]} [-v] [-L <logLevel>] [-l <logFile>] " + \
      "<samplesPath>"
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

    if opts.verbose:
        print(f"    Samples File: {opts.samplesFile}")
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
import paho.mqtt.client as mqtt 
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
