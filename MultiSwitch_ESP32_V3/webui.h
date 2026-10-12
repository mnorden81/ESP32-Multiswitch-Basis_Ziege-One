/*
 * webui.h  –  WiFi-Weboberfläche für ESP32-MultiSwitch  v3.10
 * NEU V3.00: MKan-Gruppen (2 zusaetzliche, unabhaengige Kanalquellen) -
 * siehe Kommentar bei MULTISW_NUM_GROUPS in MultiSwitch_ESP32_V3.ino.
 * NEU V3.10: bis zu 3 Konfigurationen (Kanal-Quelle + Blink/PWM) pro
 * Ausgang, Prioritaet Konfig1 > Konfig2 > Konfig3 - siehe evalSlotActive()/
 * Output() in MultiSwitch_ESP32_V3.ino. Konfig 2/3 sind bewusst NUR hier in
 * der Web-UI verfuegbar, nicht im CRSF-LUA-Menue (siehe README).
 */

#pragma once

#include <ctype.h>
#include <stdlib.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>          // NEU v2.00: Firmware-Update per WLAN (OTA), siehe handleOtaUpdate*()

// ============================================================
//  Forward-Deklarationen
// ============================================================

extern CRSF          crsf;
extern int           RC_System;
extern int           RC_System_boot;
extern int           CRSF_Channel;
extern int           einkanal_mode;
extern int           modul_adress;
extern int           Ausgang_Kanal[8];
extern int           pwm_wert[8];
extern int           mode[8];
extern int           Ausgang_Kanal2[8];         // NEU V3.10: Konfig 2 pro Ausgang
extern int           pwm_wert2[8];              // NEU V3.10: Konfig 2 pro Ausgang
extern int           mode2[8];                  // NEU V3.10: Konfig 2 pro Ausgang
extern int           Ausgang_Kanal3[8];         // NEU V3.10: Konfig 3 pro Ausgang
extern int           pwm_wert3[8];              // NEU V3.10: Konfig 3 pro Ausgang
extern int           mode3[8];                  // NEU V3.10: Konfig 3 pro Ausgang
extern int           EK_Gruppen_Adresse[2];      // NEU V3.00: MKan
extern int           SBUS_Gruppen_Channel[2];    // NEU V3.00: MKan
extern int           SBUS_Gruppen_Mode[2];       // NEU V3.00: MKan
extern char          Ausgang_Name[8][17];
extern uint16_t      channel_output[16];
extern uint16_t      einkanal_Data;
extern bool          BUS_OK;
extern bool          Ausgang[8];
extern const uint8_t OutPin[8];
extern char          g_wifi_ssid[33];
extern char          g_wifi_pass[64];
extern char          g_wifi_ip[16];
extern char          g_device_name[24];   // NEU V1.4
extern const uint16_t Version;            // Firmware-Version (z.B. 140 = v1.4)
extern const char*   AP_IP_STR;
extern uint8_t       wm_prop_value[8];   // NEU v0.14
extern void          storageSave();
extern void          nvsSave();
extern void          nvsReset();
extern bool          g_wifi_auto;          // NEU v2.00
extern uint16_t      g_wifi_auto_timeout;  // NEU v2.00
extern bool          g_wifi_always_on;     // NEU v2.02

// ============================================================
//  Debug
// ============================================================
#ifdef SERIAL_DEBUG
template<typename... A> void _dbg(A... a){ (Serial.print(a),...); Serial.println(); }
#define DBG(...) _dbg(__VA_ARGS__)
#else
#define DBG(...) do{}while(0)
#endif

// ============================================================
//  Manuelle Overrides (Web → Output)
// ============================================================
bool g_manual_override[8] = {};
bool g_manual_state[8]    = {};

// ============================================================
//  WebServer
// ============================================================
static WebServer webServer(80);

// AP-Steuerung (NEU v2.00) - Definitionen weiter unten bei webui_init(),
// hier vorab deklariert, weil handleStatus() (weiter oben im File) den
// aktuellen AP-Status schon mit ausliefert.
bool webui_isApActive();
void webui_enableAP();
void webui_disableAP();

