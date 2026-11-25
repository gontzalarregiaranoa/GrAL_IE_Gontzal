"""GrAL-eko datu-grafiko guztiak estilo berarekin sortzen ditu (GrAL/Irudiak/*.png).

  gaua.png              lehen gaueko proba, 01:40-02:46 tartea (2026-02-11), batez besteko mugikorra
  goiza.png             leihoa irekitzeko proba (2026-02-11, 07:40-07:54)
  gaua2.png             bigarren gaueko proba (2026-11-24/25)
  irismen_rssi_snr.png  irismen-proba SF7 (LoRa.h)        -> datuak/irismen-proba_SF7_LoRa.csv
  irismen_sf12_rssi.png irismen-proba SF12 (RadioLib)     -> datuak/irismen-proba_SF12_RadioLib.csv

Orduak ordu lokalean (Europe/Madrid). Erabilera:  python iot/grafikoak.py
"""
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.dates as mdates
import matplotlib.pyplot as plt
import matplotlib.ticker as mticker
import numpy as np
import pandas as pd
from matplotlib import font_manager

ROOT = Path(__file__).resolve().parent.parent
IOT = ROOT / "iot" / "datuak"
IRIS = ROOT / "datuak"
OUT = ROOT / "GrAL" / "Irudiak"

# --- Estiloa: memoriaren letra-tipoa (EHU Serif), koma hamartarra ---------------------------
for f in (ROOT / "GrAL" / "EHU Serif").glob("*.otf"):
    font_manager.fontManager.addfont(str(f))
plt.rcParams.update({
    "font.family": "EHUSerif", "font.size": 10, "axes.titlesize": 10, "axes.labelsize": 10,
    "legend.fontsize": 9, "xtick.labelsize": 9, "ytick.labelsize": 9,
    "axes.grid": True, "grid.alpha": 0.3, "grid.linewidth": 0.6,
    "axes.spines.top": False, "axes.spines.right": False,
    "axes.titlelocation": "left", "axes.titleweight": "bold",
    "figure.dpi": 100, "savefig.dpi": 200, "savefig.bbox": "tight", "savefig.pad_inches": 0.05,
    "legend.frameon": False, "lines.linewidth": 1.5,
})
C_PPM, C_T, C_S, C_H = "#2e8b3a", "#c62828", "#ef8c1a", "#1f6fb2"
C_RSSI, C_SNR = "#1f4fb2", "#2e8b3a"
GRIS = "0.55"


def koma(dec):
    return mticker.FuncFormatter(lambda v, _: f"{v:.{dec}f}".replace(".", ",").replace("-", "−"))


def ordu_ardatza(ax, locator, xlabel):
    ax.xaxis.set_major_locator(locator)
    ax.xaxis.set_major_formatter(mdates.DateFormatter("%H:%M"))
    ax.set_xlabel(xlabel)


def hiru_panel(height=6.0):
    fig, ax = plt.subplots(3, 1, figsize=(6.3, height), sharex=True, constrained_layout=True)
    return fig, ax


def irakurri_otsaila():
    d = pd.read_csv(IOT / "proba_gaua_goiza_2026-02-11.csv")
    d.columns = ["time", "hum", "ppm", "sens", "temp"]
    d["time"] = pd.to_datetime(d["time"].str[:19])
    d = d.dropna()
    hondatuak = (d.ppm > 1023) | (d.sens > 60) | (d.temp > 60) | (d.hum > 100)  # CRC gabeko paketeak
    return d[~hondatuak].set_index("time")


