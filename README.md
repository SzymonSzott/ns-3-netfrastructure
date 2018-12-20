# ns3-data-analysis
An exemplary data analysis framework for working with ns-3

## Steps to reproduce

The following is a list of steps to perform a generic data analysis (once you have a working simulation scenario file):

1. Configure bash to run your simulation script in an inner loop over several RngRun values and in an outer loop over some other variable (e.g., the traffic generator's offered load).
2. Configure your simulation script to save the results (e.g., from FlowMonitor) into a CSV file.
3. Run the bash script.
4. Use a Python script to perform 
    - a statistical analysis of the data (means and confidence intervals),
    - generate plots.
  
## File description

The exemplary files in this repository are:

- `adhoc-multihop.csv` - contains results from a simulation of an adhoc network (the contents are not relevant, but notice the column structure)
- `ns3-data-analysis.py` - contains the Python code to perform the statistical analysis and generate two plots.