// ============================================================
//  HTML (PROGMEM) – identisch zu v0.13, nur Version aktualisiert
// ============================================================
static const char WEBUI_HTML[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>MultiSwitch</title>
<style>
:root{
  --bg:#0d1117;--surf:#161b22;--surf2:#21262d;
  --border:#30363d;--accent:#4493f8;--green:#3fb950;
  --yellow:#d29922;--red:#f85149;--orange:#e3932a;
  --text:#e6edf3;--sub:#8b949e;--radius:8px;
  --mono:'Courier New',monospace;
}
*{box-sizing:border-box;margin:0;padding:0}
body{background:var(--bg);color:var(--text);font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif;font-size:14px;min-height:100vh}
header{background:var(--surf);border-bottom:1px solid var(--border);padding:12px 16px;display:flex;align-items:center;gap:10px;position:sticky;top:0;z-index:50}
header h1{font-size:16px;font-weight:700}
.hv{font-size:11px;color:var(--sub);margin-top:2px}
.dot{width:8px;height:8px;border-radius:50%;background:var(--sub);margin-left:auto;transition:.4s}
.dot.ok{background:var(--green);box-shadow:0 0 6px var(--green)}
.dot.warn{background:var(--yellow)}
.dot.err{background:var(--red)}
nav{background:var(--surf);border-bottom:1px solid var(--border);display:flex;overflow-x:auto;scrollbar-width:none}
nav::-webkit-scrollbar{display:none}
.tab{flex-shrink:0;padding:10px 16px;font-size:12px;font-weight:600;color:var(--sub);cursor:pointer;border-bottom:2px solid transparent;transition:.2s;white-space:nowrap;user-select:none}
.tab:hover{color:var(--text)}
.tab.active{color:var(--accent);border-bottom-color:var(--accent)}
main{max-width:560px;margin:0 auto;padding:20px 14px 60px}
.card{background:var(--surf);border:1px solid var(--border);border-radius:var(--radius);padding:16px;margin-bottom:12px}
.card-title{font-weight:600;font-size:13px;margin-bottom:12px;color:var(--sub);text-transform:uppercase;letter-spacing:.04em}
.row{display:flex;align-items:center;gap:8px;margin-bottom:8px}
.row label{flex:0 0 110px;font-size:13px;color:var(--sub)}
.row input,.row select{flex:1;background:var(--surf2);border:1px solid var(--border);color:var(--text);border-radius:6px;padding:6px 10px;font-size:13px}
.row input:focus,.row select:focus{outline:none;border-color:var(--accent)}
.btn{padding:7px 14px;border:none;border-radius:6px;font-size:13px;font-weight:600;cursor:pointer;transition:.15s}
.btn-primary{background:var(--accent);color:#fff}
.btn-primary:hover{filter:brightness(1.15)}
.btn-danger{background:var(--red);color:#fff}
.btn-ghost{background:transparent;border:1px solid var(--border);color:var(--text)}
.btn-ghost:hover{border-color:var(--accent);color:var(--accent)}
.btn-sm{padding:5px 10px;font-size:12px}
.grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(150px,1fr));gap:8px;margin-bottom:12px}
.tile{background:var(--surf2);border:1px solid var(--border);border-radius:var(--radius);padding:12px 10px;text-align:center;cursor:pointer;transition:.2s;position:relative}
.tile:hover{border-color:var(--accent)}
.tile.on{background:#0d2c16;border-color:var(--green)}
.tile.override{border-color:var(--orange)}
.tile-name{font-size:12px;font-weight:600;margin-bottom:4px;word-break:break-word}
.tile-sub{font-size:10px;color:var(--sub)}
.lock{position:absolute;top:6px;right:8px;font-size:10px;color:var(--orange)}
.badge{display:inline-block;padding:2px 6px;border-radius:4px;font-size:10px;font-weight:700}
.badge-ok{background:#0d2c16;color:var(--green)}
.badge-err{background:#2c0d0d;color:var(--red)}
.badge-warn{background:#2c200d;color:var(--yellow)}
.sep{border:none;border-top:1px solid var(--border);margin:12px 0}
.mono{font-family:var(--mono);font-size:12px}
/* Output detail */
#out-detail{display:none}
.out-hdr{display:flex;align-items:center;gap:10px;margin-bottom:16px}
.back-btn{background:none;border:none;color:var(--accent);cursor:pointer;font-size:20px;padding:0 4px}
.state-big{font-size:24px;font-weight:700}
.state-on{color:var(--green)}
.state-off{color:var(--sub)}
.action-row{display:flex;gap:8px;margin-bottom:16px}
.tile-btns{display:flex;gap:4px;margin-top:8px;justify-content:center}
.tile-btns button{flex:1;padding:4px 2px;font-size:10px;font-weight:600;border:none;border-radius:4px;cursor:pointer;transition:.15s}
.tile-btn-on{background:var(--green);color:#0d1117}
.tile-btn-off{background:var(--red);color:#fff}
.tile-btn-rel{background:var(--surf);border:1px solid var(--border)!important;color:var(--sub)}
.tile-btn-cfg{background:var(--accent);color:#fff}
</style>
</head>
<body>
<header>
  <div>
    <h1>⚡ MultiSwitch</h1>
    <div class="hv" id="hv">lädt…</div>
  </div>
  <div class="dot" id="dot"></div>
</header>
<nav id="nav">
  <div class="tab active" onclick="showTab('overview')">Übersicht</div>
  <div class="tab" onclick="showTab('outputs')">Ausgänge</div>
  <div class="tab" onclick="showTab('rc')">RC-System</div>
  <div class="tab" onclick="showTab('wifi')">WiFi</div>
  <div class="tab" onclick="showTab('debug')">Debug</div>
</nav>
<main>
  <!-- Übersicht -->
  <div id="tab-overview">
    <div class="grid" id="tile-grid"></div>
    <div style="display:flex;gap:8px;flex-wrap:wrap;margin-bottom:12px">
      <button class="btn btn-ghost btn-sm" onclick="releaseAll()">🔓 Alle freigeben</button>
    </div>
    <div class="card" id="card-channels" style="display:none">
      <div class="card-title">RC-Kanalwerte</div>
      <div id="ch-grid" style="display:grid;grid-template-columns:repeat(4,1fr);gap:6px;font-size:12px;font-family:var(--mono)"></div>
    </div>
  </div>

  <!-- Ausgänge -->
  <div id="tab-outputs" style="display:none">
    <div id="out-list"></div>
    <div id="out-detail">
      <div class="out-hdr">
        <button class="back-btn" onclick="showOutList()">‹</button>
        <span id="out-title" style="font-weight:700;font-size:15px"></span>
        <span id="out-state-big" class="state-big state-off"></span>
      </div>
      <div class="action-row">
        <button class="btn btn-primary" id="btn-on"  onclick="switchOut(true)">Einschalten</button>
        <button class="btn btn-danger"  id="btn-off" onclick="switchOut(false)">Ausschalten</button>
        <button class="btn btn-ghost"   id="btn-rel" onclick="releaseOut()">Freigeben</button>
      </div>
      <div class="card" id="out-cfg"></div>
    </div>
  </div>

  <!-- RC -->
  <div id="tab-rc" style="display:none">
    <div class="card">
      <div class="card-title">RC-System</div>

      <!-- RC-Protokoll -->
      <div class="row"><label>RC-Protokoll</label>
        <select id="sel-rc" onchange="onRcChange()">
          <option value="0">FrSky (SBUS)</option>
          <option value="1">FlySky (SBUS)</option>
          <option value="2">ELRS normiert (SBUS)</option>
          <option value="3">HoTT (SBUS)</option>
          <option value="4">CRSF (ELRS/TBS)</option>
        </select>
      </div>

      <!-- SBUS: Einkanal-Kanal (wie RC-Sound: SBUS 1-16 + Deaktiviert) -->
      <div class="row" id="row-sbus-ch" style="display:none"><label>Einkanal-Kanal</label>
        <select id="sel-sbus-ch" onchange="onSbusChChange()">
          <optgroup label="SBUS Kanal">
            <option value="0">SBUS Kanal 01</option>
            <option value="1">SBUS Kanal 02</option>
            <option value="2">SBUS Kanal 03</option>
            <option value="3">SBUS Kanal 04</option>
            <option value="4">SBUS Kanal 05</option>
            <option value="5">SBUS Kanal 06</option>
            <option value="6">SBUS Kanal 07</option>
            <option value="7">SBUS Kanal 08</option>
            <option value="8">SBUS Kanal 09</option>
            <option value="9">SBUS Kanal 10</option>
            <option value="10">SBUS Kanal 11</option>
            <option value="11">SBUS Kanal 12</option>
            <option value="12">SBUS Kanal 13</option>
            <option value="13">SBUS Kanal 14</option>
            <option value="14">SBUS Kanal 15</option>
            <option value="15">SBUS Kanal 16</option>
          </optgroup>
          <optgroup label="Optionen">
            <option value="999">Deaktiviert</option>
          </optgroup>
        </select>
      </div>

      <!-- SBUS: Einkanal-Modus (nur wenn nicht Deaktiviert) -->
      <div class="row" id="row-ek" style="display:none"><label>Einkanal-Modus</label>
        <select id="sel-ek" onchange="onEkChange()">
          <option value="0">Normal</option>
          <option value="10">SBUS WM Adr 0</option>
          <option value="11">SBUS WM Adr 1</option>
          <option value="12">SBUS WM Adr 2</option>
          <option value="13">SBUS WM Adr 3</option>
        </select>
      </div>

      <!-- CRSF: Modul-Adresse (0-20, wie RC-Sound Vorbild) -->
      <div class="row" id="row-adr" style="display:none"><label>Modul-Adresse</label>
        <select id="sel-adr" onchange="onAdrChange()">
          <optgroup label="Modul Adresse">
            <option value="0">0</option>
            <option value="1">1</option>
            <option value="2">2</option>
            <option value="3">3</option>
            <option value="4">4</option>
            <option value="5">5</option>
            <option value="6">6</option>
            <option value="7">7</option>
            <option value="8">8</option>
            <option value="9">9</option>
            <option value="10">10</option>
            <option value="11">11</option>
            <option value="12">12</option>
            <option value="13">13</option>
            <option value="14">14</option>
            <option value="15">15</option>
            <option value="16">16</option>
            <option value="17">17</option>
            <option value="18">18</option>
            <option value="19">19</option>
            <option value="20">20</option>
          </optgroup>
        </select>
      </div>

      <span style="color:var(--sub);font-size:12px;display:block;margin-top:6px" id="rc-hint"></span>
    </div>

    <!-- MKan-Gruppen (NEU V3.00): 2 zusaetzliche, unabhaengige Kanalquellen -->
    <div class="card">
      <div class="card-title">MKan-Gruppen (V3)</div>

      <div class="row" id="row-mkan-adr0" style="display:none"><label>MKan Gruppe 1 Adresse</label>
        <select id="sel-mkan-adr0" onchange="onMkanChange(0)">
          <option value="255">Aus</option>
          <option value="0">0</option><option value="1">1</option><option value="2">2</option>
          <option value="3">3</option><option value="4">4</option><option value="5">5</option>
          <option value="6">6</option><option value="7">7</option><option value="8">8</option>
          <option value="9">9</option><option value="10">10</option><option value="11">11</option>
          <option value="12">12</option><option value="13">13</option><option value="14">14</option>
          <option value="15">15</option><option value="16">16</option><option value="17">17</option>
          <option value="18">18</option><option value="19">19</option><option value="20">20</option>
        </select>
      </div>
      <div class="row" id="row-mkan-adr1" style="display:none"><label>MKan Gruppe 2 Adresse</label>
        <select id="sel-mkan-adr1" onchange="onMkanChange(1)">
          <option value="255">Aus</option>
          <option value="0">0</option><option value="1">1</option><option value="2">2</option>
          <option value="3">3</option><option value="4">4</option><option value="5">5</option>
          <option value="6">6</option><option value="7">7</option><option value="8">8</option>
          <option value="9">9</option><option value="10">10</option><option value="11">11</option>
          <option value="12">12</option><option value="13">13</option><option value="14">14</option>
          <option value="15">15</option><option value="16">16</option><option value="17">17</option>
          <option value="18">18</option><option value="19">19</option><option value="20">20</option>
        </select>
      </div>

      <div class="row" id="row-mkan-ch0" style="display:none"><label>MKan Gruppe 1 SBUS-Kanal</label>
        <select id="sel-mkan-ch0" onchange="onMkanChange(0)">
          <optgroup label="SBUS Kanal">
            <option value="0">SBUS Kanal 01</option><option value="1">SBUS Kanal 02</option>
            <option value="2">SBUS Kanal 03</option><option value="3">SBUS Kanal 04</option>
            <option value="4">SBUS Kanal 05</option><option value="5">SBUS Kanal 06</option>
            <option value="6">SBUS Kanal 07</option><option value="7">SBUS Kanal 08</option>
            <option value="8">SBUS Kanal 09</option><option value="9">SBUS Kanal 10</option>
            <option value="10">SBUS Kanal 11</option><option value="11">SBUS Kanal 12</option>
            <option value="12">SBUS Kanal 13</option><option value="13">SBUS Kanal 14</option>
            <option value="14">SBUS Kanal 15</option><option value="15">SBUS Kanal 16</option>
          </optgroup>
          <optgroup label="Optionen"><option value="999">Deaktiviert</option></optgroup>
        </select>
      </div>
      <div class="row" id="row-mkan-mode0" style="display:none"><label>MKan Gruppe 1 Modus</label>
        <select id="sel-mkan-mode0" onchange="onMkanChange(0)">
          <option value="0">Normal</option>
          <option value="10">SBUS WM Adr 0</option><option value="11">SBUS WM Adr 1</option>
          <option value="12">SBUS WM Adr 2</option><option value="13">SBUS WM Adr 3</option>
        </select>
      </div>
      <div class="row" id="row-mkan-ch1" style="display:none"><label>MKan Gruppe 2 SBUS-Kanal</label>
        <select id="sel-mkan-ch1" onchange="onMkanChange(1)">
          <optgroup label="SBUS Kanal">
            <option value="0">SBUS Kanal 01</option><option value="1">SBUS Kanal 02</option>
            <option value="2">SBUS Kanal 03</option><option value="3">SBUS Kanal 04</option>
            <option value="4">SBUS Kanal 05</option><option value="5">SBUS Kanal 06</option>
            <option value="6">SBUS Kanal 07</option><option value="7">SBUS Kanal 08</option>
            <option value="8">SBUS Kanal 09</option><option value="9">SBUS Kanal 10</option>
            <option value="10">SBUS Kanal 11</option><option value="11">SBUS Kanal 12</option>
            <option value="12">SBUS Kanal 13</option><option value="13">SBUS Kanal 14</option>
            <option value="14">SBUS Kanal 15</option><option value="15">SBUS Kanal 16</option>
          </optgroup>
          <optgroup label="Optionen"><option value="999">Deaktiviert</option></optgroup>
        </select>
      </div>
      <div class="row" id="row-mkan-mode1" style="display:none"><label>MKan Gruppe 2 Modus</label>
        <select id="sel-mkan-mode1" onchange="onMkanChange(1)">
          <option value="0">Normal</option>
          <option value="10">SBUS WM Adr 0</option><option value="11">SBUS WM Adr 1</option>
          <option value="12">SBUS WM Adr 2</option><option value="13">SBUS WM Adr 3</option>
        </select>
      </div>
      <span style="color:var(--sub);font-size:12px;display:block;margin-top:6px">Zusaetzlich zur Einzelkanal-Adresse oben: bis zu 16 weitere, unabhaengige Trigger-Bits fuer die Ausgangs-Kanalquelle "MKan1"/"MKan2".</span>
    </div>
  </div>

  <!-- WiFi -->
  <div id="tab-wifi" style="display:none">
    <div class="card">
      <div class="card-title">Zugangspunkt</div>
      <div class="row"><label>Ger&auml;tename (TBS Agent)</label><input type="text" id="inp-dnam" maxlength="23" placeholder="MultiSwitch"></div>
      <div class="row"><label>SSID</label><input type="text" id="inp-ssid" maxlength="32"></div>
      <div class="row"><label>Passwort</label><input type="text" id="inp-pass" maxlength="63"></div>
      <div class="row"><label>IP-Adresse</label>
        <input type="text" id="inp-ip" maxlength="15" placeholder="192.168.1.1"
          pattern="^((25[0-5]|2[0-4]\d|[01]?\d\d?)\.){3}(25[0-5]|2[0-4]\d|[01]?\d\d?)$">
      </div>
      <button class="btn btn-primary" style="margin-top:8px" onclick="saveWifi()">Speichern &amp; Neustart</button>
    </div>
    <div class="card">
      <div class="card-title">WLAN dauerhaft aktiv (empfohlen, NEU v2.02)</div>
      <div class="row"><label>Dauerhaft an</label>
        <select id="sel-wifi-always" onchange="onWifiAlwaysOnChange()">
          <option value="0">Aus</option>
          <option value="1">Ein</option>
        </select>
      </div>
      <span style="color:var(--sub);font-size:12px;display:block">Schaltet den Access Point beim Booten immer ein und nie wieder aus - unabh&auml;ngig vom Bootpin (GPIO13) und vom Auto-Failsafe unten. Wirkt erst nach dem n&auml;chsten Neustart. Default AN.</span>
    </div>
    <div class="card">
      <div class="card-title">WLAN jetzt (NEU v2.00)</div>
      <div class="row"><label>Access Point</label>
        <span id="wifi-ap-state" style="font-weight:600">–</span>
        <button class="btn btn-ghost btn-sm" onclick="toggleWifi(true)">Einschalten</button>
        <button class="btn btn-ghost btn-sm" onclick="toggleWifi(false)">Ausschalten</button>
      </div>
      <div class="row"><label>Auto bei Signalverlust</label>
        <select id="sel-wifi-auto" onchange="saveWifiAuto()">
          <option value="0">Aus</option>
          <option value="1">Ein</option>
        </select>
      </div>
      <div class="row"><label>Timeout (s)</label>
        <input type="number" id="inp-wifi-auto-to" min="5" max="240" step="1" onchange="saveWifiAuto()">
      </div>
      <span style="color:var(--sub);font-size:12px;display:block">Schaltet den Access Point automatisch ein, wenn so lange kein g&uuml;ltiges RC-Signal anliegt (z.&nbsp;B. Sender aus). Schaltet nie automatisch wieder aus.</span>
      <span id="wifi-auto-overridden-hint" style="color:var(--yellow);font-size:12px;display:none;margin-top:4px">&#9888; Wirkungslos, solange "WLAN dauerhaft aktiv" oben eingeschaltet ist - der AP l&auml;uft dann bereits seit dem Booten.</span>
    </div>
    <div class="card">
      <div class="card-title">Konfiguration sichern/wiederherstellen (NEU v2.00)</div>
      <div style="display:flex;gap:8px;flex-wrap:wrap;margin-bottom:8px">
        <button class="btn btn-ghost btn-sm" onclick="exportConfig()">&#8681; Als Datei exportieren</button>
        <button class="btn btn-ghost btn-sm" onclick="document.getElementById('cfg-import-file').click()">&#8679; Aus Datei importieren</button>
        <input type="file" id="cfg-import-file" accept="application/json" style="display:none" onchange="importConfigFile(this.files[0])">
      </div>
      <span style="color:var(--sub);font-size:12px;display:block">Sichert alle Ausg&auml;nge und Einstellungen als JSON-Datei im Browser (kein SD-Slot n&ouml;tig). Import ersetzt die aktuelle Konfiguration inkl. WLAN-Zugangsdaten.</span>
    </div>
    <div class="card" id="c-ota">
      <div class="card-title">Firmware-Update (OTA, NEU v2.00)</div>
      <span style="color:var(--sub);font-size:12px;display:block;margin-bottom:8px">Spielt eine mit der Arduino-IDE gebaute .bin-Datei direkt per WLAN auf – Ausbauen und USB-Flashen ist damit nicht mehr n&ouml;tig. Setzt eine OTA-f&auml;hige Partitionstabelle voraus (siehe README).</span>
      <button class="btn btn-ghost btn-sm" onclick="document.getElementById('ota-file').click()">&#8679; Firmware-Datei w&auml;hlen (.bin)</button>
      <input type="file" id="ota-file" accept=".bin,application/octet-stream" style="display:none" onchange="uploadOtaFile(this.files[0])">
      <div style="margin-top:8px;font-size:12px" id="ota-status"></div>
      <span style="color:var(--yellow);font-size:12px;display:block;margin-top:8px">&#9888; W&auml;hrend des Uploads die Spannungsversorgung des Moduls NICHT unterbrechen. Update m&ouml;glichst bei &uuml;ber ESC/Akku bestromtem Modul durchf&uuml;hren, nicht nur &uuml;ber USB – manche USB-Anschl&uuml;sse liefern beim Flash-Schreiben nicht genug Strom, ein dadurch ausgel&ouml;ster Reset bricht den Upload ab. Nach Erfolg startet das Modul automatisch neu, die WLAN-Verbindung bricht dabei kurz ab – das ist normal.</span>
    </div>
    <div class="card">
      <div class="card-title">Werkseinstellungen</div>
      <button class="btn btn-danger" onclick="if(confirm('Alles zurücksetzen?'))doReset()">Zurücksetzen</button>
    </div>
  </div>

  <!-- Debug -->
  <div id="tab-debug" style="display:none">
    <div class="card">
      <div class="card-title">Live-Status</div>
      <div id="debug-body" class="mono" style="line-height:1.8"></div>
    </div>
    <div class="card">
      <div class="card-title">CRSF-Diagnose (NEU v2.00)</div>
      <div id="crsf-diag-body" class="mono" style="line-height:1.8"></div>
    </div>
    <div class="card">
      <div class="card-title">MWprop-Kanalwerte</div>
      <div id="prop-body" class="mono" style="line-height:1.8"></div>
    </div>
  </div>
</main>

<script>
let state={},curOut=-1,pollTimer=null;
const $ = id => document.getElementById(id);

function showTab(t){
  ['overview','outputs','rc','wifi','debug'].forEach(n=>{
    $('tab-'+n).style.display=(n===t?'block':'none');
  });
  document.querySelectorAll('.tab').forEach((el,i)=>{
    el.classList.toggle('active',['overview','outputs','rc','wifi','debug'][i]===t);
  });
  if(t==='debug') startPoll(); else stopPoll();
  if(t==='overview'||t==='outputs'||t==='wifi') fetchStatus();
  if(t==='rc'||t==='wifi') fetchConfig();
}

async function api(path,opts={}){
  try{const r=await fetch(path,opts);return r.ok?r.json():null;}catch{return null;}
}

async function fetchStatus(){
  const d=await api('/api/status');
  if(!d)return;
  state=d;
  const dot=$('dot');
  dot.className='dot '+(d.bus_ok?'ok':'err');
  $('hv').textContent=(d.version||'v?')+' · '+d.rc_system_name+(d.bus_ok?' · Signal OK':' · Kein Signal');
  const apEl=$('wifi-ap-state');
  if(apEl) apEl.textContent=d.wifi_ap_active?'an':'aus';
  renderTiles(d);
  if(curOut>=0) renderOutDetail(d,curOut);
  // Kanalwerte auf Startseite
  if(d.channels && d.bus_ok){
    $('card-channels').style.display='';
    $('ch-grid').innerHTML=d.channels.map((v,i)=>
      `<div style="background:var(--surf2);border:1px solid var(--border);border-radius:4px;padding:4px 6px">
       <span style="color:var(--sub)">CH${i+1}</span> ${v}</div>`
    ).join('');
  } else {
    $('card-channels').style.display='none';
  }
}

function renderTiles(d){
  const g=$('tile-grid');
  g.innerHTML='';
  d.outputs.forEach((o,i)=>{
    const t=document.createElement('div');
    t.className='tile'+(o.on?' on':'')+(o.override?' override':'');
    t.innerHTML=
      `<div class="tile-name" style="cursor:pointer" onclick="showOutTab(${i})">${o.name}</div>`+
      `<div class="tile-sub">${o.on
        ?'<span class="badge badge-ok">AN</span>'
        :'<span class="badge badge-err">AUS</span>'}</div>`+
      (o.override?'<span class="lock">🔒</span>':'')+
      `<div class="tile-btns">`+
      `<button class="tile-btn-on"  onclick="event.stopPropagation();tileSwitch(${i},true)">AN</button>`+
      `<button class="tile-btn-off" onclick="event.stopPropagation();tileSwitch(${i},false)">AUS</button>`+
      `<button class="tile-btn-rel" onclick="event.stopPropagation();tileRelease(${i})">Frei</button>`+
      `<button class="tile-btn-cfg" onclick="event.stopPropagation();showOutTab(${i})">Cfg</button>`+
      `</div>`;
    g.appendChild(t);
  });
}

async function tileSwitch(i,on){
  await api('/api/switch',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({ch:i,on})});
  fetchStatus();
}
async function tileRelease(i){
  await api('/api/switch',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({ch:i,release:true})});
  fetchStatus();
}
function showOutTab(i){
  showTab('outputs');
  showOut(i);
}

function showOutList(){
  $('out-list').style.display='';
  $('out-detail').style.display='none';
  curOut=-1;
}
function showOut(i){
  curOut=i;
  $('out-list').style.display='none';
  $('out-detail').style.display='block';
  if(state.outputs) renderOutDetail(state,i);
  fetchStatus();
}
// NEU V3.10: Kanal-Quelle-Dropdown fuer einen Slot (1/2/3) aufbauen. Slot 2/3
// bekommen zusaetzlich die Option "Nicht verwendet" (255) - Konfig1 ist immer
// aktiv, Konfig2/3 sind optional (siehe evalSlotActive() in der .ino: k=255
// matched keinen der Bereiche und ist damit automatisch "nie aktiv").
function kanalOptionsHtml(slot,val){
  const kGrp=[{l:'Einzelkanal',v:Array.from({length:8},(_,n)=>n)},
              {l:'Kanal_L (<800)',v:Array.from({length:16},(_,n)=>20+n)},
              {l:'Kanal_H (>1200)',v:Array.from({length:16},(_,n)=>40+n)},
              {l:'MKan1',v:Array.from({length:8},(_,n)=>80+n)},   // NEU V3.00
              {l:'MKan2',v:Array.from({length:8},(_,n)=>88+n)}];  // NEU V3.00
  let h=`<select id="sel-kanal${slot}" style="width:100%" onchange="saveOutKanal(${slot})">`;
  if(slot>1){
    h+=`<optgroup label="Optionen"><option value="255"${val===255?' selected':''}>Nicht verwendet</option></optgroup>`;
  }
  kGrp.forEach(g=>{
    h+=`<optgroup label="${g.l}">`;
    // NEU V3.00: Nummerierung ueber die Position IN der Gruppe (idx), nicht
    // mehr ueber v%20 - das war nur zufaellig korrekt, solange alle Gruppen
    // bei einem Vielfachen von 20 anfingen (MKan2 beginnt bei 88).
    g.v.forEach((v,idx)=>{
      const n=(idx+1).toString().padStart(2,'0');
      h+=`<option value="${v}"${val===v?' selected':''}>${g.l.split(' ')[0]} ${n}</option>`;
    });
    h+='</optgroup>';
  });
  h+='</select>';
  return h;
}

// NEU V3.10: kompletten Konfig-Block (Kanal/Blink/PWM) fuer einen Slot
// (1/2/3) eines Ausgangs aufbauen - Slot 1 = bisheriges Verhalten (immer
// aktiv), Slot 2/3 = zusaetzliche, optionale Konfigurationen mit eigener
// Kanal-Quelle. Prioritaet bei mehreren aktiven Slots: 1 > 2 > 3 (siehe
// Output() in der .ino).
function buildSlotConfigHtml(slot,i,kanalVal,modeVal,pwmVal,isCrsf){
  const kHtml=kanalOptionsHtml(slot,kanalVal);

  const bv=[[0,'Dauerlicht'],[1,'0,01s'],[2,'0,02s'],[3,'0,03s'],[5,'0,05s'],
    [10,'0,1s'],[25,'0,25s'],[50,'0,5s'],[75,'0,75s'],[100,'1s'],
    [150,'1,5s'],[200,'2s'],[210,'2,1s'],[220,'2,2s'],[230,'2,3s'],[240,'2,4s'],[250,'2,5s']];
  const mInt=modeVal||0;
  const mH=(mInt>>8)&0xFF, mL=mInt&0xFF;
  let mAnHtml=`<select id="sel-mode-an${slot}" style="width:100%" onchange="saveOutMode(${slot})">`;
  bv.forEach(([v,l])=>mAnHtml+=`<option value="${v}"${mH===v?' selected':''}>${v===0?l:'Blinken '+l+' an'}</option>`);
  mAnHtml+='</select>';
  let mAusRow='';
  if(mH>0){
    let h=`<select id="sel-mode-aus${slot}" style="width:100%" onchange="saveOutMode(${slot})">`;
    bv.filter(([v])=>v>0).forEach(([v,l])=>h+=`<option value="${v}"${mL===v?' selected':''}>${'Blinken '+l+' aus'}</option>`);
    h+='</select>';
    mAusRow=`<div class="row"><label>Blink AUS-Zeit</label>${h}</div>`;
  }

  // PWM-Quelle: MWprop nur bei CRSF
  const pwm=pwmVal;
  const isMW=(pwm>=200&&pwm<=207);
  const isFest=!isMW;
  let pHtml=`<select id="sel-pwm-src${slot}" style="width:100%" onchange="saveOutPwmSrc(${slot})">`;
  pHtml+=`<option value="255"${isFest&&pwm===255?' selected':''}>Festwert (0-255)</option>`;
  if(isCrsf){
    pHtml+=`<option value="${200+i}"${isMW?' selected':''}>MWprop (Kanal=${i+1}, auto)</option>`;
  }
  pHtml+='</select>';

  // PWM-Slider (bei Festwert)
  const sv=isFest?pwm:255;
  const slRow=isFest?`<div class="row"><label>PWM-Wert: <b id="pwm-val${slot}">${sv}</b></label>
    <input type="range" min="0" max="255" step="5" value="${sv}" style="width:100%;margin-top:4px"
      oninput="$('pwm-val${slot}').textContent=this.value" onchange="saveOutPwm(${slot},this.value)"></div>`:'';

  return `
    <div class="row"><label>Kanal-Quelle</label>${kHtml}</div>
    <div class="row"><label>Blink AN-Zeit</label>${mAnHtml}</div>
    ${mAusRow}
    <div class="row"><label>PWM-Quelle</label>${pHtml}</div>
    ${slRow}`;
}

function renderOutDetail(d,i){
  const o=d.outputs[i];
  $('out-title').textContent=o.name;
  const sb=$('out-state-big');
  sb.textContent=o.on?'AN':'AUS';
  sb.className='state-big '+(o.on?'state-on':'state-off');
  const isCrsf=(state&&state.rc_system_boot===4);

  // NEU V3.10: bis zu 3 Konfigurationen pro Ausgang, Prioritaet 1>2>3 -
  // Konfig1 = bisheriges kanal/mode/pwm, Konfig2/3 = kanal2/mode2/pwm2 bzw.
  // kanal3/mode3/pwm3 (jeweils optional, Default "Nicht verwendet"=255).
  const slot1Html=buildSlotConfigHtml(1,i,o.kanal,o.mode,o.pwm,isCrsf);
  const slot2Html=buildSlotConfigHtml(2,i,o.kanal2,o.mode2,o.pwm2,isCrsf);
  const slot3Html=buildSlotConfigHtml(3,i,o.kanal3,o.mode3,o.pwm3,isCrsf);

  $('out-cfg').innerHTML=`
    <div class="card-title">Konfig 1 – ${o.name} (h&ouml;chste Priorit&auml;t)</div>
    ${slot1Html}
    <hr class="sep">
    <div class="card-title">Konfig 2 (optional)</div>
    ${slot2Html}
    <hr class="sep">
    <div class="card-title">Konfig 3 (optional, niedrigste Priorit&auml;t)</div>
    ${slot3Html}
    <hr class="sep">
    <div class="row"><label>Override</label><span>${o.override?'<b>Aktiv</b>':'Inaktiv'}</span></div>
    <div class="row" style="margin-top:6px">
      <button class="btn btn-primary" onclick="saveOutName()">Name ändern</button>
    </div>`;
}
// NEU V3.10: slot 1/2/3 -> jeweils eigenes JSON-Feld (kanal/kanal2/kanal3)
function slotField(base,slot){ return slot===1?base:base+slot; }
async function saveOutKanal(slot){
  if(curOut<0)return;
  const body={ch:curOut};
  body[slotField('kanal',slot)]=parseInt($('sel-kanal'+slot).value);
  await api('/api/output',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify(body)});
  fetchStatus();
}
async function saveOutMode(slot){
  if(curOut<0)return;
  const an=parseInt($('sel-mode-an'+slot).value);
  const ausEl=$('sel-mode-aus'+slot);
  const aus=ausEl?parseInt(ausEl.value):an;
  const body={ch:curOut};
  body[slotField('mode',slot)]=(an<<8)|aus;
  await api('/api/output',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify(body)});
  fetchStatus();
}
async function saveOutPwmSrc(slot){
  if(curOut<0)return;
  const body={ch:curOut};
  body[slotField('pwm',slot)]=parseInt($('sel-pwm-src'+slot).value);
  await api('/api/output',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify(body)});
  fetchStatus();
}
async function saveOutPwm(slot,val){
  if(curOut<0)return;
  const body={ch:curOut};
  body[slotField('pwm',slot)]=parseInt(val);
  await api('/api/output',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify(body)});
}
async function saveOutName(){
  if(curOut<0)return;
  const nm=prompt('Neuer Name (max. 16 Zeichen):',state.outputs[curOut].name);
  if(!nm)return;
  await api('/api/output',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({ch:curOut,name:nm.substring(0,16)})});
  fetchStatus();
}

async function switchOut(on){
  if(curOut<0)return;
  await api('/api/switch',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({ch:curOut,on})});
  fetchStatus();
}
async function releaseOut(){
  if(curOut<0)return;
  await api('/api/switch',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({ch:curOut,release:true})});
  fetchStatus();
}
async function releaseAll(){
  for(let i=0;i<8;i++)
    await api('/api/switch',{method:'POST',headers:{'Content-Type':'application/json'},
      body:JSON.stringify({ch:i,release:true})});
  fetchStatus();
}

