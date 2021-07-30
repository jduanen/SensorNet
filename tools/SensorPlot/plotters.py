'''
SensorPlot SensorNet Data Plotting Package
'''

import matplotlib.pyplot as plt
from abc import ABC, abstractmethod


NUM_TICKS = 20


'''
Base Class for SensorPlot Plotters
'''
class Plotter(ABC):
    #### TODO think about using Pandas
    def __init__(self, topicPrefix, description):
        self.topicPrefix = topicPrefix
        self.description = description

    @abstractmethod
    def plot(self, timestamps, sources, values):
        pass

'''
SensorPlot Plotter for Radition Monitor
'''
class RadPlotter(Plotter):
    def __init__(self):
        super().__init__("/sensors/Radiation/", "Radiation sensor (SBT-11A)")

    def plot(self, timestamps, sources, values, window=60):
        indices = [i for i, x in enumerate(values) if x == ['CPM', 'uSv/h', 'Vcc']]
        for index in sorted(indices, reverse=True):
            del timestamps[index]
            del values[index]
        cpm, uSv, volts = zip(*values)

        cpm = list(map(int, cpm))
        uSv = list(map(float, uSv))
        volts = list(map(float, cpm))
        fig, ax = plt.subplots()
        plt.xlabel("Sample Time")
        plt.ylabel("Counts per Minute")
        ax.plot(timestamps, cpm, color='b')
        ax.tick_params(axis='y', labelcolor='black', labelright=True)

        if window:
            assert window <= len(cpm), "Window too large for data size"
            avg = [sum(cpm[i:i+window])/window for i in range(len(cpm) - window)]
            ax.plot(avg, color='r')

        ticks = round(len(values) / (NUM_TICKS * 60)) * 60
        plt.xticks(timestamps[::ticks], rotation='vertical')
        plt.gcf().autofmt_xdate()
        plt.show()


'''
SensorPlot Plotter for Air Quality Monitor with PMS Sensor
'''
class PmsPlotter(Plotter):
    def __init__(self):
        super().__init__("/sensors/AirQuality/PMS/", "Air quality sensor (PMS7003)")

    def plot(self, timestamps, sources, values):
        pms1, pms2_5, pms10 = zip(*values)

        pms1 = list(map(int, pms1))
        pms2_5 = list(map(int, pms2_5))
        pms10 = list(map(int, pms10))

        plt.xlabel("Sample Time")
        plt.ylabel("Particle Count")
        plt.plot(timestamps, pms1, color='b', label="pms1", linewidth=1.0)
        plt.plot(timestamps, pms2_5, color='g', label="pms2.5", linewidth=1.0)
        plt.plot(timestamps, pms10, color='r', label="pms10", linewidth=1.0)
        plt.tick_params(axis='y', labelcolor='black')
        ticks = round(len(values) / (NUM_TICKS * 60)) * 60
        plt.xticks(timestamps[::ticks], rotation='vertical')
        plt.gcf().autofmt_xdate()
        plt.legend()
        plt.show()


'''
SensorPlot Plotter for Air Quality Monitor with SPS Sensor
'''
class SpsPlotter(Plotter):
    def __init__(self):
        super().__init__("/sensors/AirQuality/SPS/", "Air quality sensor (SPS30)")

    def plot(self, timestamps, sources, values, TPS=True):
        pm1, pm2_5, pm4, pm10, nc0_5, nc1, nc2_5, nc4, nc10, tps = zip(*values)

        pm1 = list(map(float, pm1))
        pm2_5 = list(map(float, pm2_5))
        pm4 = list(map(float, pm4))
        pm10 = list(map(float, pm10))
        nc0_5 = list(map(float, nc0_5))
        nc1 = list(map(float, nc1))
        nc2_5 = list(map(float, nc2_5))
        nc4 = list(map(float, nc4))
        nc10 = list(map(float, nc10))
        tps = list(map(float, tps))

        fig, ax = plt.subplots()
        ax.plot(timestamps, pm2_5, color='yellow', label="pm2.5", linewidth=1.0)
        ax.plot(timestamps, pm10, color='orange', label="pm10", linewidth=1.0)
        ax.plot(timestamps, nc0_5, color='cyan', label="nc0.5", linewidth=1.0)
        ax.plot(timestamps, nc2_5, color='blue', label="nc2.5", linewidth=1.0)
        ax.plot(timestamps, nc10, color='purple', label="nc10", linewidth=1.0)
        ax.tick_params(axis='y', labelcolor='black')
        ax.set_xlabel("Sample Time")
        ax.set_ylabel("Particulate Concentration")
        ax.legend(loc="upper left")

        if TPS:
            ax2 = ax.twinx()
            ax2.plot(timestamps, tps, color='r', label="avg particle size", linewidth=1.0)
            ax2.tick_params(axis='y', labelcolor='r')
            ax2.set_ylabel("Typical Particle Size")
            ax2.legend(loc="upper right")

        ticks = round(len(values) / (NUM_TICKS * 60)) * 60
        plt.xlabel("Sample Time")
        plt.xticks(timestamps[::ticks], rotation='vertical')
        plt.gcf().autofmt_xdate()
        plt.show()


PLOTTERS = {
  "rad": RadPlotter(),
  "pms": PmsPlotter(),
  "sps": SpsPlotter()
}
