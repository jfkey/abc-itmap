#!/bin/bash

# run abc for each file in list
# bash run_abc.sh <path_abc> <path_benchmark> <timeout> <path_lib>
# eg: bash run_abc_asic.sh ../bin/abc ../benchmark/EPFL 1000m ../asap7.lib
# bash run_abc_asic.sh /home/liujunfeng/ABC/abc-itmap/cmake-build-debug/abc /home/liujunfeng/benchmarks/arithmetic/ 1000m /home/liujunfeng/ABC/abc-itmap/asap7_clean.lib
# bash run_abc_asic.sh /home/liujunfeng/ABC/abc-itmap/cmake-build-debug/abc /home/liujunfeng/benchmarks/random_control/ 1000m /home/liujunfeng/ABC/abc-itmap/asap7_clean.lib

# bash run_abc_test.sh /workspaces/abc-itmap/build/abc /workspaces/abc-itmap/benchmark/EPFL/arithmetic/ 1000m /workspaces/abc-itmap/asap7_clean.lib;bash run_abc_test.sh /workspaces/abc-itmap/build/abc /workspaces/abc-itmap/benchmark/EPFL/random_control/ 1000m /workspaces/abc-itmap/asap7_clean.lib


####################################################################
binary=$(echo "$1" | awk -F "/" '{print $NF}')
timestamp=$(date +%Y%m%d%H%M%S)
#csv="${timestamp}_$binary.map_r.csv"
log="${timestamp}_$binary.log"
#touch "$csv"
touch "$log"
#echo "name, command, input, output, lat, gates, edge, area, delay, lev, stime_gates, stime_gates%, stime_cap(ff), stime_cap%, stime_Area, stime_Area%, stime_Delay(ps), stime_Delay%, cut_time, delay_time, total_time" >> $csv

files=$(find "$2" -name "*.aig")

for element in ${files[@]}
do
    echo "process $element"
    command="read_lib $4 ;read_aiger $element; map;  topo;  stime; print_stats;";
    outputs=$(timeout $3 $1 -c "$command";)
    echo $outputs >> $log 
done
 