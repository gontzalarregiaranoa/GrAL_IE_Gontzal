"""Gaueko eta goizeko proben grafikoak sortzen ditu Chronografetik esportatutako CSVetatik.

Erabilera: python grafikoak.py <csv-karpeta> <irteera-karpeta>
CSVa: datuak/proba_gaua_goiza_2026-02-11.csv (2 minutuko batez bestekoak, InfluxDB).
Pakete hondatuak (CRC gabe jasotakoak) dituzten tarteak hutsik uzten dira: MQ-135 > 1023 edo sentsazio termikoa > 60 °C.
"""
import sys
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.dates as mdates
import matplotlib.pyplot as plt
import pandas as pd

src = Path(sys.argv[1]) / "proba_gaua_goiza_2026-02-11.csv"
out = Path(sys.argv[2])

d = pd.read_csv(src, parse_dates=["time"])
d.columns = ["time", "hum", "ppm", "sens", "temp"]
d = d.dropna()
d["time"] = d["time"].dt.tz_localize(None)
hondatuak = (d.ppm > 1023) | (d.sens > 60) | (d.temp > 60) | (d.hum > 100)
print("2 min-ko tarte hondatuak:", int(hondatuak.sum()))
d.loc[hondatuak, ["hum", "ppm", "sens", "temp"]] = float("nan")  # lerroa eten egiten da

plt.rcParams.update({
    "font.family": "serif", "font.size": 9, "axes.titlesize": 9,
    "axes.labelsize": 9, "legend.fontsize": 8, "xtick.labelsize": 8, "ytick.labelsize": 8,
    "axes.grid": True, "grid.alpha": 0.3, "axes.spines.top": False, "axes.spines.right": False,
})
C_PPM, C_T, C_S, C_H = "#4a3f8f", "#c0392b", "#e67e22", "#2471a3"


def irudia(df, fitx, tarte_min):
    fig, ax = plt.subplots(3, 1, figsize=(6.3, 5.4), sharex=True, constrained_layout=True)
    ax[0].plot(df.time, df.ppm, color=C_PPM, lw=1.4, marker="o", ms=2.5)
    ax[0].set_ylabel("MQ-135\n(ADC, 0–1023)")
    ax[1].plot(df.time, df.temp, color=C_T, lw=1.4, marker="o", ms=2.5, label="Tenperatura")
    ax[1].plot(df.time, df.sens, color=C_S, lw=1.4, ls="--", label="Sentsazio termikoa")
    ax[1].set_ylabel("Tenperatura (°C)")
    ax[1].legend(loc="best", frameon=False)
    ax[2].plot(df.time, df.hum, color=C_H, lw=1.4, marker="o", ms=2.5)
    ax[2].set_ylabel("Hezetasun\nerlatiboa (%)")
    ax[2].set_xlabel("Ordua (2026-02-11)" if df.time.iloc[0].hour < 12 else "Ordua (2026-02-10/11)")
    ax[2].xaxis.set_major_locator(mdates.MinuteLocator(interval=tarte_min) if tarte_min < 60 else mdates.HourLocator())
    ax[2].xaxis.set_major_formatter(mdates.DateFormatter("%H:%M"))
    for a in ax:
        a.margins(x=0.01)
    fig.savefig(out / fitx)
    plt.close(fig)


gaua = d[(d.time >= "2026-02-10 23:00") & (d.time <= "2026-02-11 03:30")]
goiza = d[d.time >= "2026-02-11 07:30"]
irudia(gaua, "gaua.pdf", 60)
irudia(goiza, "goiza.pdf", 2)
print("gaua:", gaua.time.min(), "->", gaua.time.max(), len(gaua), "puntu")
print(gaua.iloc[[0, -1]][["time", "ppm", "temp", "sens", "hum"]].to_string())
print("goiza:", goiza[["time", "ppm", "temp", "sens", "hum"]].to_string())
