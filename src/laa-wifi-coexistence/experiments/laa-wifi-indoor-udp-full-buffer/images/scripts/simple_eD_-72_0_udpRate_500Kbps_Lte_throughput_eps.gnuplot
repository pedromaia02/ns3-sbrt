 
 set style line 1 pt 4 lt 1 
 set style line 2 pt 7 lt 2 
 set style increment 
 
 set pointsize 2 
 set grid
 
 set key bottom right 
 set term postscript eps enhanced   color   
 set output "images/ps/simple_eD_-72_0_udpRate_500Kbps_Lte_throughput.eps" 
    set xlabel "Throughput [Mbps]"
 set ylabel "CDF"
 set title "EdThresh=-72.0, udpRate=500Kbps, cellA=Lte, UDP" 
  
 unset title
 
 unset title
 
 unset title
 plot [0:0.5][0:1] "results/cdf_throughput_eD_-72.0_udpRate_500Kbps_cellA_Lte_A" using ($1):($2)  with linespoints ls 1  title "operator A (LTE-DC)"  , "results/cdf_throughput_eD_-72.0_udpRate_500Kbps_cellA_Lte_B" using ($1):($2)  with linespoints ls 2  title "operator B (Wi-Fi)"  
