import numpy as np
import pandas as pd
import scipy.stats as st

#Read data from CSV file
data = pd.read_csv('adhoc-multihop.csv', delimiter=',')

#Drop irrelevant columns 
data.drop('Timestamp', axis=1, inplace=True)
data.drop('Delay', axis=1, inplace=True)

#Group by flow means
f = data.groupby(['OfferedLoad', 'Flow']).mean()
#Convert to plottable format
plot_df = f.unstack('Flow').loc[:, 'Throughput']

#Create a plot from the dataframe
ax = plot_df.plot(title='All flows')
#Configure plot elements
ax.set(xlabel="Per-flow offered load [Mb/s]", ylabel="Throughput [Mb/s]")

#Group by sum of all flows in a given experiment run (to obtain aggregate throughput)
plot_sum = data.groupby(['OfferedLoad','RngRun'])['Throughput'].sum()
plot_sum=plot_sum.reset_index()  
plot_sum.drop('RngRun', axis=1, inplace=True)
#Set/generate parameters for calculating confidence intervals 
alpha=0.05
std = plot_sum.groupby('OfferedLoad').std().loc[:, 'Throughput']
n = plot_sum.groupby('OfferedLoad').count().loc[:, 'Throughput']
#Calculate confidence intervals 
yerr = std / np.sqrt(n) * st.t.ppf(1-alpha/2, n - 1)
#Group by offered load and calculate average (mean) aggregate throughput
plot_sum=plot_sum.groupby(['OfferedLoad']).mean()
#Plot with confidence intervals
ax = plot_sum.plot(title='Aggregate network throughput', yerr=yerr)
ax.set(xlabel="Per-flow offered load [Mb/s]", ylabel="Throughput [Mb/s]")