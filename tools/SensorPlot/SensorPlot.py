#!/usr/bin/env python3
'''
SensorPlot: Gather and plot data from SensorNet log files
'''

import argparse
import json
import logging
import os
import sys

from plotters import PLOTTERS


DEFAULTS = {
    'logLevel': "INFO",  #"DEBUG"  #"WARNING",
    'samplesFile': "/home/jdn/Data/SensorNet/sensornet.log"
}


def run(options):
    with open(options.samplesFile, 'r') as f:
        samples = f.readlines()
    timestamps = []
    values = []
    for sample in samples:
        parts = sample.split(" ")
        if len(parts) != 3:
            continue
        if parts[1].startswith(options.plotter.topicPrefix):
            timestamps.append(parts[0])
            values.append(parts[2].strip().split(','))
    options.plotter.plot(timestamps, values)


#### TODO add optional list of MAC addresses to enable/disable of the selected device type

def getOpts():
    usage = f"Usage: {sys.argv[0]} [-v] [-L <logLevel>] [-l <logFile>] " + \
      "[-s <samplesFile>] sensor"
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
        "-s", "--samplesFile", action="store", type=str,
        default=DEFAULTS['samplesFile'],
        help="Path to location of file with sensor samples")
    ap.add_argument(
        "-v", "--verbose", action="count", default=0,
        help="Enable printing of debug info")
    ap.add_argument(
        "sensor", action="store", type=str, choices=PLOTTERS.keys(),
        help="Type of Sensor")
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

    if not os.path.exists(opts.samplesFile):
        logging.error(f"Samples file '{opts.samplesFile}' doesn't exist")
        sys.exit(1)

    opts.plotter = PLOTTERS[opts.sensor]

    if opts.verbose:
        print(f"    Sensor Type:  {opts.plotter.description}")
        print(f"    Samples File: {opts.samplesFile}")
    return opts


if __name__ == '__main__':
    opts = getOpts()
    r = run(opts)
    sys.exit(r)
