import matplotlib.pyplot as plt
import numpy as np 

# 数据
labels = ['adder', 'bar', 'log2', 'cavlc', 'int2float', 'ctrl']
estimated_delays = [2613.78, 152.96, 3891.66, 185.07, 174.27, 98.53]
actual_delays = [3770.65, 1114.9, 6797.77, 93.2, 91.7, 89.9]
delta_delays = [44, 629, 75, 50, 47, 9]  # Delta Delay % for scatter plot

# 创建图形和轴
fig, ax1 = plt.subplots(figsize=(10, 6))

# 绘制条形图
bar_width = 0.35
x = np.arange(len(labels))
bars1 = ax1.bar(x - bar_width/2, estimated_delays, bar_width, label='Estimated Delays', color='b')
bars2 = ax1.bar(x + bar_width/2, actual_delays, bar_width, label='Actual Delays', color='r')

ax1.set_xlabel('Circuits')
ax1.set_ylabel('Delay (ps)', color='b')
ax1.set_title('Combo Chart of Estimated and Actual Delays with Delta Delay Scatter')
ax1.set_xticks(x)
ax1.set_xticklabels(labels)
ax1.legend(loc='upper left')

# 创建另一轴用于散点图
ax2 = ax1.twinx()
ax2.set_ylabel('Delta Delay (%)', color='g')
scatters = ax2.scatter(labels, delta_delays, color='g', label='Delta Delay (%)')
ax2.legend(loc='upper right')

plt.show()