async function fetchConfig(){
  const d=await api('/api/config');
  if(!d)return;
  $('sel-rc').value=d.rc_system;
  // SBUS: Einkanal-Kanal (crsf_channel wird fuer SBUS als Einkanal-Kanal genutzt)
  // Wenn einkanal_mode==999 -> Deaktiviert im Kanal-Dropdown
  const ek=parseInt(d.einkanal_mode);
  $('sel-sbus-ch').value=(ek===999)?999:parseInt(d.crsf_channel);
  $('sel-ek').value=(ek===999)?0:ek;
  // CRSF: Modul-Adresse
  $('sel-adr').value=d.modul_adress;
  // NEU V3.00: MKan-Gruppen
  $('sel-mkan-adr0').value=d.ek_gadr0;
  $('sel-mkan-adr1').value=d.ek_gadr1;
  $('sel-mkan-ch0').value=d.sbus_gch0;
  $('sel-mkan-mode0').value=d.sbus_gmode0;
  $('sel-mkan-ch1').value=d.sbus_gch1;
  $('sel-mkan-mode1').value=d.sbus_gmode1;
  $('inp-dnam').value=d.dnam;
  $('inp-ssid').value=d.ssid;
  $('inp-pass').value=d.pass;
  $('inp-ip').value=d.ip;
  $('sel-wifi-auto').value=d.wifi_auto?1:0;
  $('inp-wifi-auto-to').value=d.wifi_auto_timeout;
  $('sel-wifi-always').value=d.wifi_always_on?1:0;
  updateWifiAlwaysOnHint();
  updateRcRows(d.rc_system, ek);
  window._rcBoot=parseInt(d.rc_system);
}

