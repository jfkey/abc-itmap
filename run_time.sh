ABC_CMD="./abc -c"
ABC_PATH="/home/liujunfeng/benchmarks/arithmetic"
LIB_PATH="/home/liujunfeng/ABC/abc_itmap/abc-itmap/asap7_clean.lib"
ABC_PATH_C="/home/liujunfeng/benchmarks/random_control"

$ABC_CMD "read $ABC_PATH/div.aig; read_lib $LIB_PATH; map -e -v; topo; stime;" >> time_map_e.txt
$ABC_CMD "read $ABC_PATH/hyp.aig; read_lib $LIB_PATH; map -e -v; topo; stime;" >> time_map_e.txt
$ABC_CMD "read $ABC_PATH/sqrt.aig; read_lib $LIB_PATH; map -e -v; topo; stime;" >> time_map_e.txt
$ABC_CMD "read $ABC_PATH/multiplier.aig; read_lib $LIB_PATH; map -e -v; topo; stime;" >> time_map_e.txt
$ABC_CMD "read $ABC_PATH_C/mem_ctrl.aig; read_lib $LIB_PATH; map -e -v; topo; stime;" >> time_map_e.txt

$ABC_CMD "read $ABC_PATH/div.aig; read_lib $LIB_PATH; map  -v; topo; stime;" >> time_map.txt
$ABC_CMD "read $ABC_PATH/hyp.aig; read_lib $LIB_PATH; map  -v; topo; stime;" >> time_map.txt
$ABC_CMD "read $ABC_PATH/sqrt.aig; read_lib $LIB_PATH; map  -v; topo; stime;" >> time_map.txt
$ABC_CMD "read $ABC_PATH/multiplier.aig; read_lib $LIB_PATH; map  -v; topo; stime;" >> time_map.txt
$ABC_CMD "read $ABC_PATH_C/mem_ctrl.aig; read_lib $LIB_PATH; map  -v; topo; stime;" >> time_map.txt