def panelak(ax, d, gordinak=None, markers=False):
    """MQ-135 / tenperatura+sentsazioa / hezetasuna. `gordinak` emanez gero, atzean argi marraztu."""
    mk = dict(marker="o", ms=3.5) if markers else {}
    if gordinak is not None:
        ax[0].plot(gordinak.index, gordinak.ppm, color=C_PPM, lw=0.8, alpha=0.3)
        ax[1].plot(gordinak.index, gordinak.temp, color=C_T, lw=0.8, alpha=0.3)
        ax[2].plot(gordinak.index, gordinak.hum, color=C_H, lw=0.8, alpha=0.3)
    ax[0].plot(d.index, d.ppm, color=C_PPM, **mk)
    ax[0].set_title("Airearen kalitatea (MQ-135aren irakurketa gordina)")
    ax[0].set_ylabel("ADC (0–1023)")
    ax[1].plot(d.index, d.temp, color=C_T, label="Tenperatura", **mk)
    ax[1].plot(d.index, d.sens, color=C_S, ls="--", label="Sentsazio termikoa", **mk)
    ax[1].set_title("Tenperatura eta sentsazio termikoa")
    ax[1].set_ylabel("°C")
    ax[2].plot(d.index, d.hum, color=C_H, **mk)
    ax[2].set_title("Hezetasun erlatiboa")
    ax[2].set_ylabel("%")
    for a in ax:
        a.margins(x=0.01)


# --- 1. Lehen gaueko proba: 01:40-02:46, 80 min-ko batez besteko mugikor zentratua -----------
def gaua():
    d = irakurri_otsaila()
    leuna = d.rolling(40, center=True, min_periods=1).mean()   # 40 × 2 min
    t0, t1 = "2026-02-11 01:40", "2026-02-11 02:46"
    fig, ax = hiru_panel()
    panelak(ax, leuna.loc[t0:t1], gordinak=d.loc[t0:t1])
    ax[0].yaxis.set_major_formatter(koma(0))
    ax[1].yaxis.set_major_formatter(koma(2))
    ax[2].yaxis.set_major_formatter(koma(1))
    ax[1].plot([], [], color="0.4", lw=0.8, alpha=0.4, label="2 min-ko batez bestekoak")
    ax[1].legend(loc="lower right", bbox_to_anchor=(1, 0.98), ncol=3, fontsize=8)
    ordu_ardatza(ax[2], mdates.MinuteLocator(byminute=range(0, 60, 10)), "Ordua (2026-02-11)")
    fig.savefig(OUT / "gaua.png")
    plt.close(fig)
    print("gaua:", leuna.loc[t0:t1].iloc[[0, -1]].round(2).to_dict("list"))


# --- 2. Leihoa irekitzeko proba (2026-02-11 goiza) --------------------------------------------
def goiza():
    d = irakurri_otsaila().loc["2026-02-11 07:40":"2026-02-11 07:54"]
    fig, ax = hiru_panel(5.6)
    panelak(ax, d, markers=True)
    ax[0].yaxis.set_major_formatter(koma(0))
    ax[1].yaxis.set_major_formatter(koma(1))
    ax[2].yaxis.set_major_formatter(koma(0))
    ax[1].legend(loc="upper right")
    ordu_ardatza(ax[2], mdates.MinuteLocator(byminute=range(0, 60, 2)), "Ordua (2026-02-11)")
    fig.savefig(OUT / "goiza.png")
    plt.close(fig)
    print("goiza:", d.round(2).to_dict("list"))


# --- 3. Bigarren gaueko proba (2026-11-24/25) -------------------------------------------------
def gaua2():
    d = pd.read_csv(IOT / "proba_gaua_2026-11-25_2min.csv")
    d["time"] = pd.to_datetime(d["time"].str[:19])
    d = d.rename(columns=lambda c: c.replace("mqtt_consumer.mean_", "")).set_index("time")
    leihoa = (pd.Timestamp("2026-11-25 07:45"), pd.Timestamp("2026-11-25 07:59"))
    etena = (pd.Timestamp("2026-11-24 22:42:45"), pd.Timestamp("2026-11-24 22:46:25"))
    fig, ax = hiru_panel()
    panelak(ax, d)
    ax[0].yaxis.set_major_formatter(koma(0))
    ax[1].yaxis.set_major_formatter(koma(1))
    ax[2].yaxis.set_major_formatter(koma(0))
    for a in ax:
        a.axvspan(*leihoa, color=GRIS, alpha=0.3, lw=0)
        a.axvspan(*etena, color="#7b4fa0", alpha=0.3, lw=0)
    y = ax[0].get_ylim()[1]
    ax[0].text(leihoa[0], y, "leihoa\nzabalik ", ha="right", va="top", fontsize=8, color="0.3")
    ax[0].text(etena[1], y, " atebidea\n geldirik", ha="left", va="top", fontsize=8, color="#5a3a78")
    ax[1].legend(loc="lower left")
    ordu_ardatza(ax[2], mdates.HourLocator(), "Ordua (2026-11-24/25)")
    fig.savefig(OUT / "gaua2.png")
    plt.close(fig)


