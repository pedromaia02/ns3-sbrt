#!/bin/bash

rm laa_wifi*

for i in {0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1}
    do
       #echo "Welcome $i times"
	./waf --run "laa-wifi-simple" --command='%s --lteDutyCycle='"$i"''
 done
    #./waf --run "laa-wifi-simple" --command='%s --lteDutyCycle=1'

#./waf --run "laa-wifi-simple" --commands='%s '
