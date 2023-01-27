#!/usr/bin/env python3
'''
PurpleAir -- tool for polling Purple Air sensors as SensorNet sensors

 Runs as a service on the host, polls nearby sensors, publishes data to the
  SensorNet MQTT server.
'''

import argparse
import ast
import json
import logging
import os
import random
import re
import signal
import socket
import subprocess
import sys
import threading
import time
import yaml
from yaml import Loader

import paho.mqtt.client as mqtt
from purpleair.sensor import Sensor

from SensorNet import SensorNet, SubTopic, SUB_TOPICS


DEFAULTS = {
    'logLevel': "INFO",  #"DEBUG"  #"WARNING",
    'mqttBroker': "localhost",
    'sensorsFile': "./sensors.yml"
}

DEF_SAMPLE_INTERVAL = 120 # 2mins between samples

MQTT_TOPIC_BASE = "/sensors/PurpleAir"

APP_VERSION = "1.0.0"


def getMacAddress(hostname):
    """Return the MAC address for the interface used to reach the given host

      Inputs:
        hostname: string name of the target host
      Returns: string form of MAC address of interface used to reach given host
    """
    ipAddr = socket.gethostbyname(hostname)
    match = re.search(r"^.* dev (.*?) .*$", str(subprocess.check_output(["ip", "route", "get", ipAddr])))
    assert match, f"Unable to find interface for {ipAddr}"
    intf = match.group(1)
    return subprocess.check_output(["cat", f"/sys/class/net/{intf}/address"]).strip().decode("utf-8")


class PurpleAir():
    """????
    """
    def __init__(self, sensors, mqttBroker, sampleInterval=DEF_SAMPLE_INTERVAL, retries=3):
        self.sensorsInput = sensors
        self.mqttBroker = mqttBroker
        self.sampleInterval = sampleInterval
        self.retries = retries

        self.sensorIds = list(sensors.keys())
        self.sensors = {sensorId: Sensor(sensorId, parse_location=True) for sensorId in self.sensorIds}
        for sensorId, sensor in self.sensors.items():
            if sensor.location_type != 'outside':
                logging.warning(f"Sensor '{sensorId}' not outside, removing this sensor from the list")
                del self.sensors[sensorId]
        self.secureRandom = random.SystemRandom()

        self.baseMsg = f"{MQTT_TOPIC_BASE}/{getMacAddress(self.mqttBroker)}"
        self.cmdTopic = f"{self.baseMsg}/{SUB_TOPICS[SubTopic.COMMAND]}"
        self.dataTopic = f"{self.baseMsg}/{SUB_TOPICS[SubTopic.DATA]}"
        RSSI = 0
        msgSpec = f"ID:int,Label:s,tempF:d3,pressure:f4.2,humidity:d3," + \
                  f"v:f3.2,v1:f3.2,v2:f3.2,v3:f3.2,v4:f3.2,v5:f3.2,v6:f3.2," + \
                  f"pm:f3.2,lastModified:d13,timeSinceModified:d"
        self.startupMsg = f"Startup,{socket.gethostname()},PurpleAir,{APP_VERSION},{msgSpec},{RSSI}"

        self.client = mqtt.Client("PurpleAir sensor proxy")
        self.client.enable_logger(logging.getLogger(__name__))
        if self.client.connect(self.mqttBroker):
            logging.error(f"Failed to connect to MQTT broker '{self.mqttBroker}'")
            raise Exception("Failed to connect to MQTT broker")
        result, msgId = self.client.subscribe(f"{self.cmdTopic}")
        if result:
            logging.error(f"Failed to subscribe to topic '{self.cmdTopic}'")
            raise Exception("Subscription failure")
        self.client.on_message = self._onMessage
        self.client.loop_start()

        self.running = threading.Event()
        self.running.clear()
        self.thread = threading.Thread(target=self.run, name="PurpleAir")

    def _onMessage(self, client, userData, message):
        parts = message.topic.split('/')
        print(f"MSG: {parts}")
        if len(parts) < 5 or len(parts) > 6 or parts[1] != 'sensors' or parts[-1] == SubTopic.COMMAND:
            logging.warning(f"Unrecognized message: {message}")
        msg = message.payload.decode("utf-8")
        #### TODO implement the command handler here
        print("TBD")

    def start(self):
        """????
        """
        if self.running.isSet():
            logging.debug("PurpleAir thread already running")
        else:
            logging.debug("Starting PurpleAir thread")
            self.running.set()
            self.thread.start()

    def shutdown(self):
        """????
        """
        logging.debug("Shutting down PurpleAir thread")
        self.running.clear()
        while self.isRunning():
            time.sleep(1)
        logging.debug("PurpleAir thread done")

    def isRunning(self):
        return self.thread.is_alive()

    def run(self):
        """????
        """
        self.client.publish(self.cmdTopic, payload=self.startupMsg)
        logging.info(self.startupMsg)
        RSSI = 0
        while self.running.isSet():
            tryNumber = 0
            while True:
                sensorId = self.secureRandom.choice(self.sensorIds)
                logging.debug(f"Sampling sensor id #{sensorId}")
                sensor = self.sensors[sensorId]
                if sensor.is_useful():
                    data = sensor.parent_data
                    stats = ast.literal_eval(data['Stats'])
                    assert stats['timeSinceModified'] < 180000, f"Sensor {sensorId} not seen for {stats['timeSinceModified']}"
                    msg = f"{data['ID']},{data['Label']},{data['temp_f']},{data['pressure']},{data['humidity']},"
                    msg += ",".join([str(s) for s in stats.values()]) + f",{RSSI}"
                    #### TODO publish sensor data
                    self.client.publish(self.dataTopic, payload=msg)
                    logging.info(msg)
                    break
                else:
                    logging.warning(f"Sensor '{sensorId}' is not useful, skipping")
                tryNumber += 1
                if tryNumber >= self.retries:
                    logging.warning(f"Failed to get good sample in {tryNumber} attempts, giving up")
                    break
            logging.debug(f"Sleep {self.sampleInterval} seconds before sampling again")
            time.sleep(self.sampleInterval)

    '''
    sn = SensorNet(options.deviceFile)
    results = {}
    with SensorManager(options.mqttBroker) as mgr:
        msg = f"{options.cmd}={options.val}" if options.val else options.cmd
        for devName in options.deviceNames:
            results[devName] = {}
            cmdTopic = sn.buildTopic(SubTopic.COMMAND, devName)
            logging.debug(f"{devName}: {cmdTopic}, {msg}")
            results[devName] = mgr.issueCommand(cmdTopic, msg)
    json.dump(results, sys.stdout, indent=4)
    print("")
    '''

    '''

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.client.loop_stop()

    def _getResponse(self, cmdTopic):
        """Get the corresponding result for a given command topic.
        """
        tmp = cmdTopic.split('/')
        tmp[-1] = "response"
        responseTopic = "/".join(tmp)
        result, msgId = self.client.subscribe(responseTopic)
        if result:
            logging.error(f"Failed to subscribe to MQTT topic '{responseTopic}'")
            raise Exception("Failed to subscribe to MQTT topic")
        logging.debug(f"Subscribed to: {responseTopic}")

        if self.msgQ.empty():
            try:
                response = self.msgQ.get(block=True, timeout=self.responseWaitTime)
                if response[0] != responseTopic:
                    logging.error(f"Response mismatch: {response} != {responseTopic}")
                    return None
            except Exception as ex:
                logging.error(f"Get MQTT topic '{responseTopic}' timed out")
                return None
        else:
            response = self.msgQ.get()
            if response[0] != responseTopic:
                logging.error(f"Mismatched response: {response} != {responseTopic}")
                return None
        payload = response[1] if response else None

        result, msgId = self.client.unsubscribe(responseTopic)
        if result:
            logging.warning(f"Failed to unsubscribe to MQTT topic '{responseTopic}'")
        else:
            logging.debug(f"Unsubscribed from: {responseTopic}")
        return payload

    def issueCommand(self, cmdTopic, msg, retries=3):
        """Issue a given command to a device and get the response.

          N.B. no response is expected when a 'reset' command is issued.

          Inputs
            cmdTopic: string with command topic for desired device
            msg: string in either the form '<cmd>' or '<cmd>=<val>'
          Returns
            Response message or None if none is received after 'retries' attempts
        """
        self.client.publish(cmdTopic, payload=msg)
        response = None
        if msg != "reset":
            for _ in range(retries):
                response = self._getResponse(cmdTopic)
                if not response:
                    logging.debug("No response, retrying...")
                    continue
                break
        return response
    '''

