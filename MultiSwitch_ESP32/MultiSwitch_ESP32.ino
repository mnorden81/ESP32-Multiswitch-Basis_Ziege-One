/*
    ESP32-MultiSwitch  v2.05
   v2.04: CRSF-Code als gemeinsame Bibliothek, siehe AENDERUNGEN v2.04.
   v2.00 erstellt von: PiperPilot
   Basiert auf: ESP32-SBus-Switch 0.6   (Ziege-One / Der RC-Modellbauer)
   CRSF-Integration: ESP32-RC-Sound 0.43 (Ziege-One / Der RC-Modellbauer)

 ÄNDERUNGEN v2.05 (gemeinsame Basis RC_Common 1.1.0):
   - Gemeinsamer Code aller ESP32-Module liegt jetzt in src/RC_Common/
     (identische Kopie in jedem Projekt, siehe src/RC_Common/README.md):
     CRSF-Bibliothek 1.1.0, Lua-Menue als Tabelle, NVS-Schluessel,
     WLAN-Failsafe, Status-LED, OTA-Upload, JSON-Helfer. Der Ordner
     src/CRSF_ESP32/ entfaellt.
   - FIX: SBUS-Zeitlimit (wie RC-Multiswitch v4.03). Kommen 10 s lang gar
     keine SBUS-Frames (Stecker ab, Empfaenger ohne Strom), gilt das
     Signal als weg: BUS_OK wird false, die Ausgaenge gehen aus und das
     WLAN-Auto-Failsafe kann greifen. Vorher blieb ohne Frames der letzte
     Zustand stehen. Siehe checkSbusTimeout().
   - Status-LED (GPIO 2) mit den gemeinsamen Mustern: aus = kein Signal,
     langsam 500/500 ms = OK, schnell 100/100 ms = Frames ohne RC-Link bzw.
     SBUS-Failsafe.
   - Lua-Menue als Tabelle (buildLuaMenu()), gleiche Feld-IDs 0-80.
   - NVS: gemeinsame Schluessel wip, madr, wfauto, wftmo, wfalways
     (RC_Common settings_common.h). Beim ersten Start werden die alten
     Werte (ip, mod_adr, wauto, watout, walways) einmalig uebernommen; die
     alten Schluessel bleiben fuer ein Zurueckspielen stehen.

 ÄNDERUNGEN v2.04 (gemeinsame CRSF-Bibliothek):
   - CRSF-Code als gemeinsame Bibliothek src/CRSF_ESP32/ (CRSF_ESP32
     v1.0.0, identisch mit Soundmodul und den anderen Varianten).
   - Fix: Empfangs-Parser verliert keine Frames mehr, die ueber zwei
     loop()-Durchlaeufe verteilt ankommen (vorher verloren und als
     CRC-Fehler gezaehlt). Link-Statistik, Command-CRC und FOLDER-Laenge
     laut CRSF-Spezifikation korrigiert.

 /////Projektbeschreibung////
 RC-gesteuerter 8-Kanal-Schalter/PWM-Ausgabemodul für ESP32. Empfängt
 Schaltbefehle über CRSF (ExpressLRS/TBS CrossFire, bidirektional) oder
 über SBUS-basierte RC-Systeme (FrSky/FlySky/ELRS normiert/HoTT) und
 steuert darüber acht digitale bzw. PWM-fähige Ausgänge (Licht, Blinker,
 Servos, Relais o.ä.). Die gesamte Konfiguration erfolgt über eine im
 Modul integrierte Weboberfläche (WLAN-Access-Point) - es wird kein
 SD-Karten-Slot benötigt oder verwendet. Ausführliche Beschreibung siehe
 ESP32-MultiSwitch-V2-Projektbeschreibung-v2.00.docx / README.md.

 /////Pin Belegung////
 GPIO 13: WiFi Pin (LOW = AP beim Booten aktiv)
 GPIO 16: SBUS RX  /  CRSF RX
 GPIO 17: CRSF TX  (nur bei RC_System == 4)
 GPIO 18: Ausgang 1
 GPIO 19: Ausgang 2
 GPIO 21: Ausgang 3
 GPIO 22: Ausgang 4
 GPIO 23: Ausgang 5
 GPIO 25: Ausgang 6
 GPIO 26: Ausgang 7
 GPIO 27: Ausgang 8
 GPIO  2: Status LED (AUS = kein Signal, Blinken = verbunden)

 RC-System Werte:
   0 = FrSky          (SBUS)
   1 = FlySky         (SBUS)
   2 = ELRS normiert  (SBUS)
   3 = HoTT           (SBUS)
   4 = CRSF           (ExpressLRS / TBS CrossFire, bidirektional)

 ÄNDERUNGEN v2.00 (übertragen aus dem Soundmodul-Projekt, ESP32-RC-Sound v7):
   - CRSF-Diagnose-Zähler (rohe Bytes, gültige Frames, CRC-Fehler, PING/
     PARAM-READ/WRITE) in crsf_esp32.h/.cpp, sichtbar im Debug-Tab der
     Weboberfläche - macht "geht nicht" ohne Seriell-Monitor sichtbar.
   - BUS_OK bei CRSF jetzt an den Zähler für gültige Frames gekoppelt statt
     an "channel_output[0] > 0" (ein Kanalwert von 0 ist legitim und kein
     zuverlässiger Nachweis für einen frischen Frame).
   - CRC-Fehler werden gezählt statt bei jedem einzelnen Fehler sofort per
     Serial.println() ausgegeben - verhindert, dass anhaltendes Funkrauschen
     den Loop ausbremst; Sammel-Meldung höchstens alle 2s.
   - updateDevice_Info() gegen zu kurze/kaputte DEVICE_INFO-Frames
     abgesichert (signed-Rechnung + Mindestlängen-Check).
   - CRSF_FRAMETYPE_COMMAND prüft jetzt die Zieladresse wie PING/PARAMETER_
     READ/WRITE (mit Broadcast-Ausnahme fürs WM-Protokoll).
   - Konfiguration exportieren/importieren als JSON-Datei über den Browser
     (kein SD-Karten-Slot vorhanden - rein Web/WLAN-basiert), siehe webui.h.
   - WLAN-Steuerung: optionales Auto-Failsafe schaltet den Access Point
     automatisch ein, wenn längere Zeit kein gültiges RC-Signal anliegt
     (z.B. Sender aus/außer Reichweite) - Default AN seit v2.01, siehe
     wifiFailsafeCheck() unten. Ergänzend manueller "WLAN jetzt ein/aus"-
     Schalter (Web-UI), der ohne Neustart sofort wirkt, siehe webui.h.
   - Firmware-Update per WLAN (OTA): eine mit der Arduino-IDE gebaute
     .bin-Datei wird direkt im Browser hochgeladen und landet im gerade
     inaktiven OTA-Flash-Segment - Ausbauen/USB-Flashen ist für spätere
     Updates damit nicht mehr nötig. Schlägt der Upload fehl oder wird er
     abgebrochen, bootet das Modul unverändert mit der bisherigen
     Firmware weiter (kein Bricking-Risiko), siehe
     handleOtaUpdateData()/handleOtaUpdate() in webui.h. Voraussetzung:
     einmalig ein OTA-fähiges Partitionsschema per USB flashen, siehe
     README. WICHTIG (Erfahrung aus dem Soundmodul-Projekt übernommen):
     Update möglichst bei über ESC/Akku bestromtem Modul durchführen,
     nicht nur über USB - manche USB-Anschlüsse liefern beim
     Flash-Schreiben nicht genug Strom, ein dadurch ausgelöster Reset
     bricht den Upload mittendrin ab.
   - eeprom_esp32.h/hal_esp32.h/output_ctrl.h sind weiterhin NICHT
     eingebunden (totes Code-Fragment eines nicht gemergten Refactorings,
     jetzt oben in den Dateien selbst als solches gekennzeichnet).
   - Fix (nach erstem Test): webui_init() rief webServer.begin() bisher
     unabhängig vom AP-Status auf, ohne den WiFi-Treiber vorher zu
     initialisieren. War der AP beim Booten nicht aktiv, fehlten dessen
     interne FreeRTOS-Queues und es kam zum sofortigen Absturz
     (assert failed: xQueueSemaphoreTake) mit Bootloop. Behoben durch
     WiFi.mode(WIFI_AP) am Anfang von webui_init() (siehe webui.h).

 ÄNDERUNGEN v0.14 (Verbesserungen):
   - MWprop-Unterstützung aus ESP32-RC-Sound übernommen:
     Lautstärke/PWM-Steuerung einzelner Ausgänge über CRSF-Kanal möglich.
   - wm_prop_value[8] Speicher für MWprop-Kanalwerte (duty 0–255).
   - nvsSave() jetzt nur noch bei tatsächlicher Änderung (nvsDirty-Flag
     war schon vorhanden, wird nun auch in webui korrekt gesetzt).
   - CRSF-Einkanal-Timeout erhöht auf 2000 ms (war 1000 ms) –
     robuster bei kurzen Signalunterbrechungen.
   - Output(): pwm_wert >= 300 Mapping auf benannte Konstante ausgelagert.
   - Konsistente Verwendung von static constexpr überall.
   - README.md aktualisiert.
*/

