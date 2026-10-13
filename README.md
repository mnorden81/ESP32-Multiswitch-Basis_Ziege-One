# ESP32-MultiSwitch

RC-gesteuerter 8-Kanal-Schalter für ESP32 mit Web-Interface (CRSF/SBUS).

Dieses Repository enthält drei eigenständige, unabhängig voneinander baubare Projekte. Jedes liegt in einem eigenen Sketch-Ordner, dessen Name zur `.ino`-Datei passt (Arduino IDE).

| Ordner | Version | Beschreibung |
|--------|---------|--------------|
| [`MultiSwitch_ESP32/`](MultiSwitch_ESP32/) | v2.04 | Linie v2.xx. Enthält die Projektbeschreibung als Word-Dokument. |
| [`MultiSwitch_ESP32_V3/`](MultiSwitch_ESP32_V3/) | v3.13 | Eigenständiger Zweig, abgezweigt von v2.02: MKan-Mehrfachadress-Erweiterung und Mehrfachkonfiguration pro Ausgang. |
| [`MultiSwitch_ESP32_TXRX/`](MultiSwitch_ESP32_TXRX/) | v3.13 | Gleicher Firmwarestand wie V3, aber mit der Pinbelegung der Platine **Multiswitch_TXRX** (ULN2803A). |

Details, Änderungshistorie, Pin-Belegung und API stehen in der jeweiligen `README.md` im Projektordner.

Der CRSF-Code liegt in jedem Sketch-Ordner als identische Kopie der gemeinsamen Bibliothek unter `src/CRSF_ESP32/` (dieselbe Bibliothek nutzt das Soundmodul, siehe `src/CRSF_ESP32/README.md`). Die Arduino-IDE kompiliert den Unterordner automatisch mit. Benötigt wird zusätzlich nur die Bibliothek *Bolder Flight Systems SBUS*.

Die GitHub-CI prüft bei jedem Push, dass die drei Kopien gleich sind, und kompiliert alle drei Sketches (ESP32-Core 3.0.7).

*v2.00 erstellt von: PiperPilot*
