import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("data.csv")

df["name"] = df["name"].str.replace("BM_", "")
df["size"] = df["name"].str.extract("/(\d+)").astype(int)
df["algo"] = df["name"].str.extract("(.*?)/")[0]

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

annotations = {15: "L1d", 20: "L2d", 25: "L3d", 27.16: "L1d TLB"}

for x, label in annotations.items():
    plt.axvline(x=x, color="black", linestyle="--", alpha=0.5)
    plt.annotate(
        label,
        xy=(x, 200),
        xytext=(5, 0),
        textcoords="offset points",
        ha="left",
        va="center",
        fontsize=8,
        bbox=dict(facecolor="white", edgecolor="none", alpha=0.7, pad=2),
    )

plt.grid(True)
plt.xlabel("Array Length (2^n)")
plt.ylabel("Reciprocal Throughput (ns)")
plt.title("lower_bound() comparison")
plt.ylim(0, 250)
plt.legend()

plt.tight_layout()
plt.savefig("plot.png", dpi=300, bbox_inches="tight")
plt.close()
