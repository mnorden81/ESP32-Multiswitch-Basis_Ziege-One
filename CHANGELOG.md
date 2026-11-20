# Versionsgeschichte ESP32-MultiSwitch

Dieses Repo enthält zwei Sketche: **V2** (`MultiSwitch_ESP32/`) und ab v5.00 **RC_Multiswitch** (`RC_Multiswitch/`) für die Platinen Basis und TXRX. RC_Multiswitch ist dieselbe Firmware wie in RC-Multiswitch-ESP32-C3/-S3 und hat dieselben Versionsnummern (alle Versionen ab v4.00 siehe dort). Bis v3.17 gab es getrennte Ordner `MultiSwitch_ESP32_V3/` und `MultiSwitch_ESP32_TXRX/` (Stand in der Git-Historie). Die Version wird bei jeder Änderung erhöht, weil sie als CRSF „Parameter Version Number“ an den Sender geht. Neueste Version oben.

Hardware und Grundlage der Firmware stammen von Der RC-Modellbauer (Ziege-One), siehe README.

## RC_Multiswitch (Basis und TXRX, ab v5.00)

### v5.05 (RC_Common 1.5.2)
- Lua-Menü kürzer: statt acht Ordnern „Ausgang 1“ bis „Ausgang 8“ gibt es einen Ordner „Ausgaenge“. Oben wählt „Ausgang Nr“ (1–8) den Ausgang, darunter stehen Name, Quelle, Nr, Blink, AN, AUS, PWM Modus, Festwert und Test für diesen Ausgang. 34 statt 95 Lua-Felder, das Menü lädt deutlich schneller.
- Neue Feldnummern (Platine jetzt 26, Funk 27–34). Einstellungen bleiben erhalten, der Sender lädt das Menü nach dem Update neu. Web-UI unverändert.

### v5.04 (RC_Common 1.5.2)
- Hardware-Test 19.11.2026 bestanden: Multiswitch S3 als Funk-Teilnehmer mit ESP32_ESP-NOW_CRSF-Link v0.2.1 am Empfänger. Kanäle, Lua-Menü und WM-Widget bei jeder Modul-Adresse laufen über Funk.
- Diagnose WM-Adresse: Reiter „Debug“ zeigt „WM-Befehle empfangen“ und den letzten Befehl (Ziel, Quelle, Code, Befehl, Adresse im Befehl, eigene Adresse, übernommen ja/nein). Damit ist sichtbar, welche Adresse das Widget wirklich schickt. Schaltverhalten unverändert.

### v5.03 (RC_Common 1.5.2)
- Fix: WM-Befehle vom Widget schalten wieder bei jeder Modul-Adresse. Seit v4.00 wurden CRSF-Kommandos nur angenommen, wenn sie an die eigene CRSF-Adresse (0xC0 + Modul-Adresse) gingen; das Widget schickt an eine feste Zieladresse, deshalb ging nur Adresse 0. Jetzt wie in der Ziege-One-Firmware: jedes Kommando wird angenommen, gefiltert nach WM-Code und Modul-Adresse im Befehl.
- Hardware-Test Funk-Teilnehmer mit ESP32_ESP-NOW_CRSF-Link am Empfänger: Kanäle, WM und Lua-Antworten laufen über Funk (fehlendes Lua lag an einem Wackelkontakt der Link-Ader zum Empfänger).

## v5.02 (RC_Common 1.5.1)
- Fehlersuche Funk-Teilnehmer: Lua-Menü erscheint nicht, obwohl Pings ankommen. Reiter „Funk“ zeigt jetzt Gesamtzähler (Antworten in den Funk, gesendet, geschriebene Bytes, verworfen ohne Gegenstelle / Rückkanal aus / Rahmenfehler), Reiter „Debug“ zusätzlich „DEVICE_INFO gesendet“.
- Fix: Eine in der Web-UI geänderte Modul-Adresse gilt sofort auch als CRSF-Geräteadresse (Lua, WM-Befehle an 0xC0 + Adresse) und im Funk-Ping. Vorher erst nach einem Neustart, über Lua ging es schon sofort.

