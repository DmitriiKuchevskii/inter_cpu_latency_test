import seaborn as sns
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import sys

RECORD_SIZE = 2 * 4 + 8 * 14 # 2 CPU Ids and 14 timestamps

CPUS = 0
LATENCIES_STAT_FILE = sys.argv[1]

with open(LATENCIES_STAT_FILE, 'rb') as file:
    while True:
        record = file.read(RECORD_SIZE)
        if not record : break
        CPUS = max(CPUS, int.from_bytes(record[:4], byteorder='little'))

data = [[0 for _ in range(CPUS + 1)] for _ in range(CPUS + 1)]

with open(LATENCIES_STAT_FILE, 'rb') as file:
    while True:
        record = file.read(RECORD_SIZE)
        if not record : break

        fromCpuId = int.from_bytes(record[:4], byteorder='little')
        toCpuId = int.from_bytes(record[4:8], byteorder='little')
        l99 = int.from_bytes(record[11 * 8 : 12 * 8], byteorder='little')
        data[fromCpuId][toCpuId] = l99


df = pd.DataFrame(data, columns=[f'Core_{i}' for i in range(0, 6)],
                  index=[f'Core_{i}' for i in range(0, 6)])


plt.figure(figsize=(8, 6)) # Optional: Adjust figure size
sns.heatmap(df, annot=True, cmap='viridis', fmt=".1f", linewidths=.5)
plt.title('Transmission latency 99%%')
plt.show()