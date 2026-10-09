import matplotlib.pyplot as plt
import numpy as np

metrics = ["Real time (s)", "User CPU (s)", "Max RSS (MB)"]
drvm = [3.61, 3.44, 3.32]
javap = [15.12, 40.10, 103.52]

x = np.arange(len(metrics))
w = 0.36

fig, ax = plt.subplots(figsize=(10, 6))
b1 = ax.bar(x - w/2, drvm, w, label="DRVM")
b2 = ax.bar(x + w/2, javap, w, label="javap")

ax.set_ylabel("Value")
ax.set_title("DRVM vs javap  50-Run Benchmark (Tested on a M4 Mac)")
ax.set_xticks(x)
ax.set_xticklabels(metrics)
ax.legend()
ax.grid(axis="y", alpha=0.2)

for bars in (b1, b2):
    for bar in bars:
        ax.text(
            bar.get_x() + bar.get_width()/2,
            bar.get_height(),
            f"{bar.get_height():.2f}",
            ha="center", va="bottom"
        )

plt.tight_layout()
plt.savefig("drvm_vs_javap.png", dpi=300, bbox_inches="tight")
plt.show()

