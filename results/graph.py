import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("data.csv")

df["name"] = df["name"].str.replace("BM_", "")
df["size"] = df["name"].str.extract("/(\d+)").astype(int)
df["algo"] = df["name"].str.extract("([^/]+)/")

plt.figure(figsize=(6, 4), dpi=300)

for algo in df["algo"].unique():
    data = df[df["algo"] == algo]
    if algo == "StdLowerBound":
        plt.plot(data["size"], data["cpu_time"], label=algo, alpha=0.2)
    else:
        plt.plot(
            data["size"],
            data["cpu_time"],
            label=algo,
        )

plt.axvline(x=15, color="black", linestyle="--", alpha=0.5)
plt.axvline(x=20, color="black", linestyle="--", alpha=0.5)
plt.axvline(x=24, color="black", linestyle="--", alpha=0.5)

plt.grid(True)
plt.xlabel("Array Length (2^n)")
plt.ylabel("Reciprocal Throughput (ns)")
plt.title("lower_bound() comparison")
plt.ylim(0, 250)
plt.legend()

plt.tight_layout()
plt.savefig("plot.png", dpi=300, bbox_inches="tight")
plt.close()
