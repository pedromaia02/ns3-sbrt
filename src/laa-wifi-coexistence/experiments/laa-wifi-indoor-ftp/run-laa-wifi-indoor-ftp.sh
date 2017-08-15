#!/bin/bash

#
# Copyright (c) 2015 University of Washington
# Copyright (c) 2015 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation;
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Authors: Tom Henderson <tomh@tomh.org> and Nicola Baldo <nbaldo@cttc.es>
#

control_c()
{
  echo "exiting"
  exit $?
}

trap control_c SIGINT

if test ! -f ../../../../waf ; then
    echo "please run this program from within the directory `dirname $0`, like this:"
    echo "cd `dirname $0`"
    echo "./`basename $0`"
    exit 1
fi


set -x
set -o errexit

outputDir=`pwd`/results
mkdir -p "${outputDir}"
rm -f "${outputDir}"/laa_wifi_indoor_ed_*_operator?
rm -f "${outputDir}"/time_stats

# need this as otherwise waf won't find the executables
cd ../../../../

# Random number generator seed
RngRun=1
# Which node number in the scenario to enable logging on
logNodeId=0
# Transport protocol (Ftp, Tcp, or Udp).  Udp corresponds to full buffer.
transport=Ftp
# TXOP duration (ms) for LAA
lbtTxop=8
# Base simulation duration (seconds); scaled below
base_duration=10
# Enable voice instead of FTP on two UEs
voiceEnabled=0
# Enlarge wifi queue size to accommodate FTP UDP file bursts (packets)
wifiQueueMaxSize=2000
# Set to value '0' for LAA SISO, '2' for LAA MIMO
laaTxMode=2
# Distance between the operators
distance=10

# Run Wifi and LAA both with energy detection -72.0 (no effect on Wi-Fi)
for ftpLambda in 2.5 ; do
# Energy detection threshold for LAA
    for energyDetection in -62.0 -72.0 -82.0; do
        for cell in Laa Wifi; do
            for distance in 10; do
                # Make the simulation duration inversly proportional to ftpLambda
                duration=$(echo "$base_duration/$ftpLambda" | bc)
                simTag="eD_${energyDetection}_ftpLambda_${ftpLambda}_cellA_${cell}_d2_${distance}_m"
                /usr/bin/time -f '%e %U %S %K %M %x %C' -o "${outputDir}"/time_stats -a \
                ./waf --run laa-wifi-simple --command="%s --cellConfigA=Laa --cellConfigB=${cell} --lbtTxop=${lbtTxop} --logWifiRetries=1 --logWifiFailRetries=1 --logPhyArrivals=1 --logPhyNodeId=${logNodeId} --transport=${transport} --ftpLambda=${ftpLambda} --duration=${duration} --cwUpdateRule=nacks80 --logHarqFeedback=1 --logTxops=1 --logCwChanges=1 --logBackoffChanges=1 --laaEdThreshold=${energyDetection} --simTag=${simTag} --outputDir=${outputDir} --wifiQueueMaxSize=${wifiQueueMaxSize} --voiceEnabled=${voiceEnabled} --ns3::LteEnbRrc::DefaultTransmissionMode=${laaTxMode} --RngRun=${RngRun} --lteDutyCycle=1.0 --d2=${distance} --ChannelAccessManager=Lbt"
            done
        done
    done
done
