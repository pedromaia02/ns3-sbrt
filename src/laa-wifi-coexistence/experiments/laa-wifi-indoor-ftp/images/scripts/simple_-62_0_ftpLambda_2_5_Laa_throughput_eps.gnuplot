 
 set style line 1 pt 4 lt 1 
 set style line 2 pt 7 lt 2 
 set style increment 
 
 set pointsize 2 
 set grid
 
 set key bottom right 
 set term postscript eps enhanced   color   
 set output "images/ps/simple_-62_0_ftpLambda_2_5_Laa_throughput.eps" 
   set xlabel "Vazão [Mbps]"
 set ylabel "CDF"
  
 unset title
 plot [0:150][0:1] "results/cdf_throughput_eD_-62.0_ftpLambda_2.5_cellA_Laa_d2_10_m_A" using ($1):($2)  with linespoints ls 1  title "operador A (LBT)"  , "results/cdf_throughput_eD_-62.0_ftpLambda_2.5_cellA_Laa_d2_10_m_B" using ($1):($2)  with linespoints ls 2  title "operador B (Wi-Fi)"  
