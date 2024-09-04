import sys
import re

# Check if the file path is provided as a command-line argument
if len(sys.argv) < 2:
    print("Usage: python script.py <file_path>")
    sys.exit(1)

# The first command-line argument after the script name is the file path
file_path = sys.argv[1]

 

arrays_arith = ['log2', 'square', 'adder', 'sin', 'div', 'hyp', 'max', 'sqrt', 'multiplier', 'bar']
arrays_control = ['priority', 'cavlc', 'arbiter', 'i2c', 'voter', 'int2float', 'ctrl', 'dec', 'mem_ctrl', 'router']

parseRes = dict()
type = 'arith'

def parseLine(text_data):
    # Extracting content from "best para" onwards
    start_idx = text_data.find("best para")
    if start_idx != -1:
        truncated_text = text_data[start_idx:] 
        pattern = r'Area\s*=\s*([\d.]+)\s*.*?Delay\s*=\s*([\d.]+)\s*ps.*?/(arithmetic|random_control)/([^:]*?):.*?lev\s*=\s*(\d+)'
        match = re.search(pattern, truncated_text, re.DOTALL)

        if match: 
            area, delay, dir_type, cir_name, lev = match.groups()
            key = cir_name 
            value = area + ',' + delay + ',' + lev
            parseRes[key] = value
            # print(f'{cir_name}, {area}, {delay}, {lev}')
        else:
            print("No match found") 
    else:
        print("The specified marker 'best para' was not found in the text.")


def parseLine(text_data, circuit_name, log_order_id):
    pass 
    

def get_cir_name (line):
    res = 0
    for cir in arrays_arith:
        if cir in line: return cir 
    for cir in arrays_control:
        if cir in line: return cir 
    return res      
 
with open(file_path, 'r') as file:
    # Iterate over each line in the file
    for line in file:
        firstResults = 1    
        cir_name = get_cir_name(line)
        if cir_name == 0: continue 
        elif firstResults == 1: 
            parts = line.split("{}:".format(cir_name))
            if len(parts) != 3:  print("parse the first log line of {} error".format(cir_name))
            parseLine(parts[1], cir_name, 0)
            parseLine(parts[2], cir_name, 1)
            firstResults = 0 
        else: 
            parts = line.split("{}:".format(cir_name))
            if len(parts) != 2:  print("parse the second log line of {} error".format(cir_name))
            parseLine(parts[1], cir_name, 2)

            firstResults = 1
            

        elif "for exampel" in line:
            
            parseLine(line.strip())
        
        if (line.find("random_control") != -1):
            type = 'control'

if type == 'arith':
    for key in arrays_arith:
        if key in parseRes:
            print(f'{key}, {parseRes[key]}')
        else:
            print("No match found")
elif type == 'control':
    for key in arrays_control:
        if key in parseRes:
            print(f'{key}, {parseRes[key]}')
        else:
            pprint("No match found")