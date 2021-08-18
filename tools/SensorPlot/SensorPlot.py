#!/usr/bin/env python3
'''
SensorPlot: Gather and plot data from SensorNet log files

Example data:
  - 2021-08-12T11:35:09.346467,/sensors/AirQuality/SPS/c8:2b:96:2a:7a:77/cmd,Startup,ESP8266,AirQualitySPS,1.0.0,pm1_0:.2f,pm2_5:.2f,pm4_0:.2f,pm10_0:.2f,nc0_5:.2f,nc1_0:.2f,nc2_5:.2f,nc4_0:.2f,nc10_0:.2f,tps:.2f
  - 2021-08-12T11:35:09.353259,/sensors/AirQuality/PMS/c8:2b:96:29:f8:9e/cmd,Startup,ESP8266,AirQualityPMS,1.0.0,pm1_0:d,pm2_5:d,pm10_0:d
  - 2021-07-27T20:45:38.401164,/sensors/AirQuality/PMS/c8:2b:96:29:f8:9e/data,1,2,2
  - 2021-07-27T20:17:38.470971,/sensors/AirQuality/SPS/c8:2b:96:2a:7a:77/data,2.16,2.23,2.23,2.26,15.36,17.63,17.68,17.68,17.68,0.53

'''

import argparse
import csv
from datetime import datetime
from io import StringIO
import json
import logging
import os
import sys

import pandas as pd

from plotters import PLOTTERS


DEFAULTS = {
    'logLevel': "INFO",  #"DEBUG"  #"WARNING",
    'samplesFile': "/home/jdn/Data/SensorNet/sensornet.csv"
}


#### TODO do filtering/data-prep/stats with Pandas, pass off DFs to plotters
#### TODO create separate DFs for each sensor/device/schema and handle each separately
def run(options):
    def _convDtype(str):
        print(str)
        if str.endswith('d'):
            typ = "int"
        elif str.endswith('f'):
            typ = "float"
        elif str.endswith('s'):
            typ = "str"
        else:
            logging.warning(f"Unknown data type: {str}")
            typ = "str"
        return typ

    #### TODO read the *.csv file and then read all the archived *gzip files in sequence and concatenate them
    streams = {}
    with open(options.samplesFile, 'r') as f:
        reader = csv.reader(f)
        for row in reader:
            if len(row) < 3:
                logging.warning(f"Bad line in csv file: {row}")
                continue
            streamName = "_".join(row[1].split('/')[2:-1])
            if row[1].endswith("/cmd"):
                if streamName not in streams:
                    dataTypes = {'time': "str", 'topic': "str"}
                    print("ZZZ", [h for h in row[6:]])
                    dataTypes.update({h.split(':')[0]: _convDtype(h.split(':')[1]) for h in row[6:]})
                    streams[streamName] = {
                        'sensor': row[1].split('/')[2],
                        'device': row[1].split('/')[-2],
                        'appName': row[4],
                        'version': row[5],
                        'header': list(dataTypes.keys()),
                        'dataTypes': dataTypes,
                        'file': StringIO()
                    }
                continue
            if streamName not in streams:
                continue
            streams[streamName]['file'].write(",".join(row) + "\n")

    logging.info(f"Read data streams: {[k + '_' + v['version'] for k, v in streams.items()]}")
    sensors = [PLOTTERS[s]['appName'] for s in options.sensors]
    for name in list(streams.keys()):
        streams[name]['file'].flush()
        streams[name]['file'].seek(0)
        if (streams[name]['appName'] not in sensors) or (options.devices and streams[name]['device'] not in options.devices):
            streams[name]['file'].close()
            del streams[name]

    if not streams:
        logging.error("No data streams matching the given sensor/device specs")
        sys.exit(1)
    logging.info(f"Remaining data streams: {[k + '_' + v['version'] for k, v in streams.items()]}")

    for name in streams.keys():
        header = streams[name]['header']
        print("XXXXX", header)
        print("YYYYY", streams[name]['dataTypes'])
        df = pd.read_csv(streams[name]['file'],
                         sep=',',
                         names=header,
                         dtype=streams[name]['dataTypes'],
                         index_col=0,
                         parse_dates=[0])
        if options.verbose:
            df.info(True, sys.stdout, max_cols=len(header), show_counts=True)
        df.to_csv(f"/tmp/{streamName}.csv", index=False)

    sys.exit(1)

    if len(timestamps) < 1:
        logging.error("No samples")
        sys.exit(1)
    firstDate = datetime.fromisoformat(timestamps[0])
    lastDate = datetime.fromisoformat(timestamps[-1])
    print(firstDate, lastDate)
    print(options.startDate)
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
    options.duration = options.endDate - options.startDate

    avgCounts = {k: (v / options.duration.seconds) * 60 for k, v in options.sampleCounts.items()}

    if options.verbose:
        print(f"    Start date:    {options.startDate}")
        print(f"    End date:      {options.endDate}")
        print(f"    Duration:      {options.duration}")
        print(f"    Sample Counts: {options.sampleCounts}")
        print(f"    Samples/min:   {avgCounts}")

    startIndx = list(map(lambda d: datetime.fromisoformat(d) > options.startDate, timestamps)).index(True)
    indx = list(map(lambda d: datetime.fromisoformat(d) >= options.endDate, timestamps)).index(True)
    endIndx = indx if indx > 0 else -1
    options.plotter.plot(timestamps[startIndx:endIndx],
                         sources[startIndx:endIndx],
                         values[startIndx:endIndx])


def validDate(dateStr):
    try:
        return datetime.fromisoformat(dateStr)
    except ValueError:
        raise argparse.ArgumentTypeError(f"Invalid date: {dateStr}")

#### TODO take list of one or more sensor types
#### TODO add optional list of MAC addresses to enable/disable of the selected device type

def getOpts():
    usage = f"Usage: {sys.argv[0]} [-v] [-L <logLevel>] [-l <logFile>] " + \
      "[-s <samplesFile>] [-S <isodate>] [-E <isodate>] [-d <device>]* {<sensor>}+"
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "-d", "--devices", action="append", type=str,
        help="MAC address of device to explore")
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
        #### FIXME allow a list
        "sensors", nargs="+", type=str, choices=PLOTTERS.keys(),
        help="Type of sensor to explore")
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

    if opts.devices:
        for dev in opts.devices:
            if len(dev.split(":")) != 6:
                logging.error(f"Invalid device MAC address: {dev}")
                sys.exit(1)

    if opts.verbose:
        print(f"    Sensor Type(s): {[s + ': ' + PLOTTERS[s]['description'] for s in opts.sensors]}")
        if opts.devices:
            print(f"    Devices:        {opts.devices}")
        print(f"    Samples File:   {opts.samplesFile}")
    return opts


if __name__ == '__main__':
    opts = getOpts()
    r = run(opts)
    sys.exit(r)
