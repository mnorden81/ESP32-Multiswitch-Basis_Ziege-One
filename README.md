# ESP32-MultiSwitch

RC-gesteuerter 8-Kanal-Schalter für ESP32 mit Web-Interface (CRSF/SBUS).

Das Modul empfängt Schaltbefehle über CRSF (ExpressLRS/TBS Crossfire, bidirektional) oder SBUS (FrSky, FlySky, ELRS, HoTT) und schaltet acht Ausgänge für Licht, Blinker, Relais oder PWM-Verbraucher. Konfiguriert wird über eine eingebaute Weboberfläche (eigener WLAN-Access-Point) und bei CRSF zusätzlich über das Lua-Menü am Sender.

- 8 Ausgänge: Ein/Aus, PWM (fest oder proportional), 12 Blink-Modi
- Mehrere Module am selben Empfänger (WM-Multiswitch-Protokoll, eigene CRSF-Adresse je Modul)
- OTA-Update per WLAN, WLAN-Failsafe, Konfiguration als JSON sichern/laden
- ab v3: zusätzliche Kanalquellen MKan1/MKan2 und bis zu 3 Konfigurationen je Ausgang

Dieses Repository enthält drei eigenständige, unabhängig voneinander baubare Projekte. Jedes liegt in einem eigenen Sketch-Ordner, dessen Name zur `.ino`-Datei passt (Arduino IDE).

| Ordner | Version | Beschreibung |
|--------|---------|--------------|
| [`MultiSwitch_ESP32/`](MultiSwitch_ESP32/) | v2.03 | Linie v2.xx. |
| [`MultiSwitch_ESP32_V3/`](MultiSwitch_ESP32_V3/) | v3.12 | Eigenständiger Zweig, abgezweigt von v2.02: MKan-Mehrfachadress-Erweiterung und Mehrfachkonfiguration pro Ausgang. |
| [`MultiSwitch_ESP32_TXRX/`](MultiSwitch_ESP32_TXRX/) | v3.12 | Gleicher Firmwarestand wie V3, aber mit der Pinbelegung der Platine **Multiswitch_TXRX** (ULN2803A). |

Details, Änderungshistorie, Pin-Belegung und API stehen in der jeweiligen `README.md` im Projektordner.

## Projektbeschreibungen (Word)

| Variante | Dokument |
|--------|---------|
| v2.xx | [`MultiSwitch_ESP32/ESP32-MultiSwitch-V2-Projektbeschreibung-v2.00.docx`](MultiSwitch_ESP32/ESP32-MultiSwitch-V2-Projektbeschreibung-v2.00.docx) |
| V3 | [`MultiSwitch_ESP32_V3/ESP32-MultiSwitch-V3-Projektbeschreibung-v3.12.docx`](MultiSwitch_ESP32_V3/ESP32-MultiSwitch-V3-Projektbeschreibung-v3.12.docx) |
| TXRX | [`MultiSwitch_ESP32_TXRX/ESP32-MultiSwitch-TXRX-Projektbeschreibung-v3.12.docx`](MultiSwitch_ESP32_TXRX/ESP32-MultiSwitch-TXRX-Projektbeschreibung-v3.12.docx) |

*v2.00 erstellt von: PiperPilot*