// Sichtbarkeit der Rows je nach RC-System und Einkanal-Status
// RC-Sound Logik: CRSF -> nur Modul-Adresse; SBUS -> Kanal + (wenn aktiv) Modus
function updateRcRows(rc, ek){
  const isCrsf=(parseInt(rc)===4);
  const ekAktiv=(ek!==999);
  $('row-sbus-ch').style.display=isCrsf?'none':'';
  $('row-ek').style.display=(isCrsf||!ekAktiv)?'none':'';
  $('row-adr').style.display=isCrsf?'':'none';
  // NEU V3.00: MKan-Gruppen - Adresse bei CRSF, Kanal+Modus bei SBUS
  $('row-mkan-adr0').style.display=isCrsf?'':'none';
  $('row-mkan-adr1').style.display=isCrsf?'':'none';
  $('row-mkan-ch0').style.display=isCrsf?'none':'';
  $('row-mkan-mode0').style.display=isCrsf?'none':'';
  $('row-mkan-ch1').style.display=isCrsf?'none':'';
  $('row-mkan-mode1').style.display=isCrsf?'none':'';
}

// RC-System Wechsel: sofort speichern, bei SBUS<->CRSF Wechsel: Neustart
async function onRcChange(){
  const newRc=parseInt($('sel-rc').value);
  const curEk=parseInt($('sel-ek').value)||0;
  updateRcRows(newRc, $('sel-sbus-ch').value==999?999:curEk);
  const oldRc=window._rcBoot||newRc;
  const needsRestart=(newRc===4)!==(oldRc===4);
  await api('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({rc_system:newRc})});
  const hint=$('rc-hint');
  if(needsRestart){
    api('/api/restart',{method:'POST'});
    let s=5; hint.style.color='var(--yellow)';
    const t=setInterval(()=>{
      hint.textContent='Neustart… Verbinde in '+s+'s';
      if(--s<0){clearInterval(t);window.location.reload();}
    },1000);
  } else {
    hint.textContent='Gespeichert.'; hint.style.color='var(--green)';
    setTimeout(()=>{hint.textContent='';},2000);
  }
}

// SBUS Einkanal-Kanal: speichert crsf_channel; Deaktiviert -> einkanal_mode=999
async function onSbusChChange(){
  const v=parseInt($('sel-sbus-ch').value);
  const deakt=(v===999);
  // Einkanal-Modus Row ein/ausblenden
  $('row-ek').style.display=deakt?'none':'';
  if(deakt){
    await api('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},
      body:JSON.stringify({einkanal_mode:999,crsf_channel:parseInt($('sel-sbus-ch').value)})});
  } else {
    // Setzt crsf_channel auf gewaehlten SBUS Kanal; einkanal_mode bleibt
    await api('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},
      body:JSON.stringify({crsf_channel:v,einkanal_mode:parseInt($('sel-ek').value)||0})});
  }
}