// ======== Bibliotheken ==========================================
/*
  Bolder Flight Systems SBUS  8.1.4   (für RC_System 0-3)
  RC_Common 1.0.0 im Sketch-Ordner src/RC_Common/ (v2.05), enthaelt
    CRSF_ESP32 1.1.0, Basis https://github.com/Ziege-One/CRSF_ESP32
*/

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include "sbus.h"
#include "blink_presets.h"
// v2.05: gemeinsame Basis aller ESP32-Module
#include "src/RC_Common/rc_common.h"
#include "src/RC_Common/CRSF_ESP32/crsf_esp32.h"
#include "src/RC_Common/lua_menu.h"
#include "src/RC_Common/settings_common.h"
#include "src/RC_Common/status_led.h"
#include "src/RC_Common/wifi_failsafe.h"
#include "src/RC_Common/web_common.h"

constexpr uint16_t Version = 205; // 2.05 - gemeinsame Basis RC_Common

// ======== SBUS-Schwellen (benannte Konstanten) ==================
static constexpr uint16_t SBUS_LOW_THRESHOLD  =  800;
static constexpr uint16_t SBUS_HIGH_THRESHOLD = 1200;
static constexpr uint16_t SBUS_PWM_MIN        =  200;
static constexpr uint16_t SBUS_PWM_MAX        = 1850;

// ======== PWM-Wert-Quellengrenze ================================
// pwm_wert[x] >= PWM_CHANNEL_OFFSET → Kanalgesteuert (Kanal = pwm_wert - Offset)
static constexpr int PWM_CHANNEL_OFFSET = 300;

// ======== MultiSwitch-Protokoll =================================
static constexpr uint8_t MWset       = 0x01;
static constexpr uint8_t MWprop      = 0x02;  // NEU v0.14: Proportional-Kanal
static constexpr uint8_t MWset4      = 0x07;
static constexpr uint8_t MWset4m     = 0x09;
static constexpr uint8_t Multiswitch = 0xA1;

// ======== PWM-Konfiguration =====================================
static constexpr int     PWM_FREQ       = 5000;
static constexpr int     PWM_RESOLUTION = 8;
static constexpr uint8_t PWM_MAX        = 255;

// ======== GPIO ==================================================
static constexpr uint8_t OutPin[8] = {18, 19, 21, 22, 23, 25, 26, 27};
static constexpr uint8_t WifiPin   = 13;
static constexpr uint8_t LedPin    = 2;

// ======== Ausgangszustand =======================================
int     pwm_wert[8]      = {255, 255, 255, 255, 255, 255, 255, 255};
int     mode[8]          = {0, 0, 0, 0, 0, 0, 0, 0};
int     Ausgang_Kanal[8] = {0, 1, 2, 3, 4, 5, 6, 7};
char    Ausgang_Name[8][17] = {
    "Ausgang 1","Ausgang 2","Ausgang 3","Ausgang 4",
    "Ausgang 5","Ausgang 6","Ausgang 7","Ausgang 8"
};
bool    Ausgang[8]       = {};

unsigned long previousTimeLED[8] = {};
bool          blinkOn[8]          = {};

// ======== RC-Bus ================================================
int  RC_System_boot = 0;

bfs::SbusRx   sbus_rx(&Serial1, 16, 17, true); // Serial1 fuer SBUS (GPIO16/17)
bfs::SbusData sbus_data;

CRSF crsf;

uint16_t channel_output[16] = {};
bool     BUS_OK              = false;

// ======== CRSF-Timeout ==========================================
// v0.14: auf 2000 ms erhöht – robuster bei kurzen Unterbrechungen
static constexpr unsigned long CRSF_TIMEOUT_MS = 2000;
unsigned long lastCrsfPacket = 0;

