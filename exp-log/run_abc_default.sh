#!/bin/bash

# run abc for each file in list
# bash run_abc.sh <path_abc> <path_benchmark> <timeout> <path_lib>
# eg: bash run_abc_asic.sh ../bin/abc ../benchmark/EPFL 1000m ../asap7.lib
# 
# bash run_abc_test.sh /workspaces/abc-itmap/build/abc /workspaces/abc-itmap/benchmark/EPFL/arithmetic/ 1000m /workspaces/abc-itmap/asap7_clean.lib 
# bash run_abc_test.sh /workspaces/abc-itmap/build/abc /workspaces/abc-itmap/benchmark/EPFL/random_control/ 1000m /workspaces/abc-itmap/asap7_clean.lib

# bash run_abc_default.sh /home/liujunfeng/ABC/abc-itmap/cmake-build-debug/abc_default /home/liujunfeng/benchmarks/random_control/ 1000m /home/liujunfeng/ABC/abc-itmap/asap7_clean.lib

# bash run_abc_default.sh /home/liujunfeng/ABC/abc_itmap/abc-itmap/build/abc /home/liujunfeng/benchmarks/random_control/ 1000m /home/liujunfeng/ABC/abc_itmap/abc-itmap/asap7_clean.lib
# bash run_abc_default.sh /home/liujunfeng/ABC/abc_itmap/abc-itmap/build/abc /home/liujunfeng/benchmarks/arithmetic/ 1000m /home/liujunfeng/ABC/abc_itmap/abc-itmap/asap7_clean.lib
####################################################################
binary=$(echo "$1" | awk -F "/" '{print $NF}')
dataname=$(basename "${2%/}")
timestamp=$(date +%Y%m%d%H%M%S)
libname=$(echo "$(basename "$4")" | cut -d '.' -f 1)

log="${binary}_${dataname}_${libname}_${timestamp}.log"
#touch "$csv"
touch "$log"
#echo "name, command, input, output, lat, gates, edge, area, delay, lev, stime_gates, stime_gates%, stime_cap(ff), stime_cap%, stime_Area, stime_Area%, stime_Delay(ps), stime_Delay%, cut_time, delay_time, total_time" >> $csv

files=$(find "$2" -name "*.aig")

for element in ${files[@]}
do
    echo "process $element"
    command="read_lib $4 ; read_aiger $element; map; topo; print_stats; stime; buffer; print_stats; stime; upsize; dnsize; print_stats; stime;";
    outputs=$(timeout $3 $1 -c "$command";)
    echo $outputs >> $log 
done
 