// Einkanal-Modus (Normal / WM Adr 0-3)
async function onEkChange(){
  await api('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({einkanal_mode:parseInt($('sel-ek').value)})});
}

// CRSF Modul-Adresse
async function onAdrChange(){
  await api('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({modul_adress:parseInt($('sel-adr').value)})});
}
// NEU V3.00: MKan-Gruppe g (0 oder 1) - Adresse (CRSF) bzw. Kanal+Modus (SBUS)
async function onMkanChange(g){
  const isCrsf=(parseInt($('sel-rc').value)===4);
  const body={};
  if(isCrsf){
    body['ek_gadr'+g]=parseInt($('sel-mkan-adr'+g).value);
  } else {
    body['sbus_gch'+g]=parseInt($('sel-mkan-ch'+g).value);
    body['sbus_gmode'+g]=parseInt($('sel-mkan-mode'+g).value);
  }
  await api('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify(body)});
}
async function saveRc(){} // nicht mehr per Button benoetigt
async function saveWifi(){
  const ip=$('inp-ip').value.trim();
  await api('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({dnam:$('inp-dnam').value,ssid:$('inp-ssid').value,pass:$('inp-pass').value,ip:ip})});
  api('/api/restart',{method:'POST'});
  let s=5;
  const t=setInterval(()=>{
    if(--s<0){clearInterval(t);window.location.href='http://'+ip;}
  },1000);
  alert('WiFi gespeichert. Verbinde in 5s mit '+ip+'…');
}
async function doReset(){
  await api('/api/reset',{method:'POST'});
  api('/api/restart',{method:'POST'});
  let s=5;
  const t=setInterval(()=>{ if(--s<0){clearInterval(t);window.location.reload();} },1000);
  alert('Zurückgesetzt. Neustart in 5s…');
}

// ─── WLAN jetzt (NEU v2.00) ────────────────────────────────────────────
// Wirkt sofort, ohne Neustart und ohne die Konfiguration zu speichern
// (siehe handleWifi() in webui.h) - unabhängig vom Auto-Failsafe unten.
async function toggleWifi(enable){
  await api('/api/wifi',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({enable})});
  fetchStatus();
}
async function saveWifiAuto(){
  await api('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({
      wifi_auto:$('sel-wifi-auto').value==='1',
      wifi_auto_timeout:parseInt($('inp-wifi-auto-to').value)||60
    })});
}

// ─── WLAN dauerhaft aktiv (NEU v2.02) ──────────────────────────────────
// Rein persistierend, wirkt erst ab dem naechsten Neustart (siehe
// webui_init()/wifiFailsafeCheck() in der .ino) - bewusst kein sofortiges
// toggleWifi() hier, analog zum Soundmodul-Projekt seit dessen v7.15.
async function onWifiAlwaysOnChange(){
  await api('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({wifi_always_on:$('sel-wifi-always').value==='1'})});
  updateWifiAlwaysOnHint();
}
function updateWifiAlwaysOnHint(){
  $('wifi-auto-overridden-hint').style.display=($('sel-wifi-always').value==='1')?'block':'none';
}

// ─── Konfiguration Export/Import (NEU v2.00) ──────────────────────────
// Export: liest /api/config (globale Einstellungen) + /api/status (Ausgänge)
// und bietet das Ergebnis als Datei zum Download an - ganz normaler
// Browser-Download, da der Webserver direkt auf dem Modul läuft. Kein SD-
// Slot nötig/vorhanden - bewusst rein Browser-basiert.
async function exportConfig(){
  const cfg=await api('/api/config');
  const st=await api('/api/status');
  if(!cfg||!st){ alert('Export fehlgeschlagen (Gerät nicht erreichbar).'); return; }
  const data={
    device:'ESP32-MultiSwitch', version:st.version||'', exported:new Date().toISOString(),
    settings:cfg,
    outputs:st.outputs.map((o,i)=>({ch:i,name:o.name,kanal:o.kanal,pwm:o.pwm,mode:o.mode,
      kanal2:o.kanal2,pwm2:o.pwm2,mode2:o.mode2,     // NEU V3.10
      kanal3:o.kanal3,pwm3:o.pwm3,mode3:o.mode3}))   // NEU V3.10
  };
  const text=JSON.stringify(data,null,2);
  let name='multiswitch-config';
  if(cfg.dnam) name+='-'+String(cfg.dnam).replace(/[^A-Za-z0-9_-]+/g,'_');
  name+='.json';
  const blob=new Blob([text],{type:'application/json'});
  const url=URL.createObjectURL(blob);
  const a=document.createElement('a');
  a.href=url; a.download=name;
  document.body.appendChild(a); a.click(); document.body.removeChild(a);
  setTimeout(()=>URL.revokeObjectURL(url),1000);
}

// Import: Datei einlesen, grob validieren, bestätigen lassen (WLAN-
// Zugangsdaten werden überschrieben), dann Schritt für Schritt über die
// bestehenden POST-Endpunkte einspielen und zum Schluss ohne Neustart
// speichern (/api/save).
function importConfigFile(file){
  if(!file) return;
  const reader=new FileReader();
  reader.onload=function(){
    let data;
    try{ data=JSON.parse(reader.result); }catch(e){ alert('Ungültige JSON-Datei.'); return; }
    if(!data||!data.settings||!Array.isArray(data.outputs)){
      alert('Datei enthält keine gültige MultiSwitch-Konfiguration.'); return;
    }
    if(!confirm('Aktuelle Konfiguration (inkl. WLAN-Zugangsdaten) durch die Datei ersetzen?')) return;
    importConfigData(data);
  };
  reader.onerror=function(){ alert('Datei konnte nicht gelesen werden.'); };
  reader.readAsText(file);
  // Sonst laesst sich dieselbe Datei kein zweites Mal auswaehlen (onchange
  // feuert bei unveraendertem value nicht erneut).
  $('cfg-import-file').value='';
}
async function importConfigData(data){
  const s=data.settings||{};
  await api('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({
      rc_system:s.rc_system, crsf_channel:s.crsf_channel, einkanal_mode:s.einkanal_mode,
      modul_adress:s.modul_adress, dnam:s.dnam, ssid:s.ssid, pass:s.pass, ip:s.ip,
      wifi_auto:s.wifi_auto, wifi_auto_timeout:s.wifi_auto_timeout,
      wifi_always_on:s.wifi_always_on
    })});
  for(const o of (data.outputs||[])){
    if(o.ch===undefined) continue;
    await api('/api/output',{method:'POST',headers:{'Content-Type':'application/json'},
      body:JSON.stringify({ch:o.ch,kanal:o.kanal,pwm:o.pwm,mode:o.mode,name:o.name,
        kanal2:o.kanal2,pwm2:o.pwm2,mode2:o.mode2,     // NEU V3.10
        kanal3:o.kanal3,pwm3:o.pwm3,mode3:o.mode3})}); // NEU V3.10
  }
  await api('/api/save',{method:'POST'});
  alert('Konfiguration importiert. Seite wird neu geladen…');
  window.location.reload();
}

// ─── Firmware-Update per WLAN (OTA, NEU v2.00) ─────────────────────────
// Echtes multipart/form-data an /api/otaupdate (siehe handleOtaUpdateData()/
// handleOtaUpdate() in webui.h) - Fortschritt per XHR-upload.onprogress,
// da fetch() das (noch) nicht zuverlaessig unterstuetzt. Bei Erfolg startet
// das Modul selbst neu; die Antwort trifft trotzdem noch ein, bevor die
// Verbindung tatsaechlich abbricht.
function uploadOtaFile(file){
  if(!file) return;
  if(!confirm('Firmware-Update mit "'+file.name+'" starten? Waehrend des '+
              'Uploads die Spannungsversorgung des Moduls NICHT unterbrechen '+
              '- moeglichst ueber ESC/Akku bestromt, nicht nur ueber USB. '+
              'Das Modul startet danach automatisch neu.')){
    $('ota-file').value=''; return;
  }
  const status=$('ota-status');
  status.textContent='Hochladen … (0%)';

  const fd=new FormData();
  fd.append('file',file,file.name);

  const x=new XMLHttpRequest();
  x.open('POST','/api/otaupdate',true);
  x.upload.onprogress=function(e){
    if(e.lengthComputable) status.textContent='Hochladen … ('+Math.round(e.loaded/e.total*100)+'%)';
  };
  x.onload=function(){
    try{
      const d=JSON.parse(x.responseText);
      if(d.ok){
        status.textContent='Update erfolgreich ('+d.size+' Bytes) – Modul startet neu …';
      } else {
        status.textContent='Fehler ('+d.error+')';
        alert('Update fehlgeschlagen ('+d.error+').');
      }
    }catch(e){ status.textContent='Fehler beim Hochladen'; alert('Update fehlgeschlagen.'); }
  };
  x.onerror=function(){
    // Kann auch bedeuten, dass das Update erfolgreich war und das Modul
    // bereits neu startet, bevor die Antwort ankam - fuer den Nutzer nicht
    // unterscheidbar von einem echten Verbindungsfehler.
    status.textContent='Verbindung unterbrochen – Update war evtl. trotzdem erfolgreich (neu verbinden und prüfen).';
  };
  x.send(fd);
  $('ota-file').value='';
}

