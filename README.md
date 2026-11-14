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
| [`MultiSwitch_ESP32/`](MultiSwitch_ESP32/) | v2.06 | Linie v2.xx. Enthält die Projektbeschreibung als Word-Dokument. |
| [`MultiSwitch_ESP32_V3/`](MultiSwitch_ESP32_V3/) | v3.15 | Eigenständiger Zweig, abgezweigt von v2.02: MKan-Mehrfachadress-Erweiterung und Mehrfachkonfiguration pro Ausgang. |
| [`MultiSwitch_ESP32_TXRX/`](MultiSwitch_ESP32_TXRX/) | v3.15 | Gleicher Firmwarestand wie V3, aber mit der Pinbelegung der Platine **Multiswitch_TXRX** (ULN2803A; Nachfolger TBD62083APG passt direkt, siehe Ordner-README). |

Details, Änderungshistorie, Pin-Belegung und API stehen in der jeweiligen `README.md` im Projektordner.

Der gemeinsame Code (CRSF-Bibliothek, Lua-Menü, NVS, WLAN-Failsafe, Status-LED, OTA) liegt in jedem Sketch-Ordner als identische Kopie von **RC_Common** unter `src/RC_Common/` (dieselbe Basis nutzen RC-Multiswitch C3/S3 und die Soundmodule, siehe `src/RC_Common/README.md`). Die Arduino-IDE kompiliert den Unterordner automatisch mit. Benötigt werden zusätzlich die Bibliotheken *Bolder Flight Systems SBUS* 8.1.4 und *TinyGPSPlus* 1.0.3 (RC_Common enthält die GPS-Rolle der anderen Module; der Multiswitch nutzt sie nicht, die Datei wird aber mitkompiliert).

Die GitHub-CI prüft bei jedem Push die drei Kopien gegen `MANIFEST.sha256`, führt die Host-Tests von RC_Common aus und kompiliert alle drei Sketches (ESP32-Core 3.0.7).

## Firmware flashen

Es gibt drei Wege. Ein neues Modul wird einmal per USB geflasht (Weg 1 oder Weg 2), danach geht jedes Update bequem per WLAN (Weg 3).

| Weg | Braucht | Einstellungen im Modul |
|---|---|---|
| 1. Fertige Datei aus GitHub + Web-Flasher | nur Chrome oder Edge | werden **gelöscht** |
| 2. Arduino IDE | Arduino IDE 2.x | bleiben erhalten |
| 3. Update per WLAN | Handy oder PC mit WLAN | bleiben erhalten |

Zuerst die passende Variante wählen: **MultiSwitch_ESP32** (v2.xx), **MultiSwitch_ESP32_V3** (V3-Platine) oder **MultiSwitch_ESP32_TXRX** (Platine Multiswitch_TXRX). Die Firmware muss zur Platine passen, sonst stimmen die Pins nicht. `<Sketch>` steht unten für diesen Namen.

### Weg 1: Fertige Firmware aus GitHub (ohne Arduino IDE)

GitHub baut die Firmware bei jeder Änderung automatisch (Workflow „Build“).

1. Auf GitHub angemeldet sein (sonst gibt es keinen Download-Knopf).
2. In diesem Repo oben auf **Actions** klicken, links den Workflow **Build** wählen.
3. Den obersten Lauf mit grünem Haken öffnen (Branch `main`).
4. Ganz unten unter **Artifacts** das Artefakt mit dem Namen des Sketch-Ordners (**MultiSwitch_ESP32**, **MultiSwitch_ESP32_V3** oder **MultiSwitch_ESP32_TXRX**) herunterladen und die ZIP-Datei entpacken. Darin liegen u. a. `<Sketch>.ino.merged.bin` (Komplett-Image für USB) und `<Sketch>.ino.bin` (nur für Weg 3).
   Steht dort nichts (Artefakte werden nach 90 Tagen gelöscht): rechts oben **Run workflow** → **Run workflow** klicken, nach ca. 3 Minuten ist ein neuer Lauf fertig.
5. ESP32-Board mit einem USB-**Datenkabel** an den PC stecken. Windows braucht evtl. einen Treiber für den USB-Seriell-Chip auf dem Board (CP210x von Silicon Labs oder CH340); im Geräte-Manager erscheint dann ein neuer COM-Port.
6. In **Chrome** oder **Edge** die Seite https://espressif.github.io/esptool-js/ öffnen (Espressif-Web-Flasher, Firefox geht nicht).
7. **Connect** klicken und den COM-Port des Moduls wählen. Klappt das nicht: Baudrate oben auf `115200` stellen und das Modul mit der BOOT-Taste in den Flash-Modus bringen (siehe Weg 2, „Wenn das Hochladen nicht klappt“).
8. Unter **Flash Address** `0x0` eintragen, mit **Datei auswählen** die `.merged.bin` wählen, dann **Program** klicken.
9. Wenn unten „Leaving…“ steht, das USB-Kabel kurz abziehen und wieder einstecken. Fertig.

