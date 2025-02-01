import pandas as pd
import matplotlib.pyplot as plt
import numpy as np


df = pd.read_csv("data.csv")
df["name"] = df["name"].str.replace("BM_", "")
df["size"] = df["name"].str.extract("/(\d+)").astype(int)
df["algo"] = df["name"].str.extract("([^/]+)/")

plt.figure(figsize=(12, 6))

for algo in df["algo"].unique():
    data = df[df["algo"] == algo]
    if algo == "StdLowerBound":
        plt.plot(data["size"], data["cpu_time"], label=algo, marker="o", alpha=0.2)
    else:
        plt.plot(data["size"], data["cpu_time"], label=algo, marker="o")

plt.axvline(x=15, color="black", linestyle="--", alpha=0.5)
plt.axvline(x=20, color="black", linestyle="--", alpha=0.5)
plt.axvline(x=24, color="black", linestyle="--", alpha=0.5)

plt.grid(True)
plt.xlabel("Array Length (2^n)")
plt.ylabel("Reciprocal Throughput (ns)")
plt.title("Algorithm Performance Comparison")
plt.ylim(0, 250)
plt.legend()
plt.show()