function startPoll(){pollTimer=setInterval(pollDebug,800);}
function stopPoll(){clearInterval(pollTimer);}
async function pollDebug(){
  const d=await api('/api/status');
  if(!d)return;
  let html='';
  html+=`BUS_OK: <b>${d.bus_ok?'JA':'NEIN'}</b> | RC: ${d.rc_system_name}<br>`;
  d.outputs.forEach((o,i)=>{
    html+=`Ausgang ${i+1} (${o.name}): ${o.on?'<b>AN</b>':'AUS'}`;
    if(o.override)html+=' 🔒';
    html+='<br>';
  });
  html+=`Einkanal-Data: 0x${d.einkanal_data_hex}<br>`;
  html+=`Kanäle: ${d.channels.map((v,i)=>`ch${i+1}=${v}`).join(' ')}<br>`;
  html+=`WLAN-AP: ${d.wifi_ap_active?'<b>an</b>':'aus'}<br>`;
  $('debug-body').innerHTML=html;

  // CRSF-Diagnose (NEU v2.00) - bei SBUS bleiben die Zaehler auf 0
  if(d.crsf_diag){
    const c=d.crsf_diag;
    let ch='';
    ch+=`Rohe Bytes: ${c.raw_bytes}<br>`;
    ch+=`Gültige Frames: ${c.valid_frames}<br>`;
    ch+=`CRC-Fehler: ${c.crc_errors}<br>`;
    ch+=`DEVICE_PING empfangen: ${c.device_pings}<br>`;
    ch+=`PARAMETER_READ empfangen: ${c.param_reads}<br>`;
    ch+=`PARAMETER_WRITE empfangen: ${c.param_writes}<br>`;
    $('crsf-diag-body').innerHTML=ch;
  } else {
    $('crsf-diag-body').innerHTML='Keine Daten';
  }

  // MWprop
  let ph='';
  if(d.wm_prop){
    d.wm_prop.forEach((v,i)=>{ ph+=`Prop ${i+1}: ${v} (${Math.round(v/255*100)}%)<br>`; });
  }
  $('prop-body').innerHTML=ph||'Keine Daten';
}

// Init
fetchStatus();
showOutList();
</script>
</body>
</html>)HTML";

// ============================================================
//  JSON-Helfer
// ============================================================
static void sendJson(const String& json) {
    webServer.sendHeader("Cache-Control","no-cache");
    webServer.send(200, "application/json", json);
}
static void sendOk()  { sendJson("{\"ok\":true}"); }
static void send404() { webServer.send(404,"application/json","{\"error\":\"not found\"}"); }

static String jsonEscape(const char* s) {
  String out;
  if (!s) return out;
  while (*s) {
    switch (*s) {
      case '\\': out += "\\\\"; break;
      case '"':  out += "\\\""; break;
      case '\b': out += "\\b"; break;
      case '\f': out += "\\f"; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default: out += *s; break;
    }
    s++;
  }
  return out;
}

static bool jsonExtractInt(const String& body, const char* key, int& out) {
  String token = "\"" + String(key) + "\"";
  int p = body.indexOf(token);
  if (p < 0) return false;
  p = body.indexOf(':', p + token.length());
  if (p < 0) return false;
  p++;
  while (p < (int)body.length() && isspace((unsigned char)body[p])) p++;
  char* endPtr = nullptr;
  long v = strtol(body.c_str() + p, &endPtr, 10);
  if (endPtr == body.c_str() + p) return false;
  out = (int)v;
  return true;
}

static bool jsonExtractBool(const String& body, const char* key, bool& out) {
  String token = "\"" + String(key) + "\"";
  int p = body.indexOf(token);
  if (p < 0) return false;
  p = body.indexOf(':', p + token.length());
  if (p < 0) return false;
  p++;
  while (p < (int)body.length() && isspace((unsigned char)body[p])) p++;
  if (body.startsWith("true", p)) {
    out = true;
    return true;
  }
  if (body.startsWith("false", p)) {
    out = false;
    return true;
  }
  return false;
}

static bool jsonExtractString(const String& body, const char* key, String& out) {
  String token = "\"" + String(key) + "\"";
  int p = body.indexOf(token);
  if (p < 0) return false;
  p = body.indexOf(':', p + token.length());
  if (p < 0) return false;
  p++;
  while (p < (int)body.length() && isspace((unsigned char)body[p])) p++;
  if (p >= (int)body.length() || body[p] != '"') return false;
  p++;
  out = "";
  bool esc = false;
  while (p < (int)body.length()) {
    char c = body[p++];
    if (esc) {
      switch (c) {
        case '"': out += '"'; break;
        case '\\': out += '\\'; break;
        case '/': out += '/'; break;
        case 'b': out += '\b'; break;
        case 'f': out += '\f'; break;
        case 'n': out += '\n'; break;
        case 'r': out += '\r'; break;
        case 't': out += '\t'; break;
        default: out += c; break;
      }
      esc = false;
      continue;
    }
    if (c == '\\') { esc = true; continue; }
    if (c == '"') return true;
    out += c;
  }
  return false;
}

// ============================================================
//  RC-System-Name
// ============================================================
static const char* rcName(int sys) {
    switch(sys) {
        case 0: return "FrSky SBUS";
        case 1: return "FlySky SBUS";
        case 2: return "ELRS SBUS";
        case 3: return "HoTT SBUS";
        case 4: return "CRSF";
        default: return "Unbekannt";
    }
}

// ============================================================
//  API: /api/status
// ============================================================
static void handleStatus() {
    String j = "{";
    // Firmware-Version dynamisch aus Version-Konstante (z.B. 140 -> "v1.40")
    j += "\"version\":\"v" + String(Version / 100) + "." + String(Version % 100 < 10 ? "0" : "") + String(Version % 100) + "\",";
    j += "\"bus_ok\":" + String(BUS_OK ? "true" : "false") + ",";
    j += "\"rc_system\":" + String(RC_System_boot) + ",";
  j += "\"rc_system_name\":\"" + jsonEscape(rcName(RC_System_boot)) + "\",";
        j += "\"rc_system_boot\":" + String(RC_System_boot) + ",";

    // Einkanal
    char hex[8]; snprintf(hex, sizeof(hex), "%02X", einkanal_Data);
    j += "\"einkanal_data_hex\":\"" + String(hex) + "\",";

    // Ausgänge
    j += "\"outputs\":[";
    for (int i = 0; i < 8; i++) {
        if (i) j += ",";
        j += "{";
        j += "\"name\":\"" + jsonEscape(Ausgang_Name[i]) + "\",";
        j += "\"on\":"    + String(Ausgang[i] ? "true" : "false") + ",";
        j += "\"override\":" + String(g_manual_override[i] ? "true" : "false") + ",";
        j += "\"kanal\":" + String(Ausgang_Kanal[i]) + ",";
        j += "\"pwm\":"   + String(pwm_wert[i]) + ",";
        char mhex[6]; snprintf(mhex, sizeof(mhex), "0x%04X", mode[i]);
        j += "\"mode_hex\":\"" + String(mhex) + "\",";
        j += "\"mode\":" + String(mode[i]) + ",";
        // NEU V3.10: Konfig 2/3 pro Ausgang (optional, Prioritaet 1>2>3)
        j += "\"kanal2\":" + String(Ausgang_Kanal2[i]) + ",";
        j += "\"pwm2\":"   + String(pwm_wert2[i]) + ",";
        j += "\"mode2\":"  + String(mode2[i]) + ",";
        j += "\"kanal3\":" + String(Ausgang_Kanal3[i]) + ",";
        j += "\"pwm3\":"   + String(pwm_wert3[i]) + ",";
        j += "\"mode3\":"  + String(mode3[i]);
        j += "}";
    }
    j += "],";

    // Rohkanäle (erste 8)
    j += "\"channels\":[";
    for (int i = 0; i < 8; i++) { if(i)j+=","; j+=channel_output[i]; }
    j += "],";

    // MWprop-Werte (NEU v0.14)
    j += "\"wm_prop\":[";
    for (int i = 0; i < 8; i++) { if(i)j+=","; j+=wm_prop_value[i]; }
    j += "],";

    // CRSF-Diagnose-Zaehler (NEU v2.00) - bei SBUS bleiben sie auf 0
    j += "\"crsf_diag\":{";
    j += "\"raw_bytes\":"    + String(crsf.getRawBytesRx())  + ",";
    j += "\"valid_frames\":" + String(crsf.getValidFrames()) + ",";
    j += "\"crc_errors\":"   + String(crsf.getCrcErrors())   + ",";
    j += "\"device_pings\":" + String(crsf.getDevicePings()) + ",";
    j += "\"param_reads\":"  + String(crsf.getParamReads())  + ",";
    j += "\"param_writes\":" + String(crsf.getParamWrites());
    j += "},";
    j += "\"wifi_ap_active\":" + String(webui_isApActive() ? "true" : "false");

    j += "}";
    sendJson(j);
}