## v5.01 (RC_Common 1.5.0)
- Neu: **Funk** (ESP-NOW, aus der Funk-Brücke v0.1.9). Module ohne Kabel zum Empfänger bekommen CRSF per Funk. RC-System 5 = „CRSF + Funk senden (Zentrale)“: das Modul am Empfänger funkt alles vom Kabel weiter. RC-System 6 = „CRSF aus Funk (Teilnehmer)“: Kanäle, Lua und Telemetrie laufen über Funk, UART A bleibt frei. Auf Basis/TXRX wie auf C3/S3. Pro Fahrzeug eine Zentrale und bis zu 8 Teilnehmer, eine Funk-Brücke kann an jeder Seite stehen. Noch nicht auf Hardware getestet.
- Einstellungen: Fahrzeug-Kennung (1–255), Funkkanal (1–13), Sendeleistung (2–20 dBm, Vorgabe 20), bei der Zentrale „Kanäle alle“ (Vorgabe 10 ms), beim Teilnehmer „Rückkanal“ (Lua/Telemetrie, Vorgabe an). Web-UI Reiter RC-System, Karte „Funk“, oder Lua-Ordner „Funk“ (Felder 88–95). Wirken nach Neustart. NVS `fkid fkch fkpwr fkrate fkback`.
- Web-UI: neuer Reiter „Funk“ mit Ampel, Gegenstellen (Rolle, CRSF-Adresse, Pakete/s, Verlust, RSSI, Aussetzer) und Zählern, Zeile in der Übersicht. `/api/funk`, `/api/funk/reset`.
- Failsafe wie bei abgezogenem Kabel: ohne Funk-Daten nach 2 s alle Ausgänge aus.
- Zweite Zentrale mit derselben Kennung: gelber Doppelblitz. Gegenstelle mit derselben CRSF-Adresse: blauer Doppelblitz.
- Mit Funk läuft das WLAN auf dem Funkkanal (ein Funkteil für beides).

### v5.00 (RC_Common 1.4.0)
- Eine Firmware für alle Multiswitch-Platinen (Basis, TXRX, C3, S3), ersetzt `MultiSwitch_ESP32_V3/` und `MultiSwitch_ESP32_TXRX/`. Grundlage ist RC-Multiswitch v4.11 (abgezweigt von V3 v3.12).
- Platine Basis oder TXRX wählbar: Web-UI Reiter RC-System, Karte „Platine“, oder Lua Global > „Platine (Neustart)“ (Feld 87). Vorgabe Basis, wirkt nach Neustart.
- Gespeicherte Einstellungen aus v3.17 bleiben erhalten (NVS `msw`). TXRX-Platine: nach dem Update einmal „TXRX“ wählen und neu starten.
- Neu gegenüber v3.17 (aus v4.00–v4.11): Status-LED mit dem Farbschema aller Module (hier einfarbig an GPIO2), neues Lua-Menü (Ordner mit Wert, Felder ohne Wirkung ausgeblendet), RC_Common 1.4.0, Werkseinstellung RC-System CRSF bei neuen Modulen.
- Wie bisher: CRSF/SBUS an GPIO16/17, GPIO13 beim Start auf GND = WLAN an. Kein BOOT-Taster (D1 Mini hat nur Reset) und keine UART B/C (keine freien Pins).
- Projektbeschreibungen V3/TXRX v3.12 nach `docs/` verschoben.

## V3 und TXRX (bis v3.17, Ordner entfernt)

### v3.17 (RC_Common 1.3.0)
- Fix: lange Lua-Auswahlfelder (z. B. Ausgang-Quelle mit MKan) kommen Chunk für Chunk, das ELRS-Lua zeigt sie vollständig.
- Ordnernamen zeigen den aktuellen Wert, Felder ohne Wirkung werden ausgeblendet (Feld-IDs bleiben).
- Fix: Lua-Feld „Version“ zeigt die aktuelle Version.

### v3.16
- Lua: „PWM Festwert“ bleibt bei MWprop ein gesperrtes Zahlenfeld statt Info-Feld.

### v3.15 (RC_Common 1.2.0)
- Neu: Einstellungs-Backup im Flash (Tab WLAN), automatisch vor Import und Firmware-Update. Braucht ein Partitionsschema mit SPIFFS.

### v3.14 (RC_Common 1.0.0)
- Gemeinsame Basis RC_Common unter `src/RC_Common/` (CRSF-Bibliothek 1.1.0, Lua-Menü als Tabelle, NVS, WLAN-Failsafe, Status-LED, OTA). `src/CRSF_ESP32/` entfällt.
- Fix: SBUS-Zeitlimit 10 s, danach Ausgänge aus.
- Status-LED (GPIO 2) mit den gemeinsamen Mustern.
- NVS: gemeinsame Schlüssel (`wip madr wfauto wftmo wfalways`), alte Werte werden einmalig übernommen.

### v3.13
- CRSF-Code als gemeinsame Bibliothek `src/CRSF_ESP32/` (v1.0.0).
- Fixes laut CRSF-Spezifikation: Parser verliert keine Frames mehr, Link-Statistik, Command-CRC (0xBA) und FOLDER-Eintrag korrigiert.

