#!/bin/bash

control_c()
{
  echo "exiting"
  exit $?
}

trap control_c SIGINT

set -x
set -o errexit

#Number of sim repetitions:
nJobs=$1

for ((i=1; i<=nJobs; i++)) ;
	do

currentDir=`pwd`
outputDir=`pwd`/results_JOB_$i
mkdir -p "${outputDir}"
rm -f "${outputDir}"/laa_wifi_indoor_ed_*_operator?
rm -f "${outputDir}"/time_stats

cd ../

# Random number generator seed
RngRun=$RANDOM
# Which node number in the scenario to enable logging on
logNodeId=6
# Transport protocol (Ftp, Tcp, or Udp).  Udp corresponds to full buffer.
transport=Ftp
# TXOP duration (ms) for LAA
lbtTxop=8
# Base simulation duration (seconds); scaled below
base_duration=$2
# Enable voice instead of FTP on two UEs
voiceEnabled=1
# Enlarge wifi queue size to accommodate FTP UDP file bursts (packets)
wifiQueueMaxSize=2000
# Set to value '0' for LAA SISO, '2' for LAA MIMO
laaTxMode=2
# Cell A Configuration
cellA=$3
# Cell B Configuration
cellB=$4

# Run Wifi and LAA both with energy detection -72.0 (no effect on Wi-Fi)
for ftpLambda in 1.0 ; do
# Energy detection threshold for LAA
    for energyDetection in -72.0 ; do
        for cell_1 in $cellA ; do
	    for cell_2 in $cellB ; do
            	# Make the simulation duration inversly proportional to ftpLambda
            	duration=$(echo "$base_duration/$ftpLambda" | bc)
            	simTag="eD_${energyDetection}_ftpLambda_${ftpLambda}_cellA_${cell_1}_cellB_${cell_2}"
            	/usr/bin/time -f '%e %U %S %K %M %x %C' -o "${outputDir}"/time_stats -a \
            	./waf --run laa-wifi-simple --command="%s --cellConfigA=${cell_1} --cellConfigB=${cell_2} --lbtTxop=${lbtTxop} --logWifiRetries=1 --logWifiFailRetries=1 --logPhyArrivals=1 --logPhyNodeId=${logNodeId} --transport=${transport} --ftpLambda=${ftpLambda} --duration=${duration} --cwUpdateRule=nacks80 --logHarqFeedback=1 --logTxops=1 --logCwChanges=1 --logBackoffChanges=1 --laaEdThreshold=${energyDetection} --simTag=${simTag} --outputDir=${outputDir} --wifiQueueMaxSize=${wifiQueueMaxSize} --voiceEnabled=${voiceEnabled} --ns3::LteEnbRrc::DefaultTransmissionMode=${laaTxMode} --RngRun=${RngRun}"
	    done
        done
    done
done

cd "${currentDir}"
done