// ============================================================
//  API: /api/config (GET + POST)
// ============================================================
static void handleConfig() {
    if (webServer.method() == HTTP_GET) {
        String j = "{";
        j += "\"rc_system\":"     + String(RC_System)      + ",";
        j += "\"crsf_channel\":"  + String(CRSF_Channel)   + ",";
        j += "\"einkanal_mode\":" + String(einkanal_mode)  + ",";
        j += "\"modul_adress\":"  + String(modul_adress)   + ",";
        j += "\"ek_gadr0\":"    + String(EK_Gruppen_Adresse[0])   + ",";  // NEU V3.00
        j += "\"ek_gadr1\":"    + String(EK_Gruppen_Adresse[1])   + ",";  // NEU V3.00
        j += "\"sbus_gch0\":"   + String(SBUS_Gruppen_Channel[0]) + ",";  // NEU V3.00
        j += "\"sbus_gch1\":"   + String(SBUS_Gruppen_Channel[1]) + ",";  // NEU V3.00
        j += "\"sbus_gmode0\":" + String(SBUS_Gruppen_Mode[0])    + ",";  // NEU V3.00
        j += "\"sbus_gmode1\":" + String(SBUS_Gruppen_Mode[1])    + ",";  // NEU V3.00
      j += "\"dnam\":\""        + jsonEscape(g_device_name)  + "\",";
      j += "\"ssid\":\""        + jsonEscape(g_wifi_ssid)    + "\",";
      j += "\"pass\":\""        + jsonEscape(g_wifi_pass)    + "\",";
      j += "\"ip\":\""          + jsonEscape(g_wifi_ip)      + "\",";
        j += "\"wifi_auto\":"      + String(g_wifi_auto ? "true" : "false") + ",";
        j += "\"wifi_auto_timeout\":" + String(g_wifi_auto_timeout) + ",";
        j += "\"wifi_always_on\":" + String(g_wifi_always_on ? "true" : "false");
        j += "}";
        sendJson(j);
        return;
    }
    // POST
    if (!webServer.hasArg("plain")) { send404(); return; }
    String body = webServer.arg("plain");
    bool changed = false;

    int v = 0;
    if (jsonExtractInt(body, "rc_system", v)) {
      v = constrain(v, 0, 4);
      if (v != RC_System) { RC_System = v; changed = true; }
    }
    if (jsonExtractInt(body, "crsf_channel", v)) {
      if ((v >= 0 && v <= 15) || v == 999) {
        if (v != CRSF_Channel) { CRSF_Channel = v; changed = true; }
      }
    }
    if (jsonExtractInt(body, "einkanal_mode", v)) {
      bool valid = (v == 0 || v == 999 || (v >= 10 && v <= 13));
      if (valid && v != einkanal_mode) { einkanal_mode = v; changed = true; }
    }
    if (jsonExtractInt(body, "modul_adress", v)) {
      v = constrain(v, 0, 20);
      if (v != modul_adress) { modul_adress = v; changed = true; }
    }
    // NEU V3.00: MKan-Gruppen (CRSF-Adresse bzw. SBUS-Kanal/-Modus je Gruppe)
    if (jsonExtractInt(body, "ek_gadr0", v)) {
      v = (v == 255) ? 255 : constrain(v, 0, 20);
      if (v != EK_Gruppen_Adresse[0]) { EK_Gruppen_Adresse[0] = v; changed = true; }
    }
    if (jsonExtractInt(body, "ek_gadr1", v)) {
      v = (v == 255) ? 255 : constrain(v, 0, 20);
      if (v != EK_Gruppen_Adresse[1]) { EK_Gruppen_Adresse[1] = v; changed = true; }
    }
    if (jsonExtractInt(body, "sbus_gch0", v)) {
      v = (v == 999) ? 999 : constrain(v, 0, 15);
      if (v != SBUS_Gruppen_Channel[0]) { SBUS_Gruppen_Channel[0] = v; changed = true; }
    }
    if (jsonExtractInt(body, "sbus_gch1", v)) {
      v = (v == 999) ? 999 : constrain(v, 0, 15);
      if (v != SBUS_Gruppen_Channel[1]) { SBUS_Gruppen_Channel[1] = v; changed = true; }
    }
    if (jsonExtractInt(body, "sbus_gmode0", v)) {
      bool valid = (v == 0 || (v >= 10 && v <= 13));
      if (valid && v != SBUS_Gruppen_Mode[0]) { SBUS_Gruppen_Mode[0] = v; changed = true; }
    }
    if (jsonExtractInt(body, "sbus_gmode1", v)) {
      bool valid = (v == 0 || (v >= 10 && v <= 13));
      if (valid && v != SBUS_Gruppen_Mode[1]) { SBUS_Gruppen_Mode[1] = v; changed = true; }
    }

    // NEU v2.00: WLAN-Auto-Failsafe
    bool bv = false;
    if (jsonExtractBool(body, "wifi_auto", bv)) {
      if (bv != g_wifi_auto) { g_wifi_auto = bv; changed = true; }
    }
    if (jsonExtractInt(body, "wifi_auto_timeout", v)) {
      v = constrain(v, 5, 240);
      if ((uint16_t)v != g_wifi_auto_timeout) { g_wifi_auto_timeout = (uint16_t)v; changed = true; }
    }
    // NEU v2.02: WLAN dauerhaft an
    if (jsonExtractBool(body, "wifi_always_on", bv)) {
      if (bv != g_wifi_always_on) { g_wifi_always_on = bv; changed = true; }
    }

    String s;
    if (jsonExtractString(body, "dnam", s)) {
      s = s.substring(0, sizeof(g_device_name) - 1);
      if (strncmp(g_device_name, s.c_str(), sizeof(g_device_name) - 1) != 0) {
        strncpy(g_device_name, s.c_str(), sizeof(g_device_name) - 1);
        g_device_name[sizeof(g_device_name) - 1] = '\0';
        changed = true;
      }
    }
    if (jsonExtractString(body, "ssid", s)) {
      s = s.substring(0, sizeof(g_wifi_ssid) - 1);
      if (strncmp(g_wifi_ssid, s.c_str(), sizeof(g_wifi_ssid) - 1) != 0) {
        strncpy(g_wifi_ssid, s.c_str(), sizeof(g_wifi_ssid) - 1);
        g_wifi_ssid[sizeof(g_wifi_ssid) - 1] = '\0';
        changed = true;
      }
    }
    if (jsonExtractString(body, "pass", s)) {
      s = s.substring(0, sizeof(g_wifi_pass) - 1);
      if (strncmp(g_wifi_pass, s.c_str(), sizeof(g_wifi_pass) - 1) != 0) {
        strncpy(g_wifi_pass, s.c_str(), sizeof(g_wifi_pass) - 1);
        g_wifi_pass[sizeof(g_wifi_pass) - 1] = '\0';
        changed = true;
      }
    }
    if (jsonExtractString(body, "ip", s)) {
      IPAddress ip;
      if (ip.fromString(s)) {
        s = s.substring(0, sizeof(g_wifi_ip) - 1);
        if (strncmp(g_wifi_ip, s.c_str(), sizeof(g_wifi_ip) - 1) != 0) {
          strncpy(g_wifi_ip, s.c_str(), sizeof(g_wifi_ip) - 1);
          g_wifi_ip[sizeof(g_wifi_ip) - 1] = '\0';
          changed = true;
        }
      }
    }

    if (changed) nvsSave();
    sendOk();
}

// ============================================================
//  API: /api/switch
// ============================================================
static void handleSwitch() {
    if (webServer.method() != HTTP_POST) { send404(); return; }
    if (!webServer.hasArg("plain")) { send404(); return; }
    String body = webServer.arg("plain");

    // ch
    int ch = -1;
    jsonExtractInt(body, "ch", ch);
    if (ch < 0 || ch > 7) { send404(); return; }

    // release
    bool release = false;
    if (jsonExtractBool(body, "release", release) && release) {
        g_manual_override[ch] = false;
        sendOk(); return;
    }

    // on/off
    bool on = false;
    if (!jsonExtractBool(body, "on", on)) { send404(); return; }
    g_manual_override[ch] = true;
    g_manual_state[ch]    = on;
    sendOk();
}

// ============================================================
//  API: /api/output
// ============================================================
static void handleOutput() {
    if (webServer.method() != HTTP_POST) { send404(); return; }
    if (!webServer.hasArg("plain")) { send404(); return; }
    String body = webServer.arg("plain");
    int ch = -1;
    jsonExtractInt(body, "ch", ch);
    if (ch < 0 || ch > 7) { send404(); return; }
    bool changed = false;
    int val = 0;
    if (jsonExtractInt(body, "kanal", val)) {
      if (val >= 0 && val <= 95 && val != Ausgang_Kanal[ch]) {  // NEU V3.00: MKan bis 95
        Ausgang_Kanal[ch] = val;
        changed = true;
      }
    }
    int pwm_old = pwm_wert[ch];
    if (jsonExtractInt(body, "pwm", val)) {
      bool validPwm = (val >= 0 && val <= 255) || (val >= 300 && val <= 315);
      if (validPwm && val != pwm_wert[ch]) {
        pwm_wert[ch] = val;
        changed = true;
      }
    }
    // MWprop aktiviert -> Ausgang automatisch einschalten
    if (pwm_wert[ch] >= 200 && pwm_wert[ch] <= 207 &&
        !(pwm_old >= 200 && pwm_old <= 207)) {
        einkanal_Data |= (1 << ch);
    }
    if (jsonExtractInt(body, "mode", val)) {
      val = constrain(val, 0, 0xFFFF);
      if (val != mode[ch]) {
        mode[ch] = val;
        changed = true;
      }
    }

    // NEU V3.10: Konfig 2 pro Ausgang (optional - 255 = "nicht verwendet")
    if (jsonExtractInt(body, "kanal2", val)) {
      if ((val == 255 || (val >= 0 && val <= 95)) && val != Ausgang_Kanal2[ch]) {
        Ausgang_Kanal2[ch] = val;
        changed = true;
      }
    }
    if (jsonExtractInt(body, "pwm2", val)) {
      bool validPwm = (val >= 0 && val <= 255) || (val >= 300 && val <= 315);
      if (validPwm && val != pwm_wert2[ch]) {
        pwm_wert2[ch] = val;
        changed = true;
      }
    }
    if (jsonExtractInt(body, "mode2", val)) {
      val = constrain(val, 0, 0xFFFF);
      if (val != mode2[ch]) {
        mode2[ch] = val;
        changed = true;
      }
    }

    // NEU V3.10: Konfig 3 pro Ausgang (optional - 255 = "nicht verwendet")
    if (jsonExtractInt(body, "kanal3", val)) {
      if ((val == 255 || (val >= 0 && val <= 95)) && val != Ausgang_Kanal3[ch]) {
        Ausgang_Kanal3[ch] = val;
        changed = true;
      }
    }
    if (jsonExtractInt(body, "pwm3", val)) {
      bool validPwm = (val >= 0 && val <= 255) || (val >= 300 && val <= 315);
      if (validPwm && val != pwm_wert3[ch]) {
        pwm_wert3[ch] = val;
        changed = true;
      }
    }
    if (jsonExtractInt(body, "mode3", val)) {
      val = constrain(val, 0, 0xFFFF);
      if (val != mode3[ch]) {
        mode3[ch] = val;
        changed = true;
      }
    }

    String nm;
    if (jsonExtractString(body, "name", nm)) {
      nm = nm.substring(0, 16);
      if (strncmp(Ausgang_Name[ch], nm.c_str(), 16) != 0) {
        strncpy(Ausgang_Name[ch], nm.c_str(), 16);
        Ausgang_Name[ch][16] = '\0';
        changed = true;
      }
    }
    if (changed) nvsSave();
    sendOk();
}