// ======== MWprop-Speicher (NEU v0.14) ===========================
// Speichert duty-Werte (0–255) für bis zu 8 proportionale CRSF-Kanäle.
// Verwendung: pwm_wert[x] = 200..207 → wm_prop_value[pwm_wert[x]-200]
uint8_t wm_prop_value[8] = {255, 255, 255, 255, 255, 255, 255, 255};

// ======== Konfiguration (aus NVS) ===============================
int RC_System      = 0;
int einkanal_mode  = 0;
int CRSF_Channel   = 4;
int modul_adress   = 0;

uint16_t einkanal_Data       = 0;
uint16_t einkanal_SpeicherWM = 0;
uint16_t Data                = 0;

// ======== WiFi ==================================================
char        g_wifi_ssid[33]     = "MultiSwitch";
char        g_wifi_pass[64]     = "123456789";
char        g_wifi_ip[16]       = "192.168.1.1";   // konfigurierbare AP-IP
const char* AP_IP_STR           = g_wifi_ip;        // Alias fuer Abwaertskompatibilitaet
char        g_device_name[24]   = "MultiSwitch";    // NEU V1.4: Geraetename im TBS Agent / LUA

// ======== WLAN-Auto-Failsafe (NEU v2.00, Default AN seit v2.01) ========
// Schaltet den Access Point automatisch ein, wenn ueber die eingestellte
// Zeit kein gueltiges RC-Signal anliegt (nutzt BUS_OK) - z.B. wenn kein
// Sender gebunden ist oder der Empfaenger fehlt, damit man trotzdem per
// WLAN ins Web-Interface kommt, auch wenn der Boot-Pin (GPIO13) gerade
// nicht erreichbar ist. Schaltet NUR ein, nie automatisch wieder aus
// (siehe wifiFailsafeCheck() weiter unten).
// Seit v2.01 Default AN - analog zum Soundmodul-Projekt (dortiges Feature
// "WLAN dauerhaft an"), ebenfalls fuer bestehende Geraete beim Update
// (siehe Migrations-Default in nvsLoad() unten: p.getBool("wauto", true)).
// WICHTIG: dieser Schalter schaltet WLAN NICHT sofort beim Booten ein,
// sondern erst nach g_wifi_auto_timeout Sekunden OHNE gueltiges RC-Signal -
// bei normal gebundenem Sender bleibt WLAN also weiterhin aus, anders als
// beim Soundmodul, das WLAN dort unabhaengig vom RC-Signal sofort startet.
bool     g_wifi_auto         = true;
uint16_t g_wifi_auto_timeout = 60;   // Sekunden, gueltiger Bereich 5-240

// WLAN dauerhaft aktiv (NEU v2.02, uebernommen aus dem Soundmodul-Projekt
// ESP32-RC-Sound v7.16, dortiges Feature "WifiAlwaysOn"): der AP wird in
// webui_init() fest eingeschaltet und danach von der Firmware nie mehr
// ausgeschaltet - der GPIO13-Bootpin (siehe webui_init() in webui.h) UND
// der Auto-Failsafe oben (g_wifi_auto/g_wifi_auto_timeout) werden dabei
// uebersprungen, siehe wifiFailsafeCheck() weiter unten. Anders als beim
// Soundmodul gibt es hier keinen Sofort-Schalter aus dem CRSF-Menue heraus -
// Feld 80 unten wirkt wie 78/79 rein persistierend, kein Aufruf von
// webui_enableAP() aus luaSetWifiAlways() heraus (siehe Kommentar dort).
// Default AN (true), auch fuer Bestandsgeraete beim ersten Boot nach einem
// Firmware-Update (siehe Migrations-Default in nvsLoad() unten:
// p.getBool("walways", true)).
bool     g_wifi_always_on    = true;

// ======== NVS ===================================================
static bool nvsDirty = false;
static bool nvsSaveScheduled = false;
static unsigned long nvsSaveDueMs = 0;
static constexpr unsigned long NVS_SAVE_DEBOUNCE_MS = 500;

// v2.05: WLAN, Moduladresse und WLAN-Failsafe liegen unter den
// gemeinsamen RC_Common-Schluesseln, siehe src/RC_Common/settings_common.h
static void copyStr(char* dst, const char* src, size_t len) {
    strncpy(dst, src, len - 1);
    dst[len - 1] = '\0';
}

static void commonToSettings(RcCommonSettings& s) {
    memset(&s, 0, sizeof(s));
    copyStr(s.ssid,    g_wifi_ssid,   sizeof(s.ssid));
    copyStr(s.pass,    g_wifi_pass,   sizeof(s.pass));
    copyStr(s.ip,      g_wifi_ip,     sizeof(s.ip));
    copyStr(s.devName, g_device_name, sizeof(s.devName));
    s.moduleAddr  = modul_adress;
    s.wifiAuto    = g_wifi_auto;
    s.wifiTimeout = g_wifi_auto_timeout;
    s.wifiAlways  = g_wifi_always_on;
}

static void settingsToCommon(const RcCommonSettings& s) {
    copyStr(g_wifi_ssid,   s.ssid,    sizeof(g_wifi_ssid));
    copyStr(g_wifi_pass,   s.pass,    sizeof(g_wifi_pass));
    copyStr(g_wifi_ip,     s.ip,      sizeof(g_wifi_ip));
    copyStr(g_device_name, s.devName, sizeof(g_device_name));
    modul_adress        = s.moduleAddr;
    g_wifi_auto         = s.wifiAuto;
    g_wifi_auto_timeout = s.wifiTimeout;
    g_wifi_always_on    = s.wifiAlways;
}

static void nvsWriteNow() {
    Preferences p;
    p.begin("msw", false);
    p.putInt("rc_sys",  RC_System);
    p.putInt("crsf_ch", CRSF_Channel);
    p.putInt("ek_mode", einkanal_mode);
    for (int i = 0; i < 8; i++) {
        char key[4];
        snprintf(key, sizeof(key), "ak%d", i); p.putInt(key, Ausgang_Kanal[i]);
        snprintf(key, sizeof(key), "pw%d", i); p.putInt(key, pwm_wert[i]);
        snprintf(key, sizeof(key), "mo%d", i); p.putInt(key, mode[i]);
        snprintf(key, sizeof(key), "nm%d", i); p.putString(key, Ausgang_Name[i]);
    }
    // v2.05: gemeinsame Schluessel aus RC_Common (settings_common.h)
    RcCommonSettings s;
    commonToSettings(s);
    rcSettingsSave(p, s);
    p.end();
    nvsDirty = false;
    nvsSaveScheduled = false;
    Serial.println("NVS gespeichert.");
}

