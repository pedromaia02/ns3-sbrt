#!/bin/bash

source ../utils/shot
source ../utils/common

IMGDIR=images
THUMBNAILS=true
NOX11=true


BASE_OPTIONS=""
BASE_OPTIONS="${BASE_OPTIONS} ; set style line 1 pt 4 lt 1"
BASE_OPTIONS="${BASE_OPTIONS} ; set style line 2 pt 7 lt 2"
BASE_OPTIONS="${BASE_OPTIONS} ; set style increment ;"

BASE_OPTIONS="${BASE_OPTIONS} ; set pointsize 2 ; set grid;"

set -o errexit

# below value is used for thumbnail page
lbtTxop=8

# Adjust the scale of the plots here
LATENCY_CDF_RANGE="[0:500][0:1]"
THROUGHPUT_CDF_RANGE="[0:150][0:1]"



##############################################################
#  Throughput CDF
##############################################################


for ftpLambda in 2.5 ; do
    for energyDetection in -62.0 -72.0 -82.0; do
        for cell in Laa Wifi ; do
            for distance in 10 ; do
            simTag="eD_${energyDetection}_ftpLambda_${ftpLambda}_cellA_${cell}_d2_${distance}_m"
            # Some variants may not be present; skip those cases
            FILE_TO_TEST=results/laa_wifi_simple_${simTag}_operatorA
            if ! [ -f $FILE_TO_TEST ] ; then
                continue
            fi 
            imgTag="simple_"`echo "$energyDetection_${energyDetection}_ftpLambda_${ftpLambda}_${cell}" | tr '.' '_'`
            TRAFFIC=`print_traffic_model Udp`
           # TITLE="EdThresh=${energyDetection}, ftpLambda=${ftpLambda}, cellA=${cell}, ${TRAFFIC}"
            for OPERATOR in A B ; do
                THROUGHPUT_COLUMN=8
                CURRENT=results/laa_wifi_simple_${simTag}_operator${OPERATOR}
                `../utils/cdf.sh $THROUGHPUT_COLUMN $CURRENT > results/cdf_throughput_${simTag}_${OPERATOR}`
            done
            index=0        
            for OPERATOR in A B ; do
                FILES[$index]=results/cdf_throughput_${simTag}_${OPERATOR}
                YCOLS[$index]='($2)'    
                XCOLS[$index]='($1)'
                LABELS[$index]=`print_operador_${cell,,}_wifi $OPERATOR`
        
                index=`expr $index + 1`
            done

            PLOTTYPE="with linespoints"
            XLABEL="Vazão [Mbps]"
            YLABEL="CDF"
            RANGE=$THROUGHPUT_CDF_RANGE
            OPTIONS="$BASE_OPTIONS ; set key bottom right"
            IMGFILENAME="${imgTag}_throughput"
            plot

            unset FILES
            unset LABELS
            unset YCOLS
            unset XCOLS
            done
        done
    done
done


../utils/shot_thumbnails.sh $IMGDIR/thumbnails "laa-wifi-simple-ftp Laa txop=${lbtTxop} results"