// ============================================================
//  API: /api/restart
// ============================================================
static void handleRestart() {
  storageSave();
    sendOk();
    delay(200);
    ESP.restart();
}

// ============================================================
//  API: /api/reset
// ============================================================
static void handleReset() {
    nvsReset();
    sendOk();
}

// ============================================================
//  API: /api/save  (NEU v2.00)
// ============================================================
// Schreibt die Konfiguration sofort auf Flash, OHNE neu zu starten - anders
// als /api/restart. Wird vom Config-Import gebraucht, der mehrere /api/
// config- und /api/output-Aufrufe hintereinander macht und erst am Ende
// einmal sichern will (statt bei jedem einzelnen Schritt neu zu starten).
static void handleSave() {
    storageSave();
    sendOk();
}

// ============================================================
//  API: /api/wifi  (NEU v2.00)
// ============================================================
// Manueller WLAN-Schalter: wirkt SOFORT ueber webui_enableAP()/
// webui_disableAP(), absichtlich OHNE nvsSave() - wie der bisherige "Test"-
// Schalter fuer Ausgaenge ist das nur ein Laufzeit-Zustand, kein Teil der
// gespeicherten Konfiguration. Fuer dauerhaftes Verhalten siehe wifi_auto/
// wifi_auto_timeout in /api/config.
static void handleWifi() {
    if (webServer.method() != HTTP_POST) { send404(); return; }
    if (!webServer.hasArg("plain")) { send404(); return; }
    String body = webServer.arg("plain");
    bool enable = false;
    if (!jsonExtractBool(body, "enable", enable)) { send404(); return; }
    if (enable) webui_enableAP(); else webui_disableAP();
    sendOk();
}

// ============================================================
//  API: /api/otaupdate  (NEU v2.00, uebernommen aus dem Soundmodul-Projekt)
// ============================================================
// Firmware-Update direkt per WLAN: .bin-Datei per multipart/form-data hoch-
// laden, wird ueber die Arduino-"Update"-Bibliothek in das gerade INAKTIVE
// OTA-Flash-Segment geschrieben (ota_0/ota_1 - siehe README, Abschnitt
// "Partitionsschema"). Die aktuell laufende Firmware liegt im jeweils
// ANDEREN Segment und bleibt waehrend des gesamten Uploads unangetastet:
// schlaegt der Upload fehl oder wird er abgebrochen, bootet das Modul beim
// naechsten Start einfach mit der bisherigen Firmware weiter (kein Bricking-
// Risiko durch einen misslungenen Upload). Zwei-Handler-Muster wie beim
// ESP32 WebServer fuer Datei-Uploads ueblich: handleOtaUpdateData() wird
// waehrend des Uploads mehrfach aufgerufen (Callback-Argument von
// webServer.on()), handleOtaUpdate() genau einmal danach fuer die Antwort.
static bool     otaFailed  = false;
static String   otaErrCode;
static uint32_t otaBytes   = 0;
static bool     otaStarted = false;

static void handleOtaUpdateData() {
    HTTPUpload& upload = webServer.upload();

    if (upload.status == UPLOAD_FILE_START) {
        otaFailed = false; otaErrCode = ""; otaBytes = 0; otaStarted = false;
        Serial.printf("[OTA] Start: %s\n", upload.filename.c_str());
        // Bei multipart/form-data ist die Gesamtgroesse vorab nicht bekannt -
        // UPDATE_SIZE_UNKNOWN laesst die Bibliothek bis zum Ende des jeweiligen
        // OTA-Segments schreiben; die eigentliche Vollstaendigkeitspruefung
        // passiert bei Update.end() unten.
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            otaFailed = true; otaErrCode = "update_begin";
            return;
        }
        otaStarted = true;

    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (otaFailed || !otaStarted) return; // vorheriger Fehler - Rest verwerfen
        // Grobe Plausibilitaetspruefung am allerersten Haeppchen: jede gueltige
        // ESP32-Firmware beginnt mit dem Magic-Byte 0xE9. Faengt eine versehent-
        // lich falsche Datei (z.B. eine JSON-Konfigurationsdatei) frueh ab,
        // statt sie komplett hochzuladen und erst bei Update.end() abzulehnen.
        if (otaBytes == 0 && upload.currentSize > 0 && upload.buf[0] != 0xE9) {
            otaFailed = true; otaErrCode = "bad_file";
            return;
        }
        size_t written = Update.write(upload.buf, upload.currentSize);
        otaBytes += written;
        if (written != upload.currentSize) {
            otaFailed = true; otaErrCode = "flash_write"; // z.B. Segment voll
        }

    } else if (upload.status == UPLOAD_FILE_END || upload.status == UPLOAD_FILE_ABORTED) {
        if (upload.status == UPLOAD_FILE_ABORTED) { otaFailed = true; otaErrCode = "aborted"; }
        if (otaStarted) {
            if (otaFailed) {
                Update.abort();
            } else if (!Update.end(true)) {
                // "true" = neue Partition als bootfaehig markieren, aber nur wenn
                // die empfangenen Bytes vollstaendig sind - schlaegt z.B. an, wenn
                // die Datei kein vollstaendiges Firmware-Image ist oder mitten in
                // der Uebertragung abgebrochen wurde, ohne dass ABORTED erkannt wird.
                otaFailed = true; otaErrCode = "update_end";
            } else {
                Serial.printf("[OTA] Fertig: %u Bytes\n", (unsigned)otaBytes);
            }
        }
    }
}

// Wird einmal aufgerufen, NACHDEM der Upload-Callback oben fertig ist -
// schickt die JSON-Antwort und startet das Modul bei Erfolg neu. Die
// Antwort wird per flush() explizit hinausgeschickt und dann kurz gewartet,
// damit sie den Browser noch erreicht, BEVOR der Neustart die WLAN-
// Verbindung (kurzzeitig) abbrechen laesst.
static void handleOtaUpdate() {
    if (otaFailed) {
        webServer.send(200, "application/json", "{\"ok\":false,\"error\":\"" + otaErrCode + "\"}");
        return;
    }
    sendJson("{\"ok\":true,\"size\":" + String(otaBytes) + "}");
    webServer.client().flush();
    delay(500);
    ESP.restart();
}

// ============================================================
//  webui_init / webui_handle
// ============================================================
// AP-Start/-Stop als eigene Funktionen (NEU v2.00, uebernommen aus dem
// Soundmodul-Projekt) - vorher direkt in webui_init() und damit nur beim
// Booten (GPIO13 == LOW) moeglich. Jetzt auch zur Laufzeit aufrufbar: manuell
// per /api/wifi oder automatisch per wifiFailsafeCheck() in der .ino.
static bool g_apActive = false;

bool webui_isApActive() { return g_apActive; }

void webui_enableAP() {
    if (g_apActive) return;
    IPAddress apIP, gw, subnet(255,255,255,0);
    if (!apIP.fromString(g_wifi_ip)) apIP.fromString("192.168.1.1");
    gw = apIP;
    WiFi.softAPConfig(apIP, gw, subnet);
    WiFi.softAP(g_wifi_ssid, g_wifi_pass);
    g_apActive = true;
    Serial.printf("AP gestartet (Heap frei: %u Bytes), IP: %s\n",
                  (unsigned)ESP.getFreeHeap(), WiFi.softAPIP().toString().c_str());
}

void webui_disableAP() {
    if (!g_apActive) return;
    WiFi.softAPdisconnect(true);
    g_apActive = false;
    Serial.println("AP gestoppt.");
}

void webui_init() {
    // FIX v2.00: WiFi-Treiber (LWIP/Netif) MUSS initialisiert sein, bevor
    // webServer.begin() unten laeuft - sonst existieren dessen interne
    // FreeRTOS-Queues noch nicht und es kommt beim Anlegen des Server-Sockets
    // zum Absturz ("assert failed: xQueueSemaphoreTake"), naemlich immer dann,
    // wenn der AP beim Booten NICHT aktiviert wird (GPIO13 nicht LOW und
    // WLAN-Auto-Failsafe (noch) nicht ausgeloest). WiFi.mode(WIFI_AP) legt nur
    // den Treiber/die Queues an, sendet aber noch nichts sichtbares - das
    // eigentliche Einschalten des Access Points (SSID sichtbar) macht weiterhin
    // ausschliesslich webui_enableAP().
    WiFi.mode(WIFI_AP);

    // NEU v2.02: WLAN dauerhaft an (g_wifi_always_on) uebersteuert den
    // GPIO13-Bootpin vollstaendig - siehe Kommentar bei g_wifi_always_on
    // in der .ino. Default AN, siehe Migrations-Default in nvsLoad().
    if (g_wifi_always_on) {
        webui_enableAP();
    } else if (!digitalRead(WifiPin)) {
        webui_enableAP();
    }
    webServer.on("/",            HTTP_GET,  []{ webServer.send_P(200,"text/html",WEBUI_HTML); });
    webServer.on("/api/status",  HTTP_GET,  handleStatus);
    webServer.on("/api/config",  HTTP_GET,  handleConfig);
    webServer.on("/api/config",  HTTP_POST, handleConfig);
    webServer.on("/api/switch",  HTTP_POST, handleSwitch);
    webServer.on("/api/output",  HTTP_POST, handleOutput);
    webServer.on("/api/reset",   HTTP_POST, handleReset);
    webServer.on("/api/restart", HTTP_POST, handleRestart);
    webServer.on("/api/save",    HTTP_POST, handleSave);
    webServer.on("/api/wifi",    HTTP_POST, handleWifi);
    webServer.on("/api/otaupdate", HTTP_POST, handleOtaUpdate, handleOtaUpdateData);
    // WebServer.begin() bewusst unabhaengig vom AP-Status: der Server lauscht
    // so schon, sobald der AP spaeter per Auto-Failsafe/manuell aktiviert wird
    // (kein erneutes .begin() noetig).
    webServer.begin();
    Serial.println("WebServer gestartet.");
}

void webui_handle() {
    webServer.handleClient();
}