> **Achtung:** Das Komplett-Image überschreibt den ganzen Flash (4 MB). Alle Einstellungen (und die Sicherungen im Flash) sind danach weg. Vorher in der Weboberfläche die Einstellungen als Datei sichern (Karte **Konfiguration sichern/wiederherstellen** → „Als Datei exportieren“) und hinterher dort wieder importieren. Wer die Einstellungen behalten will, nimmt Weg 2 oder 3.

### Weg 2: Arduino IDE

**Einmalig einrichten**

1. Arduino IDE 2.x installieren (https://www.arduino.cc/en/software).
2. **Datei → Einstellungen → Zusätzliche Boardverwalter-URLs**: `https://espressif.github.io/arduino-esp32/package_esp32_index.json` eintragen.
3. **Werkzeuge → Board → Boardverwalter**: „esp32“ suchen, **esp32 von Espressif Systems** in Version **3.0.7** installieren.
4. **Werkzeuge → Bibliotheken verwalten**: **Bolder Flight Systems SBUS 8.1.4**, **TinyGPSPlus 1.0.3** suchen und in genau dieser Version installieren.
   Alles andere (z. B. `src/RC_Common/`) liegt schon im Sketch-Ordner.

**Flashen**

1. Repo herunterladen: oben auf GitHub **Code → Download ZIP**, entpacken (oder den vorhandenen Ordner nehmen).
2. In der Arduino IDE **Datei → Öffnen** und `<Sketch>/<Sketch>.ino` (z. B. `MultiSwitch_ESP32_V3/MultiSwitch_ESP32_V3.ino`) wählen.
3. Unter **Werkzeuge** einstellen:

   | Einstellung | Wert |
   |---|---|
   | Board | **ESP32 Dev Module** |
   | Flash Size | 4MB (32Mb) |
   | Partition Scheme | **Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)** |
   | Erase All Flash Before Sketch Upload | **Disabled** (sonst sind die Einstellungen weg) |

4. ESP32-Board mit einem USB-**Datenkabel** an den PC stecken. Windows braucht evtl. einen Treiber für den USB-Seriell-Chip auf dem Board (CP210x von Silicon Labs oder CH340); im Geräte-Manager erscheint dann ein neuer COM-Port. Dann unter **Werkzeuge → Port** den neuen COM-Port wählen.
5. Auf den Pfeil **Hochladen** klicken. Am Ende steht „Hard resetting via RTS pin…“.
6. Das Modul startet von selbst neu. Wenn nicht: USB-Kabel kurz abziehen und wieder einstecken.

Ein anderes OTA-Schema wie „Minimal SPIFFS“ geht auch, aber beim Wechsel des Partitionsschemas gehen die Einstellungen verloren. Am besten immer dasselbe Schema wie GitHub verwenden (oben).

**Wenn das Hochladen nicht klappt** („Failed to connect“ oder „No serial data received“): Noch einmal **Hochladen** und sobald unten „Connecting…“ erscheint, die Taste **BOOT** (manchmal „IO0“) auf dem ESP32-Board gedrückt halten, bis die Prozentzahlen laufen. Bei Boards ohne BOOT-Taste GPIO0 kurz mit GND verbinden.

### Weg 3: Update per WLAN (OTA)

1. Die Datei `<Sketch>.ino.bin` besorgen: aus GitHub (Weg 1, Schritte 1–4) oder in der Arduino IDE mit **Sketch → Kompilierte Binärdatei exportieren** (liegt dann im Sketch-Ordner unter `build/`). Nicht die `.merged.bin` nehmen.
2. WLAN des Moduls einschalten (ab Werk „WLAN dauerhaft aktiv“; sonst GPIO 13 beim Einschalten auf GND oder automatisch ohne RC-Signal). Name **MultiSwitch**, Passwort `123456789` (falls nicht geändert).
3. Mit Handy oder PC in dieses WLAN gehen und im Browser http://192.168.1.1 öffnen.
4. Reiter **WiFi**, Karte **Firmware-Update (OTA)**: die `.bin`-Datei auswählen und hochladen. Das Modul startet danach neu, die Einstellungen bleiben erhalten.

Schlägt das Update fehl, läuft die alte Firmware einfach weiter. Das Modul beim Update am besten über den Empfänger/BEC versorgen, nicht nur über einen schwachen USB-Anschluss.

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
