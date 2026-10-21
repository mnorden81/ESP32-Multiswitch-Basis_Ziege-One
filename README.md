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
| [`MultiSwitch_ESP32/`](MultiSwitch_ESP32/) | v2.05 | Linie v2.xx. Enthält die Projektbeschreibung als Word-Dokument. |
| [`MultiSwitch_ESP32_V3/`](MultiSwitch_ESP32_V3/) | v3.14 | Eigenständiger Zweig, abgezweigt von v2.02: MKan-Mehrfachadress-Erweiterung und Mehrfachkonfiguration pro Ausgang. |
| [`MultiSwitch_ESP32_TXRX/`](MultiSwitch_ESP32_TXRX/) | v3.14 | Gleicher Firmwarestand wie V3, aber mit der Pinbelegung der Platine **Multiswitch_TXRX** (ULN2803A). |

Details, Änderungshistorie, Pin-Belegung und API stehen in der jeweiligen `README.md` im Projektordner.

Der gemeinsame Code (CRSF-Bibliothek, Lua-Menü, NVS, WLAN-Failsafe, Status-LED, OTA) liegt in jedem Sketch-Ordner als identische Kopie von **RC_Common** unter `src/RC_Common/` (dieselbe Basis nutzen RC-Multiswitch C3/S3 und die Soundmodule, siehe `src/RC_Common/README.md`). Die Arduino-IDE kompiliert den Unterordner automatisch mit. Benötigt werden zusätzlich die Bibliotheken *Bolder Flight Systems SBUS* 8.1.4 und *TinyGPSPlus* 1.1.0 (RC_Common enthält die GPS-Rolle der anderen Module; der Multiswitch nutzt sie nicht, die Datei wird aber mitkompiliert).

Die GitHub-CI prüft bei jedem Push die drei Kopien gegen `MANIFEST.sha256`, führt die Host-Tests von RC_Common aus und kompiliert alle drei Sketches (ESP32-Core 3.0.7).

## Hardware und Herkunft

Die Hardware dieses Projekts stammt von **Der RC-Modellbauer** (Frank Verfürth, auf GitHub **Ziege-One**). Die Firmware basiert auf seinem Projekt *ESP32-SBus-Switch* 0.6, die CRSF-Anbindung auf seiner Bibliothek *CRSF_ESP32*.

- Webseite und Shop: [derrcmodellbauer.de](https://derrcmodellbauer.de/)
- Platine: [ESP32-SBUS-Switch Platine](https://derrcmodellbauer.de/produkt/esp32-sbus-switch-platine/) und [SMD-Version (positiv oder negativ schaltend)](https://derrcmodellbauer.de/produkt/esp32-sbus-switch-platine-smd/)
- GitHub: [github.com/Ziege-One](https://github.com/Ziege-One), Vorlagen [ESP32-SBus-Switch](https://github.com/Ziege-One/ESP32-SBus-Switch) (mit Platine und Schaltplänen) und [CRSF_ESP32](https://github.com/Ziege-One/CRSF_ESP32)
- YouTube: [Der RC-Modellbauer](https://www.youtube.com/@DerRCModellbauer)

## Projektbeschreibungen (Word)

| Variante | Dokument |
|--------|---------|
| v2.xx | [`MultiSwitch_ESP32/ESP32-MultiSwitch-V2-Projektbeschreibung-v2.00.docx`](MultiSwitch_ESP32/ESP32-MultiSwitch-V2-Projektbeschreibung-v2.00.docx) |
| V3 | [`MultiSwitch_ESP32_V3/ESP32-MultiSwitch-V3-Projektbeschreibung-v3.12.docx`](MultiSwitch_ESP32_V3/ESP32-MultiSwitch-V3-Projektbeschreibung-v3.12.docx) |
| TXRX | [`MultiSwitch_ESP32_TXRX/ESP32-MultiSwitch-TXRX-Projektbeschreibung-v3.12.docx`](MultiSwitch_ESP32_TXRX/ESP32-MultiSwitch-TXRX-Projektbeschreibung-v3.12.docx) |

*v2.00 erstellt von: PiperPilot*
