#!/usr/bin/env python3
'''
SensorNet Command-Runner Tool

Tool that publishes a command message to the specified SensorNet devices.

This takes as an input a specification of the devices to which the given
 command is to be sent. This specification can take the form of a list of
 (zero or more) device "nicknames," as defined the the given devices info
 file.  This file is common to all of the SensorNet tools and contains the
 name, description, MAC address, and nickname of each known SensorNet device.
 If no device nicknames are given, then all of the devices in the devices
 info file will be republished.
A switch also allows a "multicast" message to be sent that all devices that
 share the topic prefix will receive -- e.g., "/sensors", "/displays",
 "/sensors/Radiation", etc.

This allows arbitrary commands to be sent, including both get and set commands
 (i.e., commands of the form "var" stimulate the device to emit the current
 value of "var", and commands of the form "var=val" will cause the targeted
 device(s) to set their internal variable "var" to "val" and publish a message
 with the new value of "var").
Commands include those supported by the SensorNet library's core subscription
 handler (e.g., "schema", "version", "RSSI", "reset"), as well as commands that
 are device-specific.

 '''

import argparse
from datetime import datetime
import json
import logging
import os
import sys

import paho.mqtt.client as mqtt
from queue import Queue, Empty

from SensorNet import SensorNet, SubTopic, SUB_TOPICS


DEFAULTS = {
    'logLevel': "INFO",  #"DEBUG"  #"WARNING",
    'brokerHost': "localhost",
    'deviceFile': "./devices.yml",
    'command': "schema",
    'timeout': 10000 # 30000  # 30 sec timeout
}

running = True
MQTT_TOPIC = "/sensors/#"
msgQ = Queue()


def onMessage(client, userData, message):
    #### TODO figure out how to deal with TZ properly
    try:
        msg = str(message.payload.decode("utf-8"))
        logging.debug(f"Received msg: {msg}")
        if message.topic.split('/')[-1] == "response":
            #### TODO use message.timestamp instead?
            logging.debug(f"Queued msg: {msg}")
            msgQ.put(f"{datetime.now().isoformat()},{message.topic},{msg}")
    except Exception as ex:
        logging.warning(f"Message receive error: {ex}")
        return


def run(options):
    client = mqtt.Client("SensorNet Command-Runner")
    client.enable_logger(logging.getLogger(__name__))
    if client.connect(options.brokerHost):
        logging.error(f"Failed to connect to MQTT broker '{options.brokerHost}'")
        sys.exit(1)
    client.on_message = onMessage
    result, msgId = client.subscribe(MQTT_TOPIC)
    if result:
        logging.error(f"Failed to subscribe to topic '{MQTT_TOPIC}'")
        sys.exit(1)
    client.loop_start()

    if options.multicast:
        # issue multicast command message
        cmdTopic = f"{options.multicast}/cmd"
        logging.info(f"Issuing command to all devices with topic prefix: {cmdTopic}")
        client.publish(cmdTopic, payload=options.command)

        # wait for timeout period and gather responses
        devicesResponded = set([])
        timeoutTime = datetime.now().timestamp() + (options.timeout / 1000)
        while datetime.now().timestamp() < timeoutTime:
            try:
                msg = msgQ.get(block=False)
            except Empty:
                #### FIXME yield
                continue
            except Exception as ex:
                logging.error(f"Message queue get failure: {ex}")
                break
            msgParts = msg.split(',')
            macAddr = msgParts[1].split('/')[-2]
            if (msgParts[2] == "Startup"):
                devicesResponded.add(macAddr)
                logging.info(f"Got response message from : {macAddr}; response: {msgParts[-1]}")
    else:
        # issue command messages to all of the selected devices
        devicesMessaged = set([])
        devMap = {}
        for devName in options.deviceNames:
            devInfo = options.sn.getDeviceInfo(devName)
            devMap[devInfo['MACaddress']] = devName
            logging.info(f"Issuing command '{options.command}' to device '{devName}' -- {devInfo}")
            cmdTopic = options.sn.buildTopic(SubTopic.COMMAND, devName)
            client.publish(cmdTopic, payload=options.command)
            devicesMessaged.add(devInfo['MACaddress'])

        # wait for response messages from all the selected devices, or timeout
        devicesResponded = set([])
        timeoutTime = datetime.now().timestamp() + (options.timeout / 1000)
        while devicesResponded != devicesMessaged:
            if datetime.now().timestamp() > timeoutTime:
                break
            try:
                msg = msgQ.get(block=False)
            except Empty:
                #### FIXME yield
                continue
            except Exception as ex:
                logging.error(f"Message queue get failure: {ex}")
                break
            msgParts = msg.split(',')
            macAddr = msgParts[1].split('/')[-2]
            if macAddr in devicesMessaged:
                devicesResponded.add(macAddr)
                logging.info(f"Got response message from : {macAddr}; response: {msgParts[-1]}")

    client.loop_stop()
    if options.verbose:
        print(f"Responding devices: {[devMap[d] for d in devicesResponded]}")
    if options.deviceNames:
        if devicesResponded != devicesMessaged:
            unresponsive = [devMap[d] for d in (devicesMessaged - devicesResponded)]
            logging.warning(f"Did not receive response messages from all targeted devices; missing: {unresponsive}")
            sys.exit(1)
    logging.info("Exiting")