### v3.12
- Fix: CRSF „Parameter Version Number“ im DEVICE_INFO enthält die Firmware-Version (vorher fest 0), übernommen aus v2.03.

### v3.11 (nur TXRX)
- Neue Ausgangs-Pins für die Platine Multiswitch_TXRX: `{27,25,5,23,19,18,26,33}` statt `{18,19,21,22,23,25,26,27}`. Sonst identisch zu v3.10.

### v3.10
- Bis zu 3 Konfigurationen (Kanal-Quelle → Blink/PWM) je Ausgang, Priorität Konfig 1 > 2 > 3. Konfig 2/3 nur in der Web-UI.
- NVS: 6 neue Schlüssel je Ausgang (`ak2_x/pw2_x/mo2_x/ak3_x/pw3_x/mo3_x`), fehlend = ungenutzt.

### v3.00
- Eigenständiger Zweig, abgezweigt von V2 v2.02.
- MKan-Gruppen: 2 zusätzliche 8-Bit-Kanalquellen MKan1/MKan2 (Ausgang_Kanal 80–95). Bei CRSF über 2 Gruppenadressen, bei SBUS über 2 feste SBUS-Kanäle.
- Web-UI: Karte „MKan-Gruppen“, Lua: Ausgang-Quelle um MKan1/MKan2 erweitert.
- NVS: neue Schlüssel `gadr0/1`, `gch0/1`, `gmod0/1` (Vorgabe: alle Gruppen aus).

## V2

### v2.08 (RC_Common 1.3.0)
- Fix: lange Lua-Auswahlfelder kommen vollständig an (Chunk-Fix). Ordner zeigen ihren Wert, Felder ohne Wirkung werden ausgeblendet.
- Fix: Lua-Feld „Version“ zeigt die aktuelle Version.

### v2.07
- Lua: „PWM Festwert“ bleibt bei MWprop ein gesperrtes Zahlenfeld statt Info-Feld.

### v2.06 (RC_Common 1.2.0)
- Neu: Einstellungs-Backup im Flash, automatisch vor Import und Firmware-Update.

### v2.05 (RC_Common 1.0.0)
- Gemeinsame Basis RC_Common (wie V3 v3.14). Fix: SBUS-Zeitlimit 10 s. Status-LED mit gemeinsamen Mustern, gemeinsame NVS-Schlüssel.

### v2.04
- CRSF-Code als gemeinsame Bibliothek `src/CRSF_ESP32/` mit den Fixes laut Spezifikation (wie V3 v3.13).

### v2.03
- Fix: CRSF „Parameter Version Number“ im DEVICE_INFO enthält die Firmware-Version (übernommen aus ESP32-RC-Sound v7.18).

### v2.02
- Neu: „WLAN dauerhaft aktiv“ (Vorgabe an, wirkt ab dem nächsten Neustart).
- Neu: Lua-Ordner „WLAN“ (Felder 77–80: Auto-Failsafe, Timeout, Dauerhaft an), neu angehängt, Felder 0–76 unverändert.

### v2.01
- „Auto bei Signalverlust“ (WLAN-Auto-Failsafe) ab Werk an.

### v2.00
- Übernahmen aus dem Soundmodul ESP32-RC-Sound v7: CRSF-Diagnosezähler im Debug-Tab, BUS_OK an gültige Frames gekoppelt, CRC-Fehler gezählt statt einzeln ausgegeben, DEVICE_INFO-Prüfung, Zieladressen-Prüfung bei Commands.
- Neu: Konfiguration als JSON exportieren/importieren, WLAN-Auto-Failsafe mit „WLAN jetzt ein/aus“, Firmware-Update per WLAN (OTA).
- Fix nach erstem Test: Absturz mit Bootloop, wenn der AP beim Booten aus war (`WiFi.mode(WIFI_AP)` in `webui_init()`).

### v1.42
- Mehrere Module am selben Empfänger: CRSF-Geräteadresse 0xC0 + Moduladresse statt fest 0xC8, Antwort auf Ping im eigenen Zeitschlitz (übernommen aus Soundmodul v1.24). Braucht ExpressLRS ab 4.x.

### v1.41
- CRSF-Fixes (CRC im Parser, `init_crsf()` mit übergebenem Port, `send_command()`), robustere Web-API, NVS-Schreiben gebündelt.

### v0.14
- MWprop aus ESP32-RC-Sound: PWM einzelner Ausgänge über einen CRSF-Kanal.
- NVS nur bei Änderung speichern, CRSF-Einkanal-Timeout 2000 ms.
