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

import matplotlib.pylab as plt
import numpy as np
import pandas as pd


DEFAULTS = {
    'logLevel': "INFO",  #"DEBUG"  #"WARNING",
    'samplesFile': "/home/jdn/Data/SensorNet/sensornet.csv"
}

RESAMPLE_FREQS = {
    'minutes': "T",
    'hours': "H",
    'days': "D",
    'weeks': "W",
    'months': "M"
}


def radPlotter(df):
    ## consider dropping CPM and just go with uSv/h
    return df.plot(secondary_y='Vcc')


def spsPlotter(df):
    return df.plot(secondary_y='tps')


def pmsPlotter(df):
    return df.plot.line()


def bnnPlotter(df):
    ## consider dropping volts
    return df.plot(secondary_y='grams')


def whPlotter(df):
    return df.plot.line()


SENSORS = {
    'Radiation': {
        'topic': "/sensors/Radiation/",
        'description': "Radiation sensor (SBT-11A)",
        'plotters': {
            '1.0.0': radPlotter
        }
    },
    'AirQualityPMS': {
        'topic': "/sensors/AirQuality/PMS/",
        'description': "Air quality sensor (PMS7003)",
        'plotters': {
            '1.0.0': pmsPlotter
        }
    },
    'AirQualitySPS': {
        'topic': "/sensors/AirQuality/SPS/",
        'description': "Air quality sensor (SPS30)",
        'plotters': {
            '1.0.0': spsPlotter
        }
    },
    'BirdyNumNum': {
        'topic': "/sensors/BirdyNumNum",
        'description': "Hummingbird feeder sensor",
        'plotters': {
            '1.0.2': bnnPlotter
        }
    },
    'WaterHeater': {
        'topic': "/sensors/WaterHeater",
        'description': "Water heater temperature sensor",
        'plotters': {
            '0.0.0': whPlotter
        }
    }
}


#### TODO do filtering/data-prep/stats with Pandas, pass off DFs to plotters
#### TODO create separate DFs for each sensor/device/schema and handle each separately
def run(options):
    def _convDtype(str):
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

    streams = {}
    with open(options.samplesFile, 'r') as f:
        reader = csv.reader(f)
        for row in reader:
            if len(row) < 3:
                logging.warning(f"Bad line in csv file: {row}")
                continue
            parts = row[1].split('/')
            appName = parts[2] if len(parts) == 5 else "".join(parts[2:-2])
            if (appName not in options.sensors) or ((options.devices != None) and (parts[3] not in options.devices)):
                continue
            streamName = "_".join(parts[2:-1])
            if row[1].endswith("/cmd"):
                if streamName not in streams:
                    dataTypes = {'time': "str", 'topic': "str"}
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

    if not streams:
        logging.error("No data streams matching the given sensor/device specs")
        sys.exit(1)
    logging.info(f"Read data streams: {[k + '_' + v['version'] for k, v in streams.items()]}")
    for name in list(streams.keys()):
        streams[name]['file'].flush()
        streams[name]['file'].seek(0)
        if (streams[name]['appName'] not in options.sensors) or (options.devices and streams[name]['device'] not in options.devices):
            streams[name]['file'].close()
            del streams[name]

    axs = []
    for name in streams.keys():
        stream = streams[name]
        header = stream['header']
        df = pd.read_csv(stream['file'],
                         sep=',',
                         names=header,
                         dtype=stream['dataTypes'],
                         index_col=0,
                         parse_dates=[0])
        if options.verbose:
            buf = StringIO()
            df.info(options.verbose > 1, buf=buf, max_cols=len(header), show_counts=True)
            print(buf.getvalue())
        if False:  #### TMP TMP TMP
            df.to_csv(f"/tmp/{name}.csv", index=False)
        if len(df.index) < 1:
            logging.warning(f"No data for stream '{name}', skipping it")
            continue

        firstDatetime = df.first_valid_index().to_pydatetime()
        lastDatetime = df.last_valid_index().to_pydatetime()
        startDatetime = firstDatetime
        if options.startDate:
            if options.startDate < firstDatetime or options.startDate > lastDatetime:
                logging.error(f"Invalid start date: {options.startDate} not between {firstDatetime} and {lastDatetime}, skipping {name}")
                continue
            startDatetime = options.startDate
        endDatetime = lastDatetime
        if options.endDate:
            if options.endDate < firstDatetime or options.endDate > lastDatetime:
                logging.error(f"Invalid end date: {options.endDate} not between {firstDatetime} and {lastDatetime}")
                sys.exit(1)
            if options.endDate < options.startDate:
                logging.error(f"Invalid end date: {options.endDate} not after {options.startDate}")
                sys.exit(1)
            endDatetime = options.endDate
        duration = endDatetime - startDatetime
        samples = df[startDatetime:endDatetime]
        if options.resample:
#            samples = samples.rolling(options.avgSamples).mean()
            samples = samples.resample(RESAMPLE_FREQS[options.resample]).mean()
        numSamples = len(samples)
        samplesPerMin = (numSamples / duration.total_seconds()) * 60.0

        if options.verbose:
            print(f"    Sensor:  {name}")
            print(f"        Start date:        {startDatetime}")
            print(f"        End date:          {endDatetime}")
            print(f"        Duration:          {duration}")
            print(f"        Number of samples: {numSamples}")
            print(f"        Samples/min:       {samplesPerMin:.6f}")

        #### TODO slice df based on start/end times
        axs.append(SENSORS[stream['appName']]['plotters'][stream['version']](samples))
    plt.show()


def validDate(dateStr):
    try:
        return datetime.fromisoformat(dateStr)
    except ValueError:
        raise argparse.ArgumentTypeError(f"Invalid date: {dateStr}")


def getOpts():
    usage = f"Usage: {sys.argv[0]} [-v] [-L <logLevel>] [-l <logFile>] " + \
      "[-s <samplesFile>] [-r <resample>] [-S <isodate>] [-E <isodate>] [-d <device>]* {<sensor>}+"
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "-r", "--resample", action="store", choices=RESAMPLE_FREQS.keys(),
        default="minutes", help="Resample to the given frequency")
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
        "sensors", nargs="+", type=str, choices=SENSORS.keys(),
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
        print(f"    Sensor Type(s): {[s + ': ' + SENSORS[s]['description'] for s in opts.sensors]}")
        if opts.devices:
            print(f"    Devices:        {opts.devices}")
        print(f"    Samples File:   {opts.samplesFile}")
        print(f"    Resample Freq.: {opts.resample}")
    return opts


if __name__ == '__main__':
    opts = getOpts()
    r = run(opts)
    sys.exit(r)
