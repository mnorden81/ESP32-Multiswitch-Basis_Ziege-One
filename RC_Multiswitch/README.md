# RC-Multiswitch v5.03 (Platinen Basis und TXRX)

Ab v5.00 gibt es eine gemeinsame Firmware für alle Multiswitch-Platinen: die beiden Platinen dieses Repos (klassischer ESP32) sowie RC-Multiswitch C3 und S3. Der Sketch ist in allen drei Repos identisch (`FIRMWARE.sha256`, von der CI geprüft). Er ersetzt die Ordner `MultiSwitch_ESP32_V3` und `MultiSwitch_ESP32_TXRX` (v3.17). V2 (`MultiSwitch_ESP32/`) bleibt unverändert.

Aktuelle Version und alle Änderungen: [CHANGELOG.md](../CHANGELOG.md).

## Platine wählen

Basis und TXRX haben denselben Chip (ESP32), deshalb wählt man die Platine im Modul:

- Web-UI (http://192.168.1.1), Reiter **RC-System**, Karte **Platine**, oder
- Lua am Sender: **Global > Platine (Neustart)**.

Vorgabe ist **Basis**. Die Ausgangspins wechseln nach einem Neustart. Werkseinstellungen lassen die gewählte Platine unverändert. Ein Update von v3.17 behält alle Einstellungen, nur bei der TXRX-Platine muss einmal „TXRX“ gewählt und neu gestartet werden.

> **TXRX-Nutzer:** Bis zur Auswahl laufen die Ausgänge mit der Basis-Belegung. Angeschlossene Verbraucher können dabei auf falschen Pins schalten. Deshalb nach dem ersten Flashen zuerst die Platine wählen und neu starten, dann erst Verbraucher anschließen.

## Arduino IDE

Board **ESP32 Dev Module**, Partition Scheme **Default 4MB with spiffs**. Datei → Öffnen: `RC_Multiswitch/RC_Multiswitch.ino`. Bibliotheken und alle Schritte: [Haupt-README](../README.md#firmware-flashen).

## Pin-Belegung

| Funktion | Basis (ESP32 D1 Mini) | TXRX (ULN2803A / TBD62083) |
|---|---|---|
| CRSF RX / SBUS | GPIO 16 | GPIO 16 |
| CRSF TX | GPIO 17 | GPIO 17 |
| Ausgang 1 | 18 | 27 |
| Ausgang 2 | 19 | 25 |
| Ausgang 3 | 21 | 5 (Strapping-Pin) |
| Ausgang 4 | 22 | 23 |
| Ausgang 5 | 23 | 19 |
| Ausgang 6 | 25 | 18 |
| Ausgang 7 | 26 | 26 |
| Ausgang 8 | 27 | 33 |
| Status-LED | GPIO 2 | GPIO 2 |
| WLAN beim Start | GPIO 13 auf GND | GPIO 13 auf GND |
| BOOT-Taster | keiner (nur Reset) | keiner (nur Reset) |

- UART B/C (Modulkette, S.Port, GPS) sind auf diesen Platinen nicht herausgeführt und in Web-UI und Lua ausgeblendet.
- TXRX: GPIO 5 (Ausgang 3) ist ein Strapping-Pin, beim ersten Einschalten prüfen, ob das Modul sicher startet. Auf J4 liegen Ausgang 5–8 nicht in Reihenfolge: O5 → Pin 13, O6 → 11, O7 → 9, O8 → 15 (ungerade Pins = Ausgang, gerade = GND).
- TXRX: Der ULN2803A ist abgekündigt. Der **TBD62083APG** passt direkt in denselben Sockel und wird weniger warm. An der Firmware ändert sich nichts.

## RC-System

| Wert | System |
|---|---|
| 0 | FrSky (SBUS) |
| 1 | FlySky (SBUS) |
| 2 | ELRS normiert (SBUS) |
| 3 | HoTT (SBUS) |
| 4 | CRSF (ELRS/TBS), Werkseinstellung bei neuen Modulen |

| 5 | CRSF + Funk senden (Zentrale, ab v5.01) |
| 6 | CRSF aus Funk (Teilnehmer, ab v5.01) |

Lua-Menü und Telemetrie gibt es nur bei CRSF (4–6).

## Funk (ab v5.01)

Module ohne Kabel zum Empfänger bekommen CRSF per Funk (ESP-NOW). Das Modul am Empfänger läuft als **Zentrale** (RC-System 5), die anderen als **Teilnehmer** (RC-System 6, GPIO16/17 bleiben dann frei). Kanäle, Lua-Menü und Telemetrie laufen über Funk. Fahrzeug-Kennung und Funkkanal müssen überall gleich sein: Web-UI Reiter RC-System, Karte „Funk“, oder Lua-Ordner „Funk“. Der Reiter „Funk“ zeigt die Verbindungsqualität. Ohne Funk-Daten gehen nach 2 s alle Ausgänge aus. Mit Funk läuft das WLAN auf dem Funkkanal.

## Web-Interface

IP `192.168.1.1`, SSID `MultiSwitch`, Passwort `123456789` (Werkseinstellung). Die Projektbeschreibungen von v3.12 liegen jetzt in [docs/](../docs/).
