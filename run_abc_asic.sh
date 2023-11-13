#!/bin/bash

# run abc for each file in list
# bash run_abc.sh <path_abc> <path_benchmark> <timeout> <path_lib>
# eg: bash run_abc_asic.sh ../bin/abc ../benchmark/EPFL 1000m ../asap7.lib
# bash run_abc_asic.sh /home/liujunfeng/ABC/abc-itmap/cmake-build-debug/abc /home/liujunfeng/benchmarks/arithmetic/ 1000m /home/liujunfeng/ABC/abc-itmap/asap7_clean.lib
# bash run_abc_asic.sh /home/liujunfeng/ABC/abc-itmap/cmake-build-debug/abc /home/liujunfeng/benchmarks/random_control/ 1000m /home/liujunfeng/ABC/abc-itmap/asap7_clean.lib

####################################################################
binary=$(echo "$1" | awk -F "/" '{print $NF}')
timestamp=$(date +%Y%m%d%H%M%S)
csv="${timestamp}_$binary.map_r.csv"
log="${timestamp}_$binary.log"
#touch "$csv"
#touch "$log"
echo "name, command, input, output, lat, gates, edge, area, delay, lev, stime_gates, stime_gates%, stime_cap(ff), stime_cap%, stime_Area, stime_Area%, stime_Delay(ps), stime_Delay%, cut_time, delay_time, total_time" >> $csv

files=$(find "$2" -name "*.aig")

for element in ${files[@]}
do
    echo "process $element"
    command="read_lib $4 ;read_aiger $element; map -r -v ;  print_stats; topo;  stime";
    outputs=$(timeout $3 $1 -c "$command";)
    echo $outputs >> $log

    numbers=($(echo $outputs | grep -Eo '[0-9]+(\.[0-9]+)?'))
    size=${#numbers[@]}
    name=$(echo "$element" | awk -F "/" '{print $NF}')

    times=($(echo $outputs | grep -Eo '[0-9]+\.[0-9]+ sec' | grep -Eo '[0-9]+\.[0-9]+'))
    timesize=${#times[@]}

    ret="$name, $command, ${numbers[$size-28]}, ${numbers[$size-27]}, ${numbers[$size-26]},  ${numbers[$size-25]}, ${numbers[$size-24]}, ${numbers[$size-23]}, ${numbers[$size-22]}, ${numbers[$size-21]}, ${numbers[$size-18]}, ${numbers[$size-16]}, ${numbers[$size-13]}, ${numbers[$size-11]}, ${numbers[$size-8]}, ${numbers[$size-6]}, ${numbers[$size-3]},   ${times[$timesize-4]}, ${times[$timesize-3]}, ${times[$timesize-2]}, ${times[$timesize-1]}"
    echo $ret >> $csv
done

#####################################################################
##csv="${timestamp}_$binary.map.csv"
#echo "name, command, input, output, lat, gates, edge, area, delay, lev, stime_gates, stime_gates%, stime_cap(ff), stime_cap%, stime_Area, stime_Area%, stime_Delay(ps), stime_Delay%, cut_time, delay_time, area1_time, area2_time, area3_time, total_time" >> $csv
#
#files=$(find "$2" -name "*.aig")
#
#for element in ${files[@]}
#do
#    echo "process $element"
#    command="read_lib $4 ;read_aiger $element;   map -v ;  print_stats; topo;  stime";
#    outputs=$(timeout $3 $1 -c "$command";)
#    echo $outputs >> $log
#
#    numbers=($(echo $outputs | grep -Eo '[0-9]+(\.[0-9]+)?'))
#    size=${#numbers[@]}
#    name=$(echo "$element" | awk -F "/" '{print $NF}')
#
#    times=($(echo $outputs | grep -Eo '[0-9]+\.[0-9]+ sec' | grep -Eo '[0-9]+\.[0-9]+'))
#    timesize=${#times[@]}
#
#    ret="$name, $command, ${numbers[$size-28]}, ${numbers[$size-27]}, ${numbers[$size-26]},  ${numbers[$size-25]}, ${numbers[$size-24]}, ${numbers[$size-23]}, ${numbers[$size-22]}, ${numbers[$size-21]}, ${numbers[$size-18]}, ${numbers[$size-16]}, ${numbers[$size-13]}, ${numbers[$size-11]}, ${numbers[$size-8]}, ${numbers[$size-6]}, ${numbers[$size-3]}, ${numbers[$size-1]}, ${times[$timesize-6]}, ${times[$timesize-5]}, ${times[$timesize-4]}, ${times[$timesize-3]}, ${times[$timesize-2]}, ${times[$timesize-1]}"
#    echo $ret >> $csv
#done
#
#####################################################################
#echo "name, command, input, output, lat, gates, edge, area, delay, lev, stime_gates, stime_gates%, stime_cap(ff), stime_cap%, stime_Area, stime_Area%, stime_Delay(ps), stime_Delay%, cut_time, delay_time, area1_time, area2_time, area3_time, total_time" >> $csv
#files=$(find "$2" -name "*.aig")
#
#for element in ${files[@]}
#do
#    echo "process $element"
#    command="read_lib $4 ;read_aiger $element;  map -f -v ;  print_stats; topo;  stime";
#    outputs=$(timeout $3 $1 -c "$command";)
#    echo $outputs >> $log
#
#    numbers=($(echo $outputs | grep -Eo '[0-9]+(\.[0-9]+)?'))
#    size=${#numbers[@]}
#    name=$(echo "$element" | awk -F "/" '{print $NF}')
#
#    times=($(echo $outputs | grep -Eo '[0-9]+\.[0-9]+ sec' | grep -Eo '[0-9]+\.[0-9]+'))
#    timesize=${#times[@]}
#
#    ret="$name, $command, ${numbers[$size-28]}, ${numbers[$size-27]}, ${numbers[$size-26]},  ${numbers[$size-25]}, ${numbers[$size-24]}, ${numbers[$size-23]}, ${numbers[$size-22]}, ${numbers[$size-21]}, ${numbers[$size-18]}, ${numbers[$size-16]}, ${numbers[$size-13]}, ${numbers[$size-11]}, ${numbers[$size-8]}, ${numbers[$size-6]}, ${numbers[$size-3]}, ${numbers[$size-1]}, ${times[$timesize-6]}, ${times[$timesize-5]}, ${times[$timesize-4]}, ${times[$timesize-3]}, ${times[$timesize-2]}, ${times[$timesize-1]}"
#    echo $ret >> $csv
#done