static void nvsSave() {
    nvsDirty = true;
    nvsSaveScheduled = true;
    nvsSaveDueMs = millis() + NVS_SAVE_DEBOUNCE_MS;
}

static void nvsFlushPending() {
    if (nvsDirty || nvsSaveScheduled) {
        nvsWriteNow();
    }
}

static void nvsProcessPending() {
    if (!nvsSaveScheduled) return;
    if ((long)(millis() - nvsSaveDueMs) >= 0) {
        nvsWriteNow();
    }
}

static void nvsLoad() {
    Preferences p;
    p.begin("msw", true);
    if (!p.isKey("rc_sys")) { p.end(); return; }
    RC_System     = p.getInt("rc_sys",  0);
    CRSF_Channel  = p.getInt("crsf_ch", 4);
    einkanal_mode = p.getInt("ek_mode", 0);
    for (int i = 0; i < 8; i++) {
        char key[4];
        snprintf(key, sizeof(key), "ak%d", i); Ausgang_Kanal[i] = p.getInt(key, i);
        snprintf(key, sizeof(key), "pw%d", i); pwm_wert[i]      = p.getInt(key, 255);
        snprintf(key, sizeof(key), "mo%d", i); mode[i]          = p.getInt(key, 0);
        snprintf(key, sizeof(key), "nm%d", i);
        String nm = p.getString(key, Ausgang_Name[i]);
        strncpy(Ausgang_Name[i], nm.c_str(), 16); Ausgang_Name[i][16] = '\0';
    }
    // v2.05: gemeinsame Schluessel (RC_Common). Fehlen sie (erster Start
    // nach dem Update), werden die alten Schluessel einmalig uebernommen.
    // Defaults wie bisher: WLAN-Auto-Failsafe und "dauerhaft an" AN (siehe
    // Kommentare bei g_wifi_auto/g_wifi_always_on).
    RcCommonSettings s;
    commonToSettings(s);
    bool migrated = rcSettingsLoad(p, s, nullptr);
    settingsToCommon(s);
    p.end();
    nvsDirty = false;
    nvsSaveScheduled = false;
    Serial.println("NVS geladen.");
    if (migrated) {
        nvsWriteNow();
        Serial.println("NVS: Einstellungen auf die RC_Common-Schluessel uebernommen.");
    }
}

static void nvsReset() {
    RC_System = 0; CRSF_Channel = 4; einkanal_mode = 0; modul_adress = 0;
    for (int i = 0; i < 8; i++) {
        Ausgang_Kanal[i] = i; pwm_wert[i] = 255; mode[i] = 0;
        snprintf(Ausgang_Name[i], 17, "Ausgang %d", i + 1);
    }
    strncpy(g_wifi_ssid,   "MultiSwitch", sizeof(g_wifi_ssid)-1);
    strncpy(g_wifi_pass,   "123456789",   sizeof(g_wifi_pass)-1);
    strncpy(g_wifi_ip,     "192.168.1.1", sizeof(g_wifi_ip)-1);
    strncpy(g_device_name, "MultiSwitch", sizeof(g_device_name)-1);  // NEU V1.4
    g_wifi_auto = true; g_wifi_auto_timeout = 60;    // NEU v2.00, Default AN seit v2.01
    g_wifi_always_on = true;                         // NEU v2.02
    nvsSave();
    nvsFlushPending();
}

void storageSave() { nvsFlushPending(); }

// ======== Status-LED ============================================
// v2.05: gemeinsame Muster aus RC_Common (status_led.h): aus = kein
// Signal, langsam = OK, schnell = Frames ohne RC-Link bzw. SBUS-Failsafe.
// Reine Anzeige - aendert weder BUS_OK noch das Ausgangsverhalten.
static unsigned long lastSbusFrameMs = 0;
static StatusLed statusLed;   // GPIO 2

static void updateLed() {
    unsigned long now = millis();
    const LinkStatistics& ls = crsf.getLinkStatistics();
    StatusLedInput in = {};
    in.crsf            = (RC_System_boot == 4);
    in.busOk           = BUS_OK;
    in.lastRcFrameMs   = crsf.getLastRcFrameMs();
    in.linkStatsMs     = ls.lastUpdate;
    in.uplinkLQ        = ls.uplinkLQ;
    in.lastSbusFrameMs = lastSbusFrameMs;
    in.sbusFailsafe    = sbus_data.failsafe;
    statusLed.update(statusLedMode(in, now), now);
}

// ======== CRSF-Failsafe =========================================
// Nur BUS_OK setzen – kein ledcWrite() hier!
// Output() schaltet Ausgänge kontrolliert ab.
static void checkCrsfTimeout() {
    if (RC_System_boot != 4) return;
    if (millis() - lastCrsfPacket > CRSF_TIMEOUT_MS) {
        BUS_OK = false;
    }
}

// ======== SBUS-Zeitlimit (NEU v2.05, wie RC-Multiswitch v4.03) ====
// Ohne SBUS-Frames (nicht nur Failsafe-Bit) gilt das Signal nach 10 s als
// weg. lastSbusFrameMs wird beim Empfang jedes Frames gesetzt (loop()).
static constexpr unsigned long SBUS_TIMEOUT_MS = 10000;
static void checkSbusTimeout() {
    if (RC_System_boot == 4) return;
    if (lastSbusFrameMs == 0 || millis() - lastSbusFrameMs > SBUS_TIMEOUT_MS) {
        BUS_OK = false;
    }
}

// ======== MultiSwitch-Decoder ===================================
static uint8_t compressSwitches(uint16_t state) {
    uint8_t result = 0;
    for (uint8_t i = 0; i < 8; i++) result |= ((state >> (i * 2)) & 0x1) << i;
    return result;
}