def getOpts():
    usage = f"Usage: {sys.argv[0]} [-v] [-L <logLevel>] [-l <logFile>] " + \
        "[-b <brokerHost>] [-d <deviceFile>] [-t <timeout>] " + \
        "[-c <command>] [-m <topic>] {<devNickname>}*"
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "-b", "--brokerHost", action="store", type=str,
        default=DEFAULTS['brokerHost'],
        help="Hostname for where the MQTT broker is running")
    ap.add_argument(
        "-c", "--command", action="store", type=str,
        default=DEFAULTS['command'],
        help="Command string to publish (in the form of 'var' or 'var=val'")
    ap.add_argument(
        "-d", "--deviceFile", action="store", type=str,
        default=DEFAULTS['deviceFile'],
        help="Path to YAML file containing the devices to monitor")
    ap.add_argument(
        "-L", "--logLevel", action="store", type=str,
        default=DEFAULTS['logLevel'],
        choices=["DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"],
        help="Logging level")
    ap.add_argument(
        "-l", "--logFile", action="store", type=str,
        help="Path to location of logfile (create it if it doesn't exist)")
    ap.add_argument(
        "-m", "--multicast", action="store", type=str,
        help="Topic prefix to which command is to be published")
    ap.add_argument(
        "-t", "--timeout", action="store", type=int,
        default=DEFAULTS['timeout'],
        help="Number of msec to wait for all the startup messages before bailing (<1 means wait forever)")
    ap.add_argument(
        "-v", "--verbose", action="count", default=0,
        help="Enable printing of debug info")
    ap.add_argument(
        "deviceNames", action="store", type=str, nargs='*',
        help="Nickname(s) of device(s) to be targeted (defaults to all if not given)")
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

    if not os.path.exists(opts.deviceFile):
        logging.error(f"Invalid device info file: {opts.deviceFile}")
        sys.exit(1)

    opts.sn = SensorNet(opts.deviceFile)

    if opts.multicast:
        if opts.deviceNames:
            logging.warning("All devices selected, the given specific device(s) are ignored")
            opts.deviceNames = None;
    else:
        if opts.deviceNames:
            missingDevices = set(opts.deviceNames) - set(opts.sn.getDevices().keys())
            if missingDevices:
                logging.error(f"Devices {list(missingDevices)} not found in {opts.deviceFile}")
                sys.exit(1)
        else:
            opts.deviceNames = opts.sn.getDevices().keys()

    opts.timeout = opts.timeout if opts.timeout >= 0 else 0

    if opts.verbose:
        print(f"    MQTT broker:          {opts.brokerHost}")
        print(f"    Devices info file:    {opts.deviceFile}")
        if opts.timeout:
            print(f"    Timeout:              {opts.timeout} msec")
        else:
            print(f"    No timeout -- wait forever")
        if opts.logFile:
            print(f"    Logging to file:      {opts.logFile}")
        print(f"    Command to publish:   {opts.command}")
        if opts.multicast:
            print(f"    Targeting all devices with topic prefix: '{opts.multicast}'")
        else:
            print(f"    Targeting devices:    {opts.deviceNames}")
            if opts.verbose > 1:
                opts.devs = {name: opts.sn.getDeviceInfo(name) for name in opts.deviceNames}
                print(f"    Targeting devices:    {json.dumps(opts.devs, indent=4, sort_keys=True)}")
    return opts


if __name__ == '__main__':
    opts = getOpts()
    r = run(opts)
    sys.exit(r)
