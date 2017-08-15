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

if test ! -f ../../../waf ; then
    echo "please run this program from within the directory `dirname $0`, like this:"
    echo "cd `dirname $0`"
    echo "./`basename $0`"
    exit 1
fi

# return 0 if $1 <= $2 <= $3, 1 otherwise
within_tolerance()
{
  if [ 1 -eq "$(echo "$1 <= $2" | bc)" ]
  then  
    if [ 1 -eq "$(echo "$2 <= $3" | bc)" ]
    then
      echo "0"
      return
    fi
  fi 
  echo "1"
}

#set -x
set -o errexit

outputDir=`pwd`/results-phy-duration
rm -rf "${outputDir}"
mkdir -p "${outputDir}"


passed_tests=()
failed_tests=()

#################################################################
#  Test description
#
# 10 seconds of data transfer at 10 Kbps, so 13 1000 byte packets
# for each network.  Also, ~10 beacons per AP per second, so we
# should see about 26 data frames, 26 acks, and 200 beacons total
# Beacon is 0.140ms
# UDP frame is 0.108ms
# Ack is 0.028ms
# 
# All together, the above is about 31.5ms, so we will test for
# value of wifi channel occupancy of about 28.5ms < occupancy < 34.5ms
# 
#################################################################

label="10Kbps-Wifi"
lower_bound=0.0285
upper_bound=0.0345

d1=10
d2=10
simTag="10Kbps-Wifi"
transport=Udp
udpRate=10Kbps
duration=10

# need this as otherwise waf won't find the executables
cd ../../../

./waf --run laa-wifi-simple --command="%s --cellConfigA=Wifi --cellConfigB=Wifi --d1=${d1} --d2=${d2} --simTag=${simTag} --transport=${transport} --udpRate=${udpRate} --logPhyArrivals=1 --duration=10 --outputDir=${outputDir}"

cd $outputDir
if ! [ -e "laa_wifi_simple_${simTag}_phy_log" ]; then
    echo "file not found"
    failed_tests+=("$label;")
else
    # Report output
    DURATIONS=`python ../extract-phy-time.py laa_wifi_simple_${simTag}_phy_log 3`
    durations=($DURATIONS)
    # durations[1] is wifi duration
    result=`within_tolerance $lower_bound ${durations[1]} $upper_bound`
    if [ 0 -eq $result ]
    then
        passed_tests+=("$label;")
        echo ""
        echo "Test $label passed"
    else
        echo ""
        echo "Failed tolerance"
        echo "Test $label failed"
        failed_tests+=("$label;")
    fi
fi

cd ..

#################################################################
#  Test description
#
# 10 seconds of data transfer at 100 Kbps, so ~120 1000 byte packets
# for each network (because of delayed start after 2 sec).  
# Also, ~10 beacons per AP per second, so we
# should see about 240 data frames, 240 acks, and 200 beacons total
# Beacon is 0.140ms
# UDP frame is 0.108ms
# Ack is 0.028ms
# 
# All together, the above comes out to 60ms.  We observe slightly
# less than this in practice (241 data frames), due to random start 
# times for clients, so we'll test around +/- 10% of the nominal
# value of wifi channel occupancy of about 54ms < occupancy < 66ms
# 
#################################################################

label="100Kbps-Wifi"
lower_bound=0.054
upper_bound=0.066

d1=10
d2=10
simTag="100Kbps-Wifi"
transport=Udp
udpRate=100Kbps
duration=10

# need this as otherwise waf won't find the executables
cd ../../../

./waf --run laa-wifi-simple --command="%s --cellConfigA=Wifi --cellConfigB=Wifi --d1=${d1} --d2=${d2} --simTag=${simTag} --transport=${transport} --udpRate=${udpRate} --logPhyArrivals=1 --duration=10 --outputDir=${outputDir}"

cd $outputDir
if ! [ -e "laa_wifi_simple_${simTag}_phy_log" ]; then
    echo "file not found"
    failed_tests+=("$label;")
else
    # Report output
    DURATIONS=`python ../extract-phy-time.py laa_wifi_simple_${simTag}_phy_log 3`
    durations=($DURATIONS)
    # durations[1] is wifi duration
    result=`within_tolerance $lower_bound ${durations[1]} $upper_bound`
    if [ 0 -eq $result ]
    then
        passed_tests+=("$label;")
        echo ""
        echo "Test $label passed"
    else
        echo ""
        echo "Failed tolerance"
        echo "Test $label failed"
        failed_tests+=("$label;")
    fi
fi

cd ..

##############
# End of tests
##############

rm -rf "${outputDir}"

if [ ${#failed_tests[@]} == 0 ]; then
    echo "All tests passed"
    exit 0
else
    echo "Some test failed: ${failed_tests[@]}"
    exit 1
fi
 

