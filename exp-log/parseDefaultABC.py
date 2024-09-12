import sys
import re

# Check if the file path is provided as a command-line argument
if len(sys.argv) < 2:
    print("Usage: python script.py <file_name>")
    sys.exit(1)

# The first command-line argument after the script name is the file path
file_name = sys.argv[1]

  
arrays_arith = ['log2', 'square', 'adder', 'sin', 'div', 'hyp', 'max', 'sqrt', 'multiplier', 'bar', 'dft', 'netcard']
arrays_control = ['priority', 'cavlc', 'arbiter', 'i2c', 'voter', 'int2float', 'ctrl', 'dec', 'mem_ctrl', 'router']

 
parseRes = dict()
type = 'arith'
 
 
parseRes1, parseRes2, parseRes3 = dict(), dict(), dict()

def parseLine(text_data, circuit_name, log_order_id):
    
    if (log_order_id == 0 ): cur_parseRes = parseRes1
    elif (log_order_id == 1): cur_parseRes = parseRes2
    else: cur_parseRes = parseRes3
    
    nd_value, edge_value, area_value, delay_value, sarea_value, sdelay_value = 0, 0, 0, 0, 0, 0
    nd_match = re.search(r"nd\s*=\s*(\d+)", text_data)
    if nd_match:
        nd_value = int(nd_match.group(1))
    else:
        print("No match found for nd")

    edge_match = re.search(r"edge\s*=\s*(\d+)", text_data)
    if edge_match:
        edge_value = int(edge_match.group(1))
    else:
        print("No match found for edge")

    level_match = re.search(r"lev\s*=\s*(\d+)", text_data)
    if level_match:
        level_value = int(level_match.group(1))
    else:
        print("No match found for level")

    area_match = re.search(r"area\s*=\s*(\d+\.\d+)", text_data)
    if area_match:
        area_value = float(area_match.group(1))
    else:
        print("No match found for area")

    delay_match = re.search(r"delay\s*=\s*(\d+\.\d+)", text_data)
    if delay_match:
        delay_value = float(delay_match.group(1))
    else:   
        print("No match found for delay")

    sarea_match = re.search(r"Area\s*=\s*(\d+\.\d+)", text_data)
    if sarea_match:
        sarea_value = float(sarea_match.group(1))
    else:
        print("No match found for stime area")

    sdelay_match = re.search(r"Delay\s*=\s*(\d+\.\d+)", text_data)
    if sdelay_match:
        sdelay_value = float(sdelay_match.group(1))
    else:
        print("No match found for stime delay")
    
    cur_parseRes[cir_name] = (nd_value, edge_value, level_value, area_value, delay_value, sarea_value, sdelay_value) 
    # print(f'{circuit_name}, {nd_value}, {edge_value}, {area_value}, {delay_value}, {sarea_value}, {sdelay_value}')
    


def get_cir_name (line):
    res = 0
    for cir in arrays_arith:
        if cir in line: return cir 
    # for cir in arrays_control:
    #     if cir in line: return cir 
    for i, cir in enumerate(arrays_control):
        tmp_cir = "/{}:".format(cir)
        if tmp_cir in line: return cir
    return res      
 
with open(file_name, 'r') as file:
    # Iterate over each line in the file
    firstResults = 1
    for line in file:
        cir_name = get_cir_name(line)
        if cir_name == 0: continue 
        
        parts = line.split("{}:".format(cir_name))
        if len(parts) != 4:  print("parse the first log line of {} error".format(cir_name))
        parseLine(parts[1], cir_name, 0)
        parseLine(parts[2], cir_name, 1)
        parseLine(parts[3], cir_name, 2)
 
        if (line.find("random_control") != -1):
            type = 'control'

# with open(file_name, 'r') as file:
#     # Iterate over each line in the file
#     firstResults = 1
#     for line in file:
#         cir_name = get_cir_name(line)
#         if cir_name == 0: continue 
#         elif firstResults == 1: 
#             parts = line.split("{}:".format(cir_name))
#             if len(parts) != 3:  print("parse the first log line of {} error".format(cir_name))
#             parseLine(parts[1], cir_name, 0)
#             parseLine(parts[2], cir_name, 1)
#             firstResults = 0 
#         else: 
#             parts = line.split("{}:".format(cir_name))
#             if len(parts) != 2:  print("parse the second log line of {} error".format(cir_name))
#             parseLine(parts[1], cir_name, 2) 
#             firstResults = 1
             
#         if (line.find("random_control") != -1):
#             type = 'control'


if type == 'arith':
    print(" , #Gate, #Edge, #Level,  #area, #delay, #NLDM Area, #NLDM Delay")
    for key in arrays_arith:
        if key in parseRes1:
            nd_value, edge_value, level_value, area_value, delay_value, sarea_value, sdelay_value = parseRes1[key]
            print(f'{key}, {nd_value}, {edge_value}, {level_value}, {area_value}, {delay_value}, {sarea_value}, {sdelay_value}')
        else:
            print("No match found")
    print("\n")
    for key in arrays_arith:
        if key in parseRes2:
            nd_value, edge_value, level_value, area_value, delay_value, sarea_value, sdelay_value = parseRes2[key]
            print(f'{key}, {nd_value}, {edge_value}, {level_value}, {area_value}, {delay_value}, {sarea_value}, {sdelay_value}')
        else:
            print("No match found")
    print("\n")
    for key in arrays_arith:
        if key in parseRes3:
            nd_value, edge_value, level_value, area_value, delay_value, sarea_value, sdelay_value = parseRes3[key]
            print(f'{key}, {nd_value}, {edge_value}, {level_value}, {area_value}, {delay_value}, {sarea_value}, {sdelay_value}')
        else:
            print("No match found")

elif type == 'control':
    print(" , #Gate, #Edge, #Level,  #area, #delay, #NLDM Area, #NLDM Delay")
    for key in arrays_control:
        if key in parseRes1:
            nd_value, edge_value, level_value, area_value, delay_value, sarea_value, sdelay_value = parseRes1[key]
            print(f'{key}, {nd_value}, {edge_value}, {level_value}, {area_value}, {delay_value}, {sarea_value}, {sdelay_value}')
        else:
            print("No match found")
    print("\n")
    for key in arrays_control:
        if key in parseRes2:
            nd_value, edge_value, level_value, area_value, delay_value, sarea_value, sdelay_value = parseRes2[key]
            print(f'{key}, {nd_value}, {edge_value}, {level_value},  {area_value}, {delay_value}, {sarea_value}, {sdelay_value}')
        else:
            print("No match found")
    print("\n")
    for key in arrays_control:
        if key in parseRes3:
            nd_value, edge_value, level_value, area_value, delay_value, sarea_value, sdelay_value = parseRes3[key]
            print(f'{key}, {nd_value}, {edge_value}, {level_value},  {area_value}, {delay_value}, {sarea_value}, {sdelay_value}')
        else:
            print("No match found")
