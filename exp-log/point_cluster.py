from sklearn.cluster import KMeans
import numpy as np
import sys
import re

# Check if the file path is provided as a command-line argument
if len(sys.argv) < 2:
    print("Usage: python script.py <file_name1> .. <file_namen>")
    sys.exit(1)

# python point_cluster.py abc_rf_arithmetic_asap7_clean_20240910225601.log  abc_gpy_arithmetic_asap7_clean_20240910225611.log abc_de_arithmetic_asap7_clean_20240910225854.log
# python point_cluster.py abc_para_tune2_arithmetic_asap7_clean_20240910123459.log  abc_para_tune2_arithmetic_sky130_20240910123446.log abc_para_tune2_random_control_asap7_clean_20240910152246.log abc_para_tune2_random_control_sky130_20240910161515.log

file_names = sys.argv[1:]
print(file_names)

def parseLine(line):
    text_data = line.split("#### Best")
    assert len(text_data) == 2
    text_data = text_data[1]
    pattern = r"\[\d+\]=(\d+\.\d+)"
    results = re.findall(pattern, text_data)
    results = [float(value) for value in results]
    return results
    

best_paras = []
for fn in file_names:
    with open(fn, 'r') as file:
        for line in file:
            para = parseLine(line.strip())
            best_paras.append(para)

    
   
data = np.array(best_paras)

kmeans = KMeans(n_clusters=3)  # 假设我们想要将数据聚类成3类
kmeans.fit(data)

cluster_centers = kmeans.cluster_centers_

labels = kmeans.labels_

print(list(np.round(cluster_centers,2)))