
 set encoding utf8 
 
 set style line 1 pointtype 4 linetype 1 
 set style line 2 pointtype 6 linetype 2 
 set style line 3 pointtype 8 linetype 3 
 set style line 4 pointtype 5 linetype 4
 set style line 5 pointtype 7 linetype 5
 set style line 6 pointtype 9 linetype 6

 set style increment 
 
 set pointsize 2 
 set grid
 
 set key top left Left reverse
 set term postscript eps enhanced   color   
 set output "images/ps/simple_ftpLambda_2_5_Wifi_LAA__throughput.eps" 
   set xlabel "Vazão [Mbps]"
 set ylabel "CDF"
  
 unset title
 
 unset title
 plot [0:150][0:1] "results/cdf_throughput_eD_-62.0_ftpLambda_2.5_cellA_Laa_d2_10_m_A" using ($1):($2)  with linespoints ls 1  title "Operadora A - LTE-LBT - limiar LBT-ED -62dBm"  ,\
    "results/cdf_throughput_eD_-72.0_ftpLambda_2.5_cellA_Wifi_d2_10_m_A" using ($1):($2)  with linespoints ls 2  title "Operadora A - LTE-LBT - limiar LBT-ED -72dBm"  ,\
    "results/cdf_throughput_eD_-82.0_ftpLambda_2.5_cellA_Wifi_d2_10_m_A" using ($1):($2)  with linespoints ls 3  title "Operadora A - LTE-LBT - limiar LBT-ED -82dBm"  ,\
   "results/cdf_throughput_eD_-62.0_ftpLambda_2.5_cellA_Wifi_d2_10_m_B" using ($1):($2)  with linespoints ls 4  title "Operadora B - Wi-Fi - limiar LBT-ED -62dBm"  ,\
   "results/cdf_throughput_eD_-72.0_ftpLambda_2.5_cellA_Wifi_d2_10_m_B" using ($1):($2)  with linespoints ls 5  title "Operadora B - Wi-Fi - limiar LBT-ED -72dBm"  ,\
   "results/cdf_throughput_eD_-82.0_ftpLambda_2.5_cellA_Wifi_d2_10_m_B" using ($1):($2)  with linespoints ls 6  title "Operadora B - Wi-Fi- limiar LBT-ED -82dBm"
