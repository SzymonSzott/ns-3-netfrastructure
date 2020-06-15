#!/bin/bash

# nETFRAStructure example Bash script
# -----------------------------------
#
# - Description: Executes multiple runs of `he-wifi-performance`
# - Author: Szymon Szott <szott@kt.agh.edu.pl>
# - Website: https://github.com/SzymonSzott/ns-3-netfrastructure
# - Date: 2020-06-10

# Configure simulation parameter space
reps=`seq 10` #Number of repetitions (independent simulations)
nWifi=`seq 2 2 10` #Number of Wi-Fi nodes: 2, 4, 6, 8, 10

# Run ns-3 simulations over all parameters in nested `for` loops
for r in $reps 
do
	for n in $nWifi 
	do
		./waf --run "he-wifi-performance --nWifi=$n --RngRun=$r"
	done
done
