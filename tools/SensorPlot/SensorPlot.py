#!/usr/bin/env python3
'''
SensorPlot: Gather and plot data from SensorNet log files
'''

import argparse
from datetime import datetime
import json
import logging
import os
import pytz
import sys

from plotters import PLOTTERS


DEFAULTS = {
    'logLevel': "INFO",  #"DEBUG"  #"WARNING",
    'samplesFile': "/home/jdn/Data/SensorNet/sensornet.log"
}


utc = pytz.UTC


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
            timestamps.append(parts[0][:-2] + ":" + parts[0][-2:])
            values.append(parts[2].strip().split(','))

    firstDate = datetime.fromisoformat(timestamps[0])
    lastDate = datetime.fromisoformat(timestamps[-1])
    print(firstDate, lastDate)
    print(options.startDate, options.endDate)
    if options.startDate:
        if options.startDate < firstDate or options.startDate > lastDate:
            logging.error(f"Invalid start date: {options.startDate} not between {firstDate} and {lastDate}")
            sys.exit(1)
    else:
        options.startDate = firstDate
    if options.endDate:
        if options.endDate < firstDate or options.endDate > lastDate:
            logging.error(f"Invalid end date: {options.endDate} not between {firstDate} and {lastDate}")
            sys.exit(1)
        if options.endDate < options.startDate:
            logging.error(f"Invalid end date: {options.endDate} not after {options.startDate}")
            sys.exit(1)
    else:
        options.endDate = lastDate
    if options.verbose:
        print(f"    Start date:   {options.startDate}")
        print(f"    End date:     {options.endDate}")

    options.plotter.plot(timestamps, values)


def validDate(dateStr):
    try:
        return utc.localize(datetime.fromisoformat(dateStr))
    except ValueError:
        raise argparse.ArgumentTypeError(f"Invalid date: {dateStr}")

#### TODO add optional list of MAC addresses to enable/disable of the selected device type

def getOpts():
    usage = f"Usage: {sys.argv[0]} [-v] [-L <logLevel>] [-l <logFile>] " + \
      "[-s <samplesFile>] [-S <isodate>] [-E <isodate>] sensor"
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "-E", "--endDate", action="store", type=validDate,
        help="End date (in ISO8016 format) -- defaults to last date in data log")
    ap.add_argument(
        "-L", "--logLevel", action="store", type=str,
        default=DEFAULTS['logLevel'],
        choices=["DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"],
        help="Logging level")
    ap.add_argument(
        "-l", "--logFile", action="store", type=str,
        help="Path to location of logfile (create it if it doesn't exist)")
    ap.add_argument(
        "-S", "--startDate", action="store", type=validDate,
        help="Start date (in ISO8016 format) -- defaults to first date in data log")
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
