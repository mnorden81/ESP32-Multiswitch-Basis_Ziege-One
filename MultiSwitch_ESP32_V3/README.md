# ESP32-MultiSwitch V3

RC-gesteuerter 8-Kanal-Schalter für ESP32 mit Web-Interface.

*v2.00 erstellt von: PiperPilot*

**Eigenständiges Projekt**, abgezweigt von `ESP32-MultiSwitch v2.02`
(Ordner [`MultiSwitch_ESP32/`](../MultiSwitch_ESP32/) in diesem Repo).
v2.xx bleibt dort unverändert bestehen und wird von dieser Version nicht
berührt – beide Projekte sind unabhängig voneinander baubar.

Aktuelle Version und alle Änderungen: [CHANGELOG.md](../CHANGELOG.md).

## Flashen und Partitionsschema

Flash-Anleitung (fertige Firmware aus GitHub, Arduino IDE, Update per WLAN) und das passende Partitionsschema (**Default 4MB with spiffs**) stehen im [Haupt-README](../README.md#firmware-flashen). Für das Update per WLAN muss einmal per USB ein Schema mit zwei App-Partitionen (OTA) geflasht sein.

## Praxis-Hinweise für das OTA-Update (Erfahrungen aus dem Soundmodul-Projekt)

- **`.bin`-Datei erzeugen:** Arduino-IDE → Sketch → **Kompilierte Programmdatei exportieren** (Strg+Alt+S). Das kompiliert wie beim normalen Hochladen, schreibt das Ergebnis aber zusätzlich als Datei in einen `build/...`-Unterordner des Sketch-Ordners, statt es nur in einem temporären Cache abzulegen.
- **Richtige Datei:** benötigt wird ausschließlich `MultiSwitch_ESP32.ino.bin` – die Dateien `...bootloader.bin` und `...partitions.bin` im selben Ordner werden nicht gebraucht, die betreffen nur das allererste USB-Flashen eines neuen Moduls.
- **Speicherbedarf prüfen:** Am Ende des Kompiliervorgangs zeigt die IDE „Der Sketch verwendet ... Bytes (NN %) des Programmspeicherplatzes". Solange der Wert unter 100 % liegt, passt die Firmware in eine OTA-Partition.
- ⚠️ **Stromversorgung während des Updates:** Bestätigte Fehlerursache aus dem Soundmodul-Projekt – bei Versorgung nur über USB kann der Spannungseinbruch beim Flash-Schreiben einen Reset auslösen, der den Upload mittendrin abbricht (die alte Firmware bleibt dabei unangetastet, ein erneuter Versuch ist jederzeit möglich). Update deshalb möglichst bei über ESC/Akku bestromtem Modul durchführen, nicht nur über USB.
- **Nach dem Update:** Das Modul startet automatisch neu, die WLAN-Verbindung bricht dabei kurz ab – das ist normal. Anschließend im WLAN neu verbinden und die Funktion prüfen.
- **Falls das Webinterface danach nicht mehr erreichbar ist:** Modul kurz von der Spannungsversorgung trennen und neu starten. Sollte die neue Firmware fehlerhaft sein, hilft in letzter Konsequenz ein reguläres Flashen per USB mit einer bekannt funktionierenden Version.

## Pin-Belegung

| GPIO | Funktion |
|------|----------|
| 13 | WiFi-Pin (LOW beim Booten = AP aktiv; AP kann ab v2.00 auch per Web oder Auto-Failsafe zur Laufzeit gestartet werden) |
| 16 | SBUS RX / CRSF RX |
| 17 | CRSF TX |
| 18–27 | Ausgänge 1–8 |
| 2 | Status-LED |

## Bibliotheken

- Bolder Flight Systems SBUS 8.1.4
- TinyGPSPlus 1.0.3 (wird von RC_Common mitkompiliert)
- CRSF_ESP32 (https://github.com/Ziege-One/CRSF_ESP32)

## RC-System Werte

| Wert | System |
|------|--------|
| 0 | FrSky (SBUS) |
| 1 | FlySky (SBUS) |
| 2 | ELRS normiert (SBUS) |
| 3 | HoTT (SBUS) |
| 4 | CRSF (ELRS/TBS) |

## Web-Interface

IP: `192.168.1.1` (Standard)
SSID: `MultiSwitch` / Passwort: `123456789`

### API-Endpunkte

| Methode | Pfad | Beschreibung |
|---------|------|--------------|
| GET | `/api/status` | Aktueller Status (Ausgänge, Kanäle, MWprop, CRSF-Diagnose, AP-Status) |
| GET/POST | `/api/config` | Konfiguration lesen/schreiben (inkl. WLAN-Auto-Failsafe) |
| POST | `/api/switch` | Ausgang manuell schalten / freigeben |
| POST | `/api/output` | Ausgangs-Konfiguration (Kanal, PWM, Blink-Modus, Name) – *seit v3.10:* je Ausgang bis zu 3 Konfigurationen (`kanal/pwm/mode`, `kanal2/pwm2/mode2`, `kanal3/pwm3/mode3`) |
| POST | `/api/wifi` | *(NEU v2.00)* Access Point sofort ein-/ausschalten, ohne Neustart |
| POST | `/api/save` | *(NEU v2.00)* Konfiguration sofort auf Flash schreiben, ohne Neustart |
| POST | `/api/otaupdate` | *(NEU v2.00)* Firmware-Update per WLAN hochladen (multipart/form-data, `.bin`-Datei) – startet das Modul bei Erfolg neu |
| POST | `/api/reset` | Werkseinstellungen |
| POST | `/api/restart` | Neustart (nach Sichern) |