static void einkanalFunctionCRSF() {
    uint8_t WMcode  = crsf.get_cmd_buffer(5);
    uint8_t command = crsf.get_cmd_buffer(6);
    if (WMcode != Multiswitch) return;
    switch (command) {
        case MWset4: {
            uint8_t address = crsf.get_cmd_buffer(7);
            if (address == (uint8_t)modul_adress) {
                uint16_t state = ((uint16_t)crsf.get_cmd_buffer(8) << 8)
                               |             crsf.get_cmd_buffer(9);
                einkanal_Data = compressSwitches(state);
            }
            break;
        }
        case MWset4m: {
            uint8_t count = min((uint8_t)crsf.get_cmd_buffer(7), (uint8_t)7);
            for (uint8_t i = 0; i < count; i++) {
                uint8_t address = crsf.get_cmd_buffer(8 + (3 * i));
                if (address == (uint8_t)modul_adress) {
                    uint16_t state = ((uint16_t)crsf.get_cmd_buffer(9  + (3*i)) << 8)
                                   |             crsf.get_cmd_buffer(10 + (3*i));
                    einkanal_Data = compressSwitches(state);
                }
            }
            break;
        }
        case MWset: {
            uint8_t address = crsf.get_cmd_buffer(7);
            if (address == (uint8_t)modul_adress)
                einkanal_Data = crsf.get_cmd_buffer(8);
            break;
        }
        // NEU v0.14: MWprop – setzt duty-Wert für proportionale PWM-Ausgänge
        case MWprop: {
            uint8_t address = crsf.get_cmd_buffer(7);
            if (address == (uint8_t)modul_adress) {
                uint8_t channel = crsf.get_cmd_buffer(8);
                uint8_t duty    = crsf.get_cmd_buffer(9); // 0-100%
                if (channel < 8) {
                    // MWprop sendet Prozent (0-100), PWM braucht 0-255
                    wm_prop_value[channel] = (uint8_t)((uint16_t)duty * 255 / 100);
                }
            }
            break;
        }
    }
}

// ======== SBUS-Einkanal-Decoder =================================
static void einkanalFunctionSBUS(uint16_t channel) {
    einkanal_Data = channel;
    if (einkanal_mode == 0) {
        switch (RC_System) {
            case 0: einkanal_Data /= 8; break;
            case 1:
                einkanal_Data = constrain(einkanal_Data, 206, 1837);
                einkanal_Data = ((einkanal_Data - 206) * 10 + 20) / 64;
                break;
            case 2: {
                float v = ((float)einkanal_Data - 172.0f + 1.5f) * 0.155677655677655f;
                einkanal_Data = (uint16_t)v;
                break;
            }
        }
    } else if (einkanal_mode >= 10) {
        uint16_t n = 0;
        switch (RC_System) {
            case 0: n = (channel >= 172) ? (channel - 172 + 1) : 0; break;
            case 1: n = (channel >= 220) ? (channel - 220)     : 0; n += (n >> 6); break;
            case 2: n = (channel >= 172) ? (channel - 172)     : 0; break;
            case 3: n = (channel >= 205) ? (channel - 205)     : 0; break;
        }
        uint8_t v       = (uint8_t)(n >> 4);
        uint8_t address = (v >> 4) & 0b11;
        uint8_t sw      = (v >> 1) & 0b111;
        uint8_t state   = v & 0b1;
        if (address == (uint8_t)(einkanal_mode - 10))
            bitWrite(einkanal_SpeicherWM, sw, state);
        einkanal_Data = einkanal_SpeicherWM;
    }
}

// ======== CRSF LUA Parameter-System =====================================
// Parameterstruktur (nach rcmultiswitchG030 Referenz):
//   0:  Root Folder, 1: Version Info, 2: Folder "Global"
//   3: Switch Addr, 4: CRSF Kanal
//   5+x*5+0..4: Folder + 4 Parameter pro Ausgang (x=0..7)
// ======== CRSF LUA / TBS Agent Parameter-System ========================
//
// Pro Ausgang (9 Eintraege inkl. Folder):
//   fi+0: Folder
//   fi+1: Sel "Ausgang-Quelle"  Einzelkanal;Kanal_L;Kanal_H
//   fi+2: U8  "Kanal Nr"        1-8 (Einzelkanal) oder 1-16 (Kanal_L/H)
//   fi+3: Sel "Blink"           Dauerlicht;Blinken
//   fi+4: U8  "Blink AN *10ms"  1-250
//   fi+5: U8  "Blink AUS *10ms" 1-250
//   fi+6: Sel "PWM Modus"       Festwert;MWprop (nur CRSF)
//   fi+7: U8  "PWM Festwert"    0-255
//   fi+8: Sel "Test"            Aus;Ein
// Global: 0=Root,1=Version,2=Folder,3=Switch Addr,4=CRSF Kanal
// WLAN (NEU v2.02, uebernommen aus dem Soundmodul-Projekt, dortiges
// CRSF/Lua-Menue fuer WLAN-Einstellungen, Felder 174-181): eigener
// Ordner 77 mit Kindern 78-80, an die Root angehaengt statt bestehende
// IDs 0-76 umzunummerieren, damit bestehende Radio-Profile/Lua-Skripte
// mit festen Feld-IDs weiterlaufen (gleiches Vorgehen wie beim
// Soundmodul beim Anhaengen von Feld 181 an Ordner 160).
//   77: Folder "WLAN"         (Kinder 78,79,80)
//   78: Sel  "Auto-Failsafe"  Aus;Ein  -> g_wifi_auto
//   79: U8   "Auto Timeout"   5-240 s  -> g_wifi_auto_timeout
//   80: Sel  "Dauerhaft an"   Aus;Ein  -> g_wifi_always_on
// Total: 5+8*9+4=81 Parameter (0..80), Tabelle siehe buildLuaMenu()

// ── CRSF-Geraeteadresse + Ping-Slot aus der WM-Adresse (uebernommen aus Soundmodul v1.24) ──
// Adresse = 0xC0 + modul_adress; Slot = (Adresse-0xC0)*2; Antwort erst im eigenen
// Zeit-Slot, damit sich die Geraeteantworten mehrerer Module nicht ueberlappen.
#define CRSF_SLOT_MS 5
static inline uint8_t  crsfAddrFromWM()  { return 0xC0 + (uint8_t)constrain(modul_adress, 0, 15); }
static inline uint16_t crsfSlotDelayMs() { return (uint16_t)((uint8_t)constrain(modul_adress, 0, 15) * 2) * CRSF_SLOT_MS; }

