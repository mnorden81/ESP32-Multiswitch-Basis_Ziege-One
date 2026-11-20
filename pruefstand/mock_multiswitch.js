// Simuliertes RC-Multiswitch-Modul (Firmware RC_Multiswitch, C3/S3/ESP32).
// Antworten wie webui.h; Pins, Namen und Vorgaben kommen aus dem Code
// (config.json "export"). Gleiche Datei in RC-Multiswitch-ESP32-C3/-S3 und
// ESP32-Multiswitch-Basis_Ziege-One.
PST.register('mock_multiswitch', function (variant, ctx) {
  var K = ctx.variant.consts || {};
  var chip = variant === 'v2' ? 'esp32' : variant;   // 's3' | 'c3' | 'esp32' | 'v2' (MultiSwitch_ESP32 V2)
  var RC_NAMES = ['FrSky', 'FlySky', 'ELRS SBUS', 'Hott', 'ELRS CRSF', 'CRSF + Funk', 'CRSF aus Funk'];
  var names = (K.Ausgang_Name || []).slice();
  for (var n = names.length; n < 8; n++) names.push('Ausgang ' + (n + 1));
  function fresh() {
    return {
      rc_system: 4, crsf_channel: 999, einkanal_mode: 0, modul_adress: 0,
      ek_gadr0: 255, ek_gadr1: 255, sbus_gch0: 999, sbus_gch1: 999, sbus_gmode0: 0, sbus_gmode1: 0,
      dnam: 'MultiSwitch', ssid: 'MultiSwitch', pass: '123456789', ip: '192.168.4.1',
      wifi_auto: true, wifi_auto_timeout: 60, wifi_always_on: true,
      portb_role: 1, portc_role: 1, portb_baud: 420000, portc_baud: 420000,
      board: 0, funk_id: 1, funk_ch: 1, funk_pwr: 20, funk_rate: 10, funk_back: true,
      led_rgb: true,
      out: names.map(function (nm, i) {
        return { name: nm, kanal: (K.Ausgang_Kanal || [])[i] != null ? K.Ausgang_Kanal[i] : i, pwm: 255, mode: 0,
          kanal2: 255, pwm2: 255, mode2: 0, kanal3: 255, pwm3: 255, mode3: 0, on: false, override: false, manual: false };
      })
    };
  }
  var c = fresh();
  // Startwerte aus dem Code, wenn build.py handleConfig() ausgewertet hat
  var sample = ctx.variant.api && ctx.variant.api['GET /api/config'];
  if (sample) Object.keys(c).forEach(function (k) { if (k !== 'out' && sample[k] != null && typeof sample[k] === typeof c[k]) c[k] = sample[k]; });
  var boot = { rc: c.rc_system, board: c.board };
  var ekData = 0, apActive = true, outSel = 0;
  var hasB = !!K.UART_B_RX || chip !== 'esp32', hasC = chip === 's3';
  function boardName() { if (variant === 'v2') return 'V2'; return chip === 'esp32' ? (K.BOARD_VARIANT_NAMES || ['Basis', 'TXRX'])[boot.board] : (chip === 's3' ? 'S3' : 'C3'); }
  function pins() {
    var p = K.BOARD_OUT_PINS || K.OutPin || [];
    return Array.isArray(p[0]) ? p[boot.board] : p;
  }
  function verText() { return 'v' + ctx.variant.version + (variant === 'v2' ? '' : ' ' + boardName()); }
  var common = PST.common({
    name: function () { return c.dnam; }, addr: function () { return c.modul_adress; },
    funkRole: function () { return boot.rc === 5 ? 'Zentrale' : (boot.rc === 6 ? 'Teilnehmer' : null); },
    funkCfg: function () { return { id: c.funk_id, ch: c.funk_ch, pwr: c.funk_pwr, rate: c.funk_rate, back: c.funk_back }; },
    backupData: function () { return config(); }
  });
  function config() {
    var j = {};
    ['rc_system', 'crsf_channel', 'einkanal_mode', 'modul_adress', 'ek_gadr0', 'ek_gadr1', 'sbus_gch0', 'sbus_gch1', 'sbus_gmode0', 'sbus_gmode1',
      'dnam', 'ssid', 'pass', 'ip', 'wifi_auto', 'wifi_auto_timeout', 'wifi_always_on'].forEach(function (k) { j[k] = c[k]; });
    j.port_roles = 'Aus;CRSF;S.Port;GPS;Raw';
    j.portb_role = c.portb_role; j.portc_role = c.portc_role; j.portb_baud = c.portb_baud; j.portc_baud = c.portc_baud;
    j.has_portc = hasC; j.portc_dup = false; j.has_portb = hasB;
    j.board = c.board; j.board_select = variant === 'esp32';
    j.board_info = { name: boardName(), chip: K.BOARD_CHIP || chip, rx: K.UART_A_RX, tx: K.UART_A_TX, out: pins(), led: K.BOARD_LED_TEXT || '', wifi: K.BOARD_WIFI_TEXT || '' };
    j.funk_id = c.funk_id; j.funk_ch = c.funk_ch; j.funk_pwr = c.funk_pwr; j.funk_rate = c.funk_rate; j.funk_back = c.funk_back;
    return j;
  }
  function status() {
    var t = Date.now() / 1000;
    var ch = [992, 992, 172 + Math.round(800 * (1 + Math.sin(t / 2))), 992, 1811, 172, 992, 992];
    var outs = c.out.map(function (o, i) {
      var on = o.manual ? o.on : !!(ekData & (1 << i));
      return { name: o.name, on: on, override: o.manual, kanal: o.kanal, pwm: o.pwm,
        mode_hex: '0x' + ('000' + o.mode.toString(16).toUpperCase()).slice(-4), mode: o.mode,
        kanal2: o.kanal2, pwm2: o.pwm2, mode2: o.mode2, kanal3: o.kanal3, pwm3: o.pwm3, mode3: o.mode3 };
    });
    var j = { version: verText(), bus_ok: boot.rc !== 6, rc_system: boot.rc, rc_system_name: RC_NAMES[boot.rc] || '?', rc_system_boot: boot.rc,
      einkanal_data_hex: ('0' + ekData.toString(16).toUpperCase()).slice(-2), outputs: outs, channels: ch, wm_prop: [0, 0, 0, 0, 0, 0, 0, 0],
      crsf_diag: { raw_bytes: Math.round(t * 1450) % 1e7, valid_frames: Math.round(t * 150) % 1e6, crc_errors: 0, device_pings: 3, param_reads: 41, param_writes: 2,
        devinfo_sent: 3, wm_cmds: 12, wm_hit: true, wm_own: c.modul_adress, wm_last: [0xC8, 0xEA, 0xA0, 0x01, c.modul_adress, 0, 0, 0],
        fwd_down: 0, fwd_up: 0, fwd_dropped: 0, down_crc: 0, dup_addr: false },
      ports: { b: hasB ? (c.portb_role ? 'aktiv' : 'aus') : 'nicht vorhanden', c: hasC ? (c.portc_role ? 'aktiv' : 'aus') : 'nicht vorhanden', has_c: hasC, c_dup: false },
      wifi_ap_active: apActive, funk: common.funkSummary() };
    if (c.portb_role === 2 || c.portc_role === 2) {
      j.ports.sport = [{ online: true, cells: 3, total: 11.82, min: 3.93 }]; j.ports.sport_polls = 1200; j.ports.sport_bytes = 9600; j.ports.sport_frames = 400; j.ports.sport_crc = 0;
    }
    if (c.portb_role === 3 || c.portc_role === 3) j.ports.gps = { fix: true, sats: 9, kmh: 12.4 };
    return j;
  }
  function body(b) { return PST.parseBody(b); }
  // Felder, die nur die Firmware kennt, aus der Beispielantwort (build.py) ergaenzen
  function withSample(route, o) {
    var s = ctx.variant.api && ctx.variant.api[route];
    if (!s) return o;
    Object.keys(s).forEach(function (k) { if (!(k in o)) o[k] = s[k]; });
    return o;
  }
  var NUMKEYS = ['rc_system', 'crsf_channel', 'einkanal_mode', 'modul_adress', 'ek_gadr0', 'ek_gadr1', 'sbus_gch0', 'sbus_gch1', 'sbus_gmode0', 'sbus_gmode1',
    'wifi_auto_timeout', 'portb_role', 'portc_role', 'portb_baud', 'portc_baud', 'board', 'funk_id', 'funk_ch', 'funk_pwr', 'funk_rate'];
  function handle(m, path, q, b) {
    var r = common.handle(m, path, q, b); if (r) return r;
    if (path === '/api/status') return PST.json(withSample('GET /api/status', status()));
    if (path === '/api/config' && m === 'GET') return PST.json(withSample('GET /api/config', config()));
    if (path === '/api/config') {
      var d = body(b);
      NUMKEYS.forEach(function (k) { if (d[k] != null) c[k] = parseInt(d[k], 10); });
      ['wifi_auto', 'wifi_always_on', 'funk_back'].forEach(function (k) { if (d[k] != null) c[k] = d[k] === true || d[k] === 'true' || d[k] === 1; });
      ['dnam', 'ssid', 'pass', 'ip'].forEach(function (k) { if (d[k] != null) c[k] = String(d[k]); });
      return PST.json({ ok: true });
    }
    if (path === '/api/switch') {
      var s = body(b), o = c.out[s.ch]; if (!o) return PST.text('Not found', 404);
      if (s.release) o.manual = false; else { o.manual = true; o.on = !!s.on; }
      return PST.json({ ok: true });
    }
    if (path === '/api/output') {
      var p = body(b), x = c.out[p.ch]; if (!x) return PST.text('Not found', 404);
      ['kanal', 'pwm', 'mode', 'kanal2', 'pwm2', 'mode2', 'kanal3', 'pwm3', 'mode3'].forEach(function (k) { if (p[k] != null) x[k] = parseInt(p[k], 10); });
      if (p.name != null) x.name = String(p.name).slice(0, 16);
      return PST.json({ ok: true });
    }
    if (path === '/api/save') return PST.json({ ok: true });
    if (path === '/api/wifi') { var w = body(b); if (w.on != null) apActive = !!w.on; return PST.json({ ok: true }); }
    if (path === '/api/reset') { var keep = c.board; c = fresh(); if (sample) Object.keys(c).forEach(function (k) { if (k !== 'out' && sample[k] != null && typeof sample[k] === typeof c[k]) c[k] = sample[k]; }); c.board = keep; PST.note('Werkseinstellungen geladen (Platine bleibt).'); return PST.json({ ok: true }); }
    if (path === '/api/restart') { setTimeout(PST.reboot, 300); return PST.json({ ok: true }); }
    return null;
  }
  // ── Lua, gleiche Funktionsnamen wie in RC_Multiswitch.ino ──
  function q(x) { var k = c.out[x].kanal; return k < 20 ? 0 : k < 40 ? 1 : k < 60 ? 2 : k < 88 ? 3 : 4; }
  function nr(x) { var k = c.out[x].kanal; return k < 20 ? k + 1 : k < 40 ? k - 19 : k < 60 ? k - 39 : k < 88 ? k - 79 : k - 87; }
  function setQN(x, qq, n) { var i = Math.max(0, n - 1); c.out[x].kanal = [Math.min(i, 7), 20 + Math.min(i, 15), 40 + Math.min(i, 15), 80 + Math.min(i, 7), 88 + Math.min(i, 7)][qq]; }
  function blink(x) { return (c.out[x].mode >> 8) & 0xFF ? 1 : 0; }
  var funkCfg = { get id() { return c.funk_id; }, set id(v) { c.funk_id = v; }, get ch() { return c.funk_ch; }, set ch(v) { c.funk_ch = v; },
    get pwr() { return c.funk_pwr; }, set pwr(v) { c.funk_pwr = v; }, get rate() { return c.funk_rate; }, set rate(v) { c.funk_rate = v; },
    get back() { return c.funk_back; }, set back(v) { c.funk_back = v; } };
  var lua = PST.mergeLua({
    get: {
      luaGetAddr: function () { return c.modul_adress; }, luaGetCrsfCh: function () { return c.crsf_channel === 999 ? 0 : c.crsf_channel; },
      luaGetLedRgb: function () { return c.led_rgb ? 0 : 1; }, luaGetBoard: function () { return c.board; },
      luaGetOutNr: function () { return outSel; }, luaGetZero: function () { return 0; },
      luaGetSelQuelle: function () { return q(outSel); }, luaGetSelNr: function () { return nr(outSel); },
      luaGetSelBlink: function () { return blink(outSel); },
      luaGetSelBlinkOn: function () { return ((c.out[outSel].mode >> 8) & 0xFF) || 10; },
      luaGetSelBlinkOff: function () { return (c.out[outSel].mode & 0xFF) || 10; },
      luaGetSelPwmModus: function () { var p = c.out[outSel].pwm; return p >= 200 && p <= 207 ? 1 : 0; },
      luaGetSelPwm: function () { return Math.min(255, c.out[outSel].pwm); }, luaGetTest: function () { return 0; },
      // bis v5.04 / V2: ein Ordner je Ausgang (Argument = Ausgang)
      luaGetQuelle: function (x) { return q(x); }, luaGetNr: function (x) { return nr(x); }, luaGetBlink: function (x) { return blink(x); },
      luaGetBlinkOn: function (x) { return ((c.out[x].mode >> 8) & 0xFF) || 10; }, luaGetBlinkOff: function (x) { return (c.out[x].mode & 0xFF) || 10; },
      luaGetPwmModus: function (x) { var p = c.out[x].pwm; return p >= 200 && p <= 207 ? 1 : 0; }, luaGetPwm: function (x) { return Math.min(255, c.out[x].pwm); },
      luaGetWifiAuto: function () { return c.wifi_auto ? 1 : 0; }, luaGetWifiTo: function () { return c.wifi_auto_timeout; },
      luaGetWifiAlways: function () { return c.wifi_always_on ? 1 : 0; },
      luaGetRole: function (i) { return i ? c.portc_role : c.portb_role; },
      luaGetBaud: function (i) { var b = i ? c.portc_baud : c.portb_baud; var L = [1, 9600, 19200, 38400, 57600, 115200, 230400, 420000]; return Math.max(0, L.indexOf(b)); }
    },
    set: {
      luaSetAddr: function (a, v) { c.modul_adress = v; }, luaSetCrsfCh: function (a, v) { c.crsf_channel = v; },
      luaSetLedRgb: function (a, v) { c.led_rgb = v === 0; }, luaSetBoard: function (a, v) { c.board = v; },
      luaSetOutNr: function (a, v) { outSel = v; },
      luaSetSelQuelle: function (a, v) { setQN(outSel, v, nr(outSel)); }, luaSetSelNr: function (a, v) { setQN(outSel, q(outSel), v); },
      luaSetSelBlink: function (a, v) { var o = c.out[outSel]; if (!v) o.mode = 0; else o.mode = (((o.mode >> 8) & 0xFF) || 10) << 8 | ((o.mode & 0xFF) || 10); },
      luaSetSelBlinkOn: function (a, v) { var o = c.out[outSel]; o.mode = (v << 8) | (o.mode & 0xFF); },
      luaSetSelBlinkOff: function (a, v) { var o = c.out[outSel]; o.mode = (o.mode & 0xFF00) | v; },
      luaSetSelPwmModus: function (a, v) { var o = c.out[outSel]; if (v) { o.pwm = 200 + outSel; ekData |= 1 << outSel; } else if (o.pwm >= 200 && o.pwm <= 207) o.pwm = 255; },
      luaSetSelPwm: function (a, v) { c.out[outSel].pwm = v; },
      luaSetSelTest: function (a, v) { if (v) ekData |= 1 << outSel; else ekData &= ~(1 << outSel); },
      luaSetQuelle: function (x, v) { setQN(x, v, nr(x)); }, luaSetNr: function (x, v) { setQN(x, q(x), v); },
      luaSetBlink: function (x, v) { var o = c.out[x]; if (!v) o.mode = 0; else o.mode = (((o.mode >> 8) & 0xFF) || 10) << 8 | ((o.mode & 0xFF) || 10); },
      luaSetBlinkOn: function (x, v) { var o = c.out[x]; o.mode = (v << 8) | (o.mode & 0xFF); },
      luaSetBlinkOff: function (x, v) { var o = c.out[x]; o.mode = (o.mode & 0xFF00) | v; },
      luaSetPwmModus: function (x, v) { var o = c.out[x]; if (v) { o.pwm = 200 + x; ekData |= 1 << x; } else if (o.pwm >= 200 && o.pwm <= 207) o.pwm = 255; },
      luaSetPwm: function (x, v) { c.out[x].pwm = v; },
      luaSetTest: function (x, v) { if (v) ekData |= 1 << x; else ekData &= ~(1 << x); },
      luaSetWifiAuto: function (a, v) { c.wifi_auto = !!v; }, luaSetWifiTo: function (a, v) { c.wifi_auto_timeout = v; },
      luaSetWifiAlways: function (a, v) { c.wifi_always_on = !!v; },
      luaSetRole: function (i, v) { if (i) c.portc_role = v; else c.portb_role = v; },
      luaSetBaud: function (i, v) { var L = [1, 9600, 19200, 38400, 57600, 115200, 230400, 420000]; if (i) c.portc_baud = L[v]; else c.portb_baud = L[v]; }
    },
    views: {
      luaShowGlobalFolder: function (it, v) { v.name = 'Global (Adr ' + c.modul_adress + ')'; },
      luaHideSbusOnly: function (it, v) { if (boot.rc >= 4) v.hidden = true; },
      luaShowOutName: function (it, v) { v.text = c.out[outSel].name; v.max = 0; v.value = 0; },
      luaViewSelNr: function (it, v) { var qq = q(outSel); var rc = qq === 1 || qq === 2; v.name = rc ? 'RC-Kanal Nr' : 'Bit Nr'; v.max = rc ? 16 : 8; },
      luaShowOutFolder: function (it, v) { var Q = ['Bit', 'Kanal_L', 'Kanal_H', 'MKan1 Bit', 'MKan2 Bit']; v.name = c.out[it.arg].name + ' (' + Q[q(it.arg)] + ' ' + nr(it.arg) + ')'; },
      luaViewNr: function (it, v) { var qq = q(it.arg); var rc = qq === 1 || qq === 2; v.name = rc ? 'RC-Kanal Nr' : 'Bit Nr'; v.max = rc ? 16 : 8; },
      luaViewBlinkTime: function (it, v) { if (!blink(it.arg)) v.hidden = true; },
      luaViewPwm: function (it, v) { var p = c.out[it.arg].pwm; if (p >= 200 && p <= 207) { v.min = v.max = v.value; v.hidden = true; } },
      luaViewSelBlinkTime: function (it, v) { if (!blink(outSel)) v.hidden = true; },
      luaViewPwmModus: function (it, v) { if (boot.rc < 4) { v.type = 'info'; v.text = 'Festwert (SBUS)'; } },
      luaViewSelPwm: function (it, v) { var p = c.out[outSel].pwm; if (p >= 200 && p <= 207) { v.min = v.max = v.value; v.hidden = true; } },
      luaShowUartFolder: function (it, v) { var R = ['Aus', 'CRSF', 'S.Port', 'GPS', 'Raw']; v.name = hasC ? 'UART B/C (' + R[c.portb_role] + ', ' + R[c.portc_role] + ')' : 'UART B (' + R[c.portb_role] + ')'; },
      luaViewBaud: function (it, v) { if (!(it.arg ? c.portc_role : c.portb_role)) v.hidden = true; }
    }
  }, PST.funkLua(funkCfg, function () { return c.rc_system >= 5 ? c.rc_system - 4 : 0; }, function (v) { c.rc_system = 4 + Math.min(v, 2); }));
  return {
    handle: handle, get: lua.get, set: lua.set, views: lua.views,
    deviceName: function () { return c.dnam + '@' + c.modul_adress; },
    reboot: function () { boot.rc = c.rc_system; boot.board = c.board; },
    hw: function () {
      return [['Chip', K.BOARD_CHIP || chip], ['Platine', boardName()], ['UART A', 'RX ' + K.UART_A_RX + ' / TX ' + K.UART_A_TX],
        ['Ausgänge', pins().join(', ')], ['UART B/C', hasB ? (hasC ? 'B und C' : 'nur B') : 'nicht herausgeführt'],
        ['Status-LED', K.BOARD_LED_TEXT || '–'], ['WLAN-Start', K.BOARD_WIFI_TEXT || '–'], ['RC-System (Start)', RC_NAMES[boot.rc]]];
    }
  };
});