def run(options):
    pa = PurpleAir(options.sensors, options.mqttBroker)

    def shutdownHandler(signum, frame):
        logging.debug(f"Caught signal: {signum}")
        pa.shutdown()

    for s in ('TERM', 'HUP', 'INT'):
        sig = getattr(signal, 'SIG'+s)
        signal.signal(sig, shutdownHandler)

    pa.start()
    while pa.isRunning():
        time.sleep(10)
    pa.shutdown()


def getOpts():
    usage = f"Usage: {sys.argv[0]} [-v] [-L <logLevel>] [-l <logFile>] " + \
      "[-m <mqttHost>] [-s <sensorsFile>]"
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
        "-s", "--sensorsFile", action="store", type=str,
        default=DEFAULTS['sensorsFile'],
        help="Path to file containing identifiers for local Purple Air sensors")
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

    if not os.path.exists(opts.sensorsFile):
        logging.error(f"Invalid sensor file: {opts.sensorsFile}")
        sys.exit(1)

    with open(opts.sensorsFile, "r") as f:
        opts.sensors = yaml.load(f, Loader=Loader)

    #### TODO read and parse the sensors file -- yml with description/location and id

    if opts.verbose:
        print(f"    MQTT Broker:  {opts.mqttBroker}")
        print(f"    Sensors File: {opts.sensorsFile}")
        if opts.verbose > 1:
            print(f"    Sensors:      {json.dumps(opts.sensors, indent=4, sort_keys=True)}")
        else:
            print(f"    Sensors:      {list(opts.sensors.keys())}")
    return opts


if __name__ == '__main__':
    opts = getOpts()
    r = run(opts)
    sys.exit(r)