// Ausgang_Kanal[x] -> Quelle (0=Einzelkanal,1=Kanal_L,2=Kanal_H) + Kanalnummer (1-basiert)
static uint8_t getKanalQuelle(int x) {
    int k = Ausgang_Kanal[x];
    if (k < 20) return 0;
    if (k < 40) return 1;
    return 2;
}
static uint8_t getKanalNr(int x) {
    int k = Ausgang_Kanal[x];
    if (k < 20) return k + 1;       // Einzelkanal: Bit 1-8
    if (k < 40) return k - 20 + 1; // Kanal_L: Kanal 1-16
    return k - 40 + 1;              // Kanal_H: Kanal 1-16
}
static void setKanalQuelleNr(int x, uint8_t quelle, uint8_t nr) {
    uint8_t idx = (nr > 0) ? nr - 1 : 0; // 0-basiert
    if (quelle == 0) Ausgang_Kanal[x] = min(idx, (uint8_t)7);  // Einzelkanal: 0-7
    else if (quelle == 1) Ausgang_Kanal[x] = 20 + min(idx, (uint8_t)15); // Kanal_L
    else Ausgang_Kanal[x] = 40 + min(idx, (uint8_t)15);        // Kanal_H
}

// PWM-Modus: 0=Festwert, 1=MWprop
static uint8_t getPwmModus(int x) {
    return (pwm_wert[x] >= 200 && pwm_wert[x] <= 207) ? 1 : 0;
}

// ── Lua-Menue als Tabelle (v2.05, RC_Common lua_menu.h) ──────────────
// Jeder Eintrag steht einmal in buildLuaMenu(), Senden und Schreiben
// laufen generisch ueber luaMenu. Lese-/Schreibfunktionen bekommen als
// Argument die Ausgang-Nummer. Schreiben ist auf min/max der gerade
// gesendeten Ansicht begrenzt.
static LuaItem luaItems[90];
static LuaMenu luaMenu(luaItems, 90);
static char    luaOutName[8][12];   // "Ausgang 1".."Ausgang 8"

static uint8_t luaGetAddr(uint8_t)              { return (uint8_t)modul_adress; }
static void    luaSetAddr(uint8_t, uint8_t v)   { modul_adress = v; crsf.setDeviceAddress(crsfAddrFromWM()); nvsSave(); }
static uint8_t luaGetCrsfCh(uint8_t)            { return (uint8_t)CRSF_Channel; }
static void    luaSetCrsfCh(uint8_t, uint8_t v) { CRSF_Channel = v; nvsSave(); }

static uint8_t luaGetQuelle(uint8_t x)            { return getKanalQuelle(x); }
static void    luaSetQuelle(uint8_t x, uint8_t v) { setKanalQuelleNr(x, v, getKanalNr(x)); nvsSave(); }  // Nr bleibt
static uint8_t luaGetNr(uint8_t x)                { return getKanalNr(x); }
static void    luaSetNr(uint8_t x, uint8_t v)     { setKanalQuelleNr(x, getKanalQuelle(x), v); nvsSave(); }
static void    luaViewNr(uint8_t x, LuaView& v, char*) {
    uint8_t q = getKanalQuelle(x);
    bool rc = (q == 1 || q == 2);   // Kanal_L/Kanal_H: RC-Kanal 1-16, sonst Bit 1-8
    v.name = rc ? "RC-Kanal Nr" : "Bit Nr";
    v.max  = rc ? 16 : 8;
}

static uint8_t luaGetBlink(uint8_t x) { return ((mode[x] >> 8) & 0xFF) ? 1 : 0; }
static void    luaSetBlink(uint8_t x, uint8_t v) {
    if (v == 0) {
        mode[x] = 0;
    } else {
        uint8_t mH = (mode[x] >> 8) & 0xFF;
        uint8_t mL =  mode[x]        & 0xFF;
        if (!mH) mH = 10;
        if (!mL) mL = 10;
        mode[x] = ((uint16_t)mH << 8) | mL;
    }
    nvsSave();
}
static uint8_t luaGetBlinkOn(uint8_t x)  { uint8_t mH = (mode[x] >> 8) & 0xFF; return mH ? mH : 10; }
static void    luaSetBlinkOn(uint8_t x, uint8_t v)  { mode[x] = ((uint16_t)v << 8) | (mode[x] & 0xFF); nvsSave(); }
static uint8_t luaGetBlinkOff(uint8_t x) { uint8_t mL = mode[x] & 0xFF; return mL ? mL : 10; }
static void    luaSetBlinkOff(uint8_t x, uint8_t v) { mode[x] = (mode[x] & 0xFF00) | v; nvsSave(); }

static uint8_t luaGetPwmModus(uint8_t x) { return getPwmModus(x); }
static void    luaSetPwmModus(uint8_t x, uint8_t v) {
    if (v == 1) {
        pwm_wert[x] = 200 + x;        // MWprop
        einkanal_Data |= (1 << x);    // Ausgang einschalten
    } else if (getPwmModus(x) == 1) {
        pwm_wert[x] = 255;            // zurueck auf Festwert
    }
    nvsSave();
}
static void luaViewPwmModus(uint8_t, LuaView& v, char*) {
    if (RC_System_boot != 4) { v.type = LUA_INFO; v.text = "Festwert (SBUS)"; }   // MWprop nur bei CRSF
}
static uint8_t luaGetPwm(uint8_t x)            { return (uint8_t)pwm_wert[x]; }
static void    luaSetPwm(uint8_t x, uint8_t v) { pwm_wert[x] = v; nvsSave(); }
static void    luaViewPwm(uint8_t x, LuaView& v, char* buf) {
    if (getPwmModus(x) != 1) return;
    snprintf(buf, LUA_VIEW_BUF, "MWprop Kanal %d (auto)", x + 1);
    v.type = LUA_INFO; v.name = "PWM Wert"; v.text = buf;
}
static uint8_t luaGetTest(uint8_t)             { return 0; }
static void    luaSetTest(uint8_t x, uint8_t v) {
    if (v) einkanal_Data |=  (1 << x);
    else   einkanal_Data &= ~(1 << x);
}

// WLAN 78-80: rein persistierend, wirkt beim naechsten Timeout bzw. Booten
// (kein WLAN-Start aus dem CRSF-Handler, Lehre aus dem Soundmodul v7.15)
static uint8_t luaGetWifiAuto(uint8_t)              { return g_wifi_auto ? 1 : 0; }
static void    luaSetWifiAuto(uint8_t, uint8_t v)   { g_wifi_auto = (v != 0); nvsSave(); }
static uint8_t luaGetWifiTo(uint8_t)                { return (uint8_t)constrain(g_wifi_auto_timeout, 5, 240); }
static void    luaSetWifiTo(uint8_t, uint8_t v)     { g_wifi_auto_timeout = v; nvsSave(); }
static uint8_t luaGetWifiAlways(uint8_t)            { return g_wifi_always_on ? 1 : 0; }
static void    luaSetWifiAlways(uint8_t, uint8_t v) { g_wifi_always_on = (v != 0); nvsSave(); }

