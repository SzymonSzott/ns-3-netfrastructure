# nETFRAStructure
an Exemplary Text-based Framework for Running and Analyzing Simulations in [ns-3](https://www.nsnam.org/)

## Overview

The goal of this framework is to provide an all-text-based environment for conducting a complete performance analysis in ns-3.

![](C:\Repo\DCF-simulation\ns3-data-analysis\netfrastructure.png)

1. The bash script should be configured to run the simulation script with a loop over all input variables (including `RngRun` for independent results and experiment replication).
2. The ns-3 simulation model should save (append) the output results (e.g., from [FlowMonitor](https://www.nsnam.org/docs/models/html/flow-monitor.html)) into a CSV file.
3. The Python script is used to:
   - perform a statistical analysis of the data (e.g., means and confidence intervals),
   - generate plots.

All the above elements are fairly straightforward, but one of the key elements is the CSV file structured as follows:

| Timestamp | Input  1 | Input 2 | ...  | RngRun | Flow Id | Output 1 | Output 2 | ...  |
| --------- | -------- | ------- | ---- | ------ | ------- | -------- | -------- | ---- |
|           |          |         |      |        |         |          |          |      |

- The timestamp is for reference.

- There should be as many columns of input variables as necessary to uniquely identify your scenario.

- A separate RngRun column should store the run value used.

- Again, there should be as many columns of output values (e.g., throughput, delay) as necessary for the analysis.

- The flow id column allows to identify the outputs and store them with the chosen granularity (per-flow in this case): one line in the CSV file corresponds to one flow (so a single experiment will generate as many lines in the CSV as there are flows).

Using [pandas](https://pandas.pydata.org/), this structure allows to easily calculate aggregate throughput in a given experiment run (as a sum over all flows). Assuming `df` is a dataframe read from the CSV file and the input variable is the number of flows `nFlows` we have:

```python
df2 = df.groupby(['nFlows','RngRun'])['Throughput'].sum().reset_index()
```

Next, we can easily calculate the average (mean) aggregate throughput as a function of the number of flows and plot the results:

```python
df2.groupby(['nWifi'])['Throughput'].mean().plot()
```

## Summary of Features

Advantages:

- Rapid deployment
- Quick to learn and explain (can be used in teaching ns-3)
- Easy to share code & results
- Decouples each step (simulation, analysis) 
- Modular approach -- replace individual blocks with your preferred tools
- Output results in human-readable format (can be quickly analyzed with both text-based editors and spreadsheet software)
- Supports parallel simulation execution on home computers ([GNU Parallel](https://www.gnu.org/software/parallel/)) and server clusters ([Slurm](https://slurm.schedmd.com/documentation.html))
- Good introduction to more advanced simulation execution frameworks (e.g., [SEM](https://simulationexecutionmanager.readthedocs.io/))

Disadvantages:

- Does not automate running of "missing experiments"
- Specifying a parameter space requires modifying the bash script
- Lack of seamless integration with cluster-based resource management tools

Note that using [SEM](https://simulationexecutionmanager.readthedocs.io/) resolves all of the above issues.

## File description

The exemplary files in this repository are:

- `he-wifi-performance.cc` -- an ns-3 simulation model for analyzing the saturation performance of 802.11ax for a varying number of transmitting stations (`nWifi`),
- `run-he-wifi-performance.sh` -- a Bash script which executes 10 runs of the above model for five values of the `nWifi` input parameter,
- `he-wifi-performance.csv` -- an output file containing the results from the simulation model (taken from ns-3.30.1),
- `plot-he-wifi-performance.py` --  a Python script which generates the plot,
- `he-wifi-performance.png` -- the resulting line plot (802.11ax network throughput in a 20 MHz channel):
  ![](C:\Repo\DCF-simulation\ns3-data-analysis\he-wifi-performance.png)

## Acknowledgements

This work was partially supported by the Motorola Solutions Foundation. Thanks to Lucjan Janowski for initial discussions on an early version.

## Use in research

This framework was used to analyze ns-3 simulation results in the following research papers:

- I. Tinnirello, P. Gallo, S. Szott, and K. Kosek-Szott, "[Impact of LTE’s Periodic Interference on Heterogeneous Wi-Fi Transmissions](http://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=8576599&isnumber=5534602)," in IEEE Communications Letters, vol. 23, no. 2, pp. 342--345, Feb. 2019, doi: 10.1109/LCOMM.2018.2886902, [arxiv.org/abs/1812.08541](https://arxiv.org/abs/1812.08541).