# --- 4. Irismen-probak (RSSI eta SNR) ---------------------------------------------------------
def segmentuak(df, max_jauzia):
    """Lerroa eten pakete asko galdu diren tokietan (bestela, daturik gabeko tarteak lerro zuzenez lotzen dira)."""
    df = df.reset_index(drop=True)
    eten = df.pkt.diff() > max_jauzia
    out = df.astype(float).copy()
    rows = []
    for i in range(len(df)):
        if eten.iloc[i]:
            rows.append({"pkt": np.nan, "rssi": np.nan, "snr": np.nan})
        rows.append(out.iloc[i].to_dict())
    return pd.DataFrame(rows)


def irismen(csv, fitx, sens, sens_label, faseak=None, distantziak=None, max_jauzia=10):
    df = pd.read_csv(csv)
    s = segmentuak(df, max_jauzia)
    fig, ax = plt.subplots(2, 1, figsize=(6.3, 4.8), sharex=True, constrained_layout=True)
    for a, col, c, lab in ((ax[0], "rssi", C_RSSI, "RSSI (dBm)"), (ax[1], "snr", C_SNR, "SNR (dB)")):
        a.plot(s.pkt, s[col], color=c, lw=1.0)
        a.plot(df.pkt, df[col], "o", color=c, ms=2.2)
        a.set_ylabel(lab)
        a.yaxis.set_major_formatter(koma(0))
    ax[0].axhline(sens, color="#c62828", ls="--", lw=1)
    ax[0].text(ax[0].get_xlim()[0], sens, f"  {sens_label}", va="bottom", fontsize=8, color="#c62828")
    ax[1].axhline(0, color="0.3", lw=0.8, ls=":")
    ax[1].set_xlabel("Pakete-zenbakia")
    if faseak:
        for (a0, a1, kol, lab) in faseak:
            for a in ax:
                a.axvspan(a0, a1, color=kol, alpha=0.18, lw=0)
            ax[0].text((a0 + a1) / 2, ax[0].get_ylim()[1], lab, ha="center", va="top", fontsize=8, color="0.25")
    if distantziak:
        for p, lab in distantziak:
            for a in ax:
                a.axvline(p, color="0.5", ls=":", lw=0.9)
            ax[0].text(p, ax[0].get_ylim()[1], lab, ha="center", va="bottom", fontsize=8, color="0.3")
    fig.savefig(OUT / fitx)
    plt.close(fig)


if __name__ == "__main__":
    gaua()
    goiza()
    gaua2()
    sf7, sf12 = IRIS / "irismen-proba_SF7_LoRa.csv", IRIS / "irismen-proba_SF12_RadioLib.csv"
    if sf7.exists():
        irismen(sf7, "irismen_rssi_snr.png", -126, "sentsibilitate-muga praktikoa (≈ −126 dBm)",
                faseak=[(447, 480, "#2e8b3a", "1"), (480, 535, "#9ccc65", "2"), (535, 605, "#ef8c1a", "3"),
                        (605, 975, "#c62828", "4"), (975, 1063, "#1f6fb2", "5")])
    if sf12.exists():
        irismen(sf12, "irismen_sf12_rssi.png", -136, "SX1276 SF12 sentsibilitate nominala (−136 dBm)",
                distantziak=[(65, "350 m"), (111, "507 m"), (167, "672 m"), (210, "775 m")])
