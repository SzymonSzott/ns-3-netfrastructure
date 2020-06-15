#!/usr/bin/python3
# -*- coding: utf-8 -*-

"""nETFRAStructure example Python plotting script

   - Description: Plots results of the `he-wifi-performance` ns-3 scenario
   - Author: Szymon Szott <szott@kt.agh.edu.pl>
   - Website: https://github.com/SzymonSzott/ns-3-netfrastructure
   - Date: 2020-06-13
"""
import pandas as pd
import matplotlib.pyplot as plt

# Read data from CSV file
df = pd.read_csv('he-wifi-performance.csv', delimiter=',')

# Group by sum of all flows in a given experiment run (to obtain aggregate throughput)
df = df.groupby(['nWifi','RngRun'])['Throughput'].sum().reset_index()

# Group by nWifi and calculate average (mean) aggregate throughput
df = df.groupby(['nWifi'])['Throughput'].mean()

# Plot
ax = df.plot(title='IEEE 802.11ax Performance',  marker='o', legend=False, ylim=(0,140))
ax.set(xlabel="Number of transmitting Wi-Fi stations", ylabel="Network throughput [Mb/s]")

# Save to file
plt.tight_layout()
plt.savefig('he-wifi-performance.png');