static void buildLuaMenu() {
    LuaMenu& m = luaMenu;
    m.clear();
    m.folder(0, 0, "");
    m.info(1, 0, "Version", "v2.05 ESP32");
    m.folder(2, 0, "Global");
    m.u8(3, 2, "Switch Addr", 0, 20, "", luaGetAddr, luaSetAddr);
    m.u8(4, 2, "CRSF Kanal", 0, 15, "", luaGetCrsfCh, luaSetCrsfCh);
    for (uint8_t x = 0; x < 8; x++) {
        uint8_t fi = 5 + x * 9;
        snprintf(luaOutName[x], sizeof(luaOutName[x]), "Ausgang %d", x + 1);
        m.folder(fi, 0, luaOutName[x]);
        m.select(fi + 1, fi, "Ausgang-Quelle", "Einzelkanal;Kanal_L;Kanal_H",
                 luaGetQuelle, luaSetQuelle, x);
        m.u8(fi + 2, fi, "Bit Nr", 1, 8, "", luaGetNr, luaSetNr, x)->view = luaViewNr;
        m.select(fi + 3, fi, "Blink", "Dauerlicht;Blinken", luaGetBlink, luaSetBlink, x);
        m.u8(fi + 4, fi, "Blink AN *10ms", 1, 250, "", luaGetBlinkOn, luaSetBlinkOn, x);
        m.u8(fi + 5, fi, "Blink AUS *10ms", 1, 250, "", luaGetBlinkOff, luaSetBlinkOff, x);
        m.select(fi + 6, fi, "PWM Modus", "Festwert;MWprop (Kanal=Ausgang)",
                 luaGetPwmModus, luaSetPwmModus, x)->view = luaViewPwmModus;
        m.u8(fi + 7, fi, "PWM Festwert", 0, 255, "", luaGetPwm, luaSetPwm, x)->view = luaViewPwm;
        m.select(fi + 8, fi, "Test", "Aus;Ein", luaGetTest, luaSetTest, x);
    }
    m.folder(77, 0, "WLAN");
    m.select(78, 77, "Auto-Failsafe", "Aus;Ein", luaGetWifiAuto, luaSetWifiAuto);
    m.u8(79, 77, "Auto Timeout", 5, 240, "s", luaGetWifiTo, luaSetWifiTo);
    m.select(80, 77, "Dauerhaft an", "Aus;Ein", luaGetWifiAlways, luaSetWifiAlways);
}

// ======== Webinterface ==========================================





#include "webui.h"

// ======== WLAN-Auto-Failsafe (NEU v2.00) ========================
// Schaltet NUR ein, nie automatisch wieder aus: einmal aktiviertes WLAN
// bleibt an bis zum manuellen Ausschalten (Web) oder Neustart - ein
// automatisches Wieder-Abschalten wuerde eine gerade laufende Konfiguration
// mitten drin abwuergen, sobald zufaellig kurz ein gueltiges RC-Signal
// hereinkaeme. g_wifi_auto/g_wifi_auto_timeout siehe weiter oben, webui_
// isApActive()/webui_enableAP() in webui.h.
static WifiFailsafe wifiFailsafe;   // RC_Common wifi_failsafe.h (v2.05)
static void wifiFailsafeCheck() {
    // "Dauerhaft an": AP laeuft seit dem Booten (webui_init()), der Timer
    // ist dann wirkungslos. Laeuft der AP schon, ist nichts zu tun.
    if (wifiFailsafe.check(millis(), g_wifi_always_on, g_wifi_auto, g_wifi_auto_timeout,
                           BUS_OK, webui_isApActive())) {
        Serial.printf("Auto-WLAN-Failsafe: %u s ohne gueltiges RC-Signal - schalte WLAN ein.\n",
                      (unsigned)g_wifi_auto_timeout);
        webui_enableAP();
    }
}

// ======== PWM-Duty für Ausgang x ermitteln (NEU v0.14) ==========
// Unterstützt drei Quellen:
//   pwm_wert[x] < 200          → Festwert (direkt als uint8_t)
//   pwm_wert[x] 200..207       → wm_prop_value[pwm_wert[x]-200] (MWprop)
//   pwm_wert[x] >= PWM_CHANNEL_OFFSET → Kanalgesteuert (SBUS/CRSF-Kanal)
static uint8_t getPwmDuty(int x) {
    int pv = pwm_wert[x];
    if (pv >= PWM_CHANNEL_OFFSET) {
        // Kanal-gemappter PWM-Wert
        return (uint8_t)map(channel_output[pv - PWM_CHANNEL_OFFSET],
                            SBUS_PWM_MIN, SBUS_PWM_MAX, 0, PWM_MAX);
    } else if (pv >= 200 && pv <= 207) {
        // MWprop-Kanal (NEU v0.14)
        return wm_prop_value[pv - 200];
    } else {
        return (uint8_t)pv;
    }
}

// ======== Output ================================================
// Zentrale Ausgabefunktion – EINZIGER Ort, der ledcWrite() auf OutPin[] aufruft.
static void Output(uint16_t d) {
    unsigned long now = millis();
    for (int x = 0; x < 8; x++) {

        if (g_manual_override[x]) {
            bool newState = g_manual_state[x];
            if (newState && !Ausgang[x]) {
                previousTimeLED[x] = now - 9999;
                blinkOn[x] = false;
            }
            Ausgang[x] = newState;

        } else if (!BUS_OK) {
            Ausgang[x] = false;

        } else {
            int k = Ausgang_Kanal[x];
            // MWprop-Modus: Ausgang immer AN (PWM-Wert steuert Helligkeit)
            if (pwm_wert[x] >= 200 && pwm_wert[x] <= 207) {
                Ausgang[x] = true;
            } else if (k < 20) {
                Ausgang[x] = bitRead(d, k);
            } else if (k < 40) {
                Ausgang[x] = (channel_output[k-20] < SBUS_LOW_THRESHOLD);
            } else if (k < 60) {
                Ausgang[x] = (channel_output[k-40] > SBUS_HIGH_THRESHOLD);
            }
        }

        if (Ausgang[x]) {
            int modeH = (mode[x] >> 8) & 0xFF;
            int modeL =  mode[x]        & 0xFF;
            if (modeH > 0 && modeL == 0) modeL = modeH;

            uint8_t duty = getPwmDuty(x);  // NEU: ausgelagerter Helfer

            if (modeH > 0) {
                if (!blinkOn[x]) {
                    if (now - previousTimeLED[x] >= (unsigned long)(10 * modeL)) {
                        previousTimeLED[x] = now;
                        blinkOn[x] = true;
                        ledcWrite(OutPin[x], duty);
                    }
                } else {
                    if (now - previousTimeLED[x] >= (unsigned long)(10 * modeH)) {
                        previousTimeLED[x] = now;
                        blinkOn[x] = false;
                        ledcWrite(OutPin[x], 0);
                    }
                }
            } else {
                blinkOn[x] = true;
                ledcWrite(OutPin[x], duty);
            }
        } else {
            blinkOn[x] = false;
            ledcWrite(OutPin[x], 0);
        }
    }
}

