"""Bigarren gaueko probaren grafikoa (2026-11-24/25), GrAL-eko irudirako.

Sarrera:  datuak/proba_gaua_2026-11-25_2min.csv  (esportatu.py-k sortua)
Irteera:  ../GrAL/Irudiak/gaua2.png

Erabilera:  python grafikoa_gaua2.py
"""
from pathlib import Path

import matplotlib.dates as mdates
import matplotlib.pyplot as plt
import pandas as pd

HERE = Path(__file__).parent
CSV = HERE / "datuak" / "proba_gaua_2026-11-25_2min.csv"
OUT = HERE.parent / "GrAL" / "Irudiak" / "gaua2.png"

LEIHOA = ("2026-11-25 07:45", "2026-11-25 07:59")     # leihoa zabalik
ETENA = ("2026-11-24 22:42:45", "2026-11-24 22:46:25")  # atebidea geldirik

d = pd.read_csv(CSV)
d["time"] = pd.to_datetime(d["time"].str[:19])
d = d.rename(columns=lambda c: c.replace("mqtt_consumer.mean_", ""))

fig, ax = plt.subplots(3, 1, figsize=(10, 7.2), sharex=True)
ax[0].plot(d.time, d.ppm, color="tab:green", lw=1.6)
ax[0].set_ylabel("MQ-135 (ADC)")
ax[0].set_title("Airearen kalitatea (MQ-135aren irakurketa gordina)")
ax[1].plot(d.time, d.temp, color="tab:red", lw=1.6, label="Tenperatura")
ax[1].plot(d.time, d.sens, color="tab:orange", lw=1.4, ls="--", label="Sentsazioa")
ax[1].set_ylabel("°C")
ax[1].set_title("Tenperatura eta sentsazio termikoa")
ax[1].legend(loc="lower left", fontsize=9)
ax[2].plot(d.time, d.hum, color="tab:blue", lw=1.6)
ax[2].set_ylabel("%")
ax[2].set_title("Hezetasun erlatiboa")

for a in ax:
    a.grid(alpha=0.3)
    a.axvspan(pd.Timestamp(LEIHOA[0]), pd.Timestamp(LEIHOA[1]), color="0.6", alpha=0.35, lw=0)
    a.axvspan(pd.Timestamp(ETENA[0]), pd.Timestamp(ETENA[1]), color="tab:purple", alpha=0.25, lw=0)
ax[0].annotate("leihoa\nzabalik", xy=(pd.Timestamp("2026-11-25 07:52"), d.ppm.max() - 15),
               ha="right", fontsize=9, xytext=(-8, 0), textcoords="offset points")
ax[0].annotate("atebidea\ngeldirik", xy=(pd.Timestamp(ETENA[1]), d.ppm.max() - 15),
               ha="left", fontsize=9, xytext=(6, 0), textcoords="offset points")

ax[2].xaxis.set_major_locator(mdates.HourLocator())
ax[2].xaxis.set_major_formatter(mdates.DateFormatter("%H:%M"))
ax[2].set_xlabel("Ordua (2026-11-24/25)")
fig.tight_layout()
fig.savefig(OUT, dpi=150)
print("->", OUT)
