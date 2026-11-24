"""InfluxDBko datuak CSVra esportatu (gaueko proba).

Bi fitxategi sortzen ditu iot/datuak/ karpetan:
  - <izena>_gordinak.csv : puntu guztiak (10 s-tik behin), ordu lokalarekin
  - <izena>_2min.csv     : 2 minutuko batez bestekoak, Chronografetik esportatutako
                            CSVaren formatu berean (aurreko gauarekin alderatzeko)

Erabilera:  python esportatu.py 2026-11-24T22:00 2026-11-25T09:00 proba_gaua_2026-11-25
(orduak ordu lokalean, Europe/Madrid)
"""
import csv
import json
import sys
import urllib.parse
import urllib.request
from datetime import datetime
from pathlib import Path
from zoneinfo import ZoneInfo

INFLUX = "http://localhost:8086/query"
DB = "sentsore_db"
TZ = ZoneInfo("Europe/Madrid")
OUT = Path(__file__).parent / "datuak"


def kontsulta(q):
    url = INFLUX + "?" + urllib.parse.urlencode({"db": DB, "q": q, "epoch": "ms"})
    res = json.load(urllib.request.urlopen(url))["results"][0]
    if "series" not in res:
        return [], []
    s = res["series"][0]
    return s["columns"], s["values"]


def utc(t):
    return datetime.fromisoformat(t).replace(tzinfo=TZ).astimezone(ZoneInfo("UTC")).strftime("%Y-%m-%dT%H:%M:%SZ")


def lokala(ms):
    return datetime.fromtimestamp(ms / 1000, TZ).isoformat(timespec="milliseconds")


def main(hasi, amaitu, izena):
    OUT.mkdir(exist_ok=True)
    tartea = f"time >= '{utc(hasi)}' AND time < '{utc(amaitu)}'"

    _, rows = kontsulta(f"SELECT ts, ppm, hum, temp, sens FROM mqtt_consumer WHERE {tartea}")
    with open(OUT / f"{izena}_gordinak.csv", "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["time", "ts", "ppm", "hum", "temp", "sens"])
        for r in rows:
            w.writerow([lokala(r[0])] + r[1:])

    _, mean = kontsulta(
        f"SELECT mean(hum), mean(ppm), mean(sens), mean(temp) FROM mqtt_consumer "
        f"WHERE {tartea} GROUP BY time(2m) fill(none)")
    with open(OUT / f"{izena}_2min.csv", "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f, quoting=csv.QUOTE_NONNUMERIC)
        w.writerow(["time", "mqtt_consumer.mean_hum", "mqtt_consumer.mean_ppm",
                    "mqtt_consumer.mean_sens", "mqtt_consumer.mean_temp"])
        for r in mean:
            w.writerow([lokala(r[0])] + r[1:])

    print(f"{len(rows)} puntu gordin, {len(mean)} 2 min-ko tarte -> {OUT}")


if __name__ == "__main__":
    if len(sys.argv) != 4:
        sys.exit(__doc__)
    main(*sys.argv[1:])
