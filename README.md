# ESP32-MultiSwitch

RC-gesteuerter 8-Kanal-Schalter für ESP32 mit Web-Interface (CRSF/SBUS).

Dieses Repository enthält drei eigenständige, unabhängig voneinander baubare Projekte. Jedes liegt in einem eigenen Sketch-Ordner, dessen Name zur `.ino`-Datei passt (Arduino IDE).

| Ordner | Version | Beschreibung |
|--------|---------|--------------|
| [`MultiSwitch_ESP32/`](MultiSwitch_ESP32/) | v2.03 | Linie v2.xx. Enthält die Projektbeschreibung als Word-Dokument. |
| [`MultiSwitch_ESP32_V3/`](MultiSwitch_ESP32_V3/) | v3.12 | Eigenständiger Zweig, abgezweigt von v2.02: MKan-Mehrfachadress-Erweiterung und Mehrfachkonfiguration pro Ausgang. |
| [`MultiSwitch_ESP32_TXRX/`](MultiSwitch_ESP32_TXRX/) | v3.12 | Gleicher Firmwarestand wie V3, aber mit der Pinbelegung der Platine **Multiswitch_TXRX** (ULN2803A). |

Details, Änderungshistorie, Pin-Belegung und API stehen in der jeweiligen `README.md` im Projektordner.

## Hardware und Herkunft

Die Hardware dieses Projekts stammt von **Der RC-Modellbauer** (Frank Verfürth, auf GitHub **Ziege-One**). Die Firmware basiert auf seinem Projekt *ESP32-SBus-Switch* 0.6, die CRSF-Anbindung auf seiner Bibliothek *CRSF_ESP32*.

- Webseite und Shop: [derrcmodellbauer.de](https://derrcmodellbauer.de/)
- Platine: [ESP32-SBUS-Switch Platine](https://derrcmodellbauer.de/produkt/esp32-sbus-switch-platine/) und [SMD-Version (positiv oder negativ schaltend)](https://derrcmodellbauer.de/produkt/esp32-sbus-switch-platine-smd/)
- GitHub: [github.com/Ziege-One](https://github.com/Ziege-One), Vorlagen [ESP32-SBus-Switch](https://github.com/Ziege-One/ESP32-SBus-Switch) (mit Platine und Schaltplänen) und [CRSF_ESP32](https://github.com/Ziege-One/CRSF_ESP32)
- YouTube: [Der RC-Modellbauer](https://www.youtube.com/@DerRCModellbauer)

*v2.00 erstellt von: PiperPilot*