// ======== Setup =================================================
void setup() {
    Serial.begin(115200);
    nvsLoad();

    RC_System_boot = RC_System;

    if (RC_System_boot == 4) {
        crsf.init_crsf(&Serial2, 16, 17); // Serial2 fuer CRSF (GPIO16=RX, 17=TX)
        crsf.setDeviceAddress(crsfAddrFromWM());   // eindeutige CRSF-Adresse aus WM-Adresse
        Serial.printf("CRSF gestartet (RX=16, TX=17, 420000 Bd)  Geraeteadresse 0x%02X  Slot %d\n",
                      crsfAddrFromWM(), (uint8_t)constrain(modul_adress,0,15)*2);
    } else {
        sbus_rx.Begin();
        Serial.printf("SBUS gestartet (RX=16, RC-System=%d)\n", RC_System_boot);
    }

    for (int i = 0; i < 8; i++) {
        ledcAttach(OutPin[i], PWM_FREQ, PWM_RESOLUTION);
        ledcWrite(OutPin[i], 0);
    }

    pinMode(WifiPin, INPUT_PULLUP);
    statusLed.begin(LedPin, false);   // v2.05: RC_Common status_led.h
    buildLuaMenu();

    webui_init();

    Serial.printf("MultiSwitch ESP32 v%d.%02d\n", Version / 100, Version % 100);
    Serial.printf("SSID: %s\n", g_wifi_ssid);
    Serial.printf("IP:   %s\n", AP_IP_STR);
}

// ======== Loop ==================================================
void loop() {

    nvsProcessPending();
    wifiFailsafeCheck(); // NEU v2.00 - unbedingt am Loop-Anfang, damit auch der SBUS-Failsafe-Fruehausstieg unten ihn nicht ueberspringt

    if (RC_System_boot == 4) {
        crsf.read_packets(0);
        for (int i = 0; i < 16; i++)
            channel_output[i] = crsf.get_crfs_channels(i);

        // BUS_OK (v2.00 FIX): jetzt an "wurde gerade ein gueltiger CRSF-Frame
        // geparst" gekoppelt (crsf.getValidFrames() hat sich erhoeht), nicht
        // mehr an "channel_output[0] > 0" - ein Kanalwert von 0 ist legitim
        // und daher kein zuverlaessiger Nachweis fuer einen frischen Frame.
        static uint32_t lastValidFrameCount = 0;
        uint32_t validFrames = crsf.getValidFrames();
        if (validFrames != lastValidFrameCount) {
            lastValidFrameCount = validFrames;
            BUS_OK = true;
            lastCrsfPacket = millis();
        }
        checkCrsfTimeout();

        // Kein Telemetrie-Dummy: DEVICE_PING / LUA-Pakete werden durch
        // device_info und param-Antworten sichergestellt (keine Dummy-Daten noetig)

        // Bei CRSF: Einkanal-Steuerung NUR ueber WM-Protokoll (einkanalFunctionCRSF).
        // einkanalFunctionSBUS NICHT aufrufen - CRSF-Kanalwerte sind zu gross
        // fuer SBUS-WM-Dekodierung und wuerden einkanal_Data verfaelschen.
        // (Fehler in V1.1: ELSE-Zweig lief auch bei CRSF -> falsches Schalten)
        // ELRS LUA Script: Device Info Antwort auf DEVICE_PING
        if (crsf.getDeviceInfoReplyPending() && (millis() - crsf.getPingTime() >= crsfSlotDelayMs())) {
            crsf.setDeviceInfoReplyPending(false);
            char devName[24];
            snprintf(devName, sizeof(devName), "%s@%d", g_device_name, modul_adress);  // NEU V1.4
            // NEU v2.03, uebernommen aus dem Soundmodul-Projekt (v7.18):
            // Firmware-Version als CRSF "Parameter Version Number"
            // mitgeben (war zuvor fest 0x00, siehe Kommentar in
            // crsf_esp32.cpp/send_device_info()) - damit erkennen
            // Konfiguratoren wie TBS Agent eine geaenderte Parameter-
            // struktur zuverlaessig und lesen sie neu ein.
            crsf.send_device_info(devName, luaMenu.maxId(), (uint8_t)(Version & 0xFF));
        }
        // ELRS LUA Script: Parameter lesen
        if (crsf.getDeviceReadReplyPending()) {
            crsf.setDeviceReadReplyPending(false);
            luaMenu.send(crsf, crsf.getParamReadIndex());
        }
        // ELRS LUA Script: Parameter schreiben + Read-Back (CRSF Standard)
        if (crsf.getDeviceWriteReplyPending()) {
            uint8_t widx = crsf.getParamWriteIndex();
            uint8_t wval = crsf.getParamWriteValue();
            crsf.setDeviceWriteReplyPending(false);
            luaMenu.write(widx, wval);
            // Read-Back: geschriebenen Parameter direkt zuruecksenden
            luaMenu.send(crsf, widx);
        }
        // WM-Protokoll Befehle
        if (crsf.getDeviceCommandReplyPending()) {
            crsf.setDeviceCommandReplyPending(false);
            einkanalFunctionCRSF();
        }
        Data = einkanal_Data;

    } else {
        if (sbus_rx.Read()) {
            sbus_data = sbus_rx.data();
            lastSbusFrameMs = millis();   // v2.05: SBUS-Zeitlimit und LED

            if (sbus_data.failsafe) {
                BUS_OK = false;
                updateLed();
                webui_handle();
                return;
            }

            BUS_OK = true;
            for (int i = 0; i < 16; i++)
                channel_output[i] = sbus_data.ch[i];

            if (einkanal_mode != 999 && CRSF_Channel < 16)
                einkanalFunctionSBUS(sbus_data.ch[CRSF_Channel]);
            Data = einkanal_Data;
        }
        checkSbusTimeout();   // v2.05
    }

    Output(Data);
    updateLed();
    webui_handle();
}
