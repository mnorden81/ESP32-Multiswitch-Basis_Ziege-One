/*
 * board_config.h  -  Pin- und Board-Konfiguration RC-Multiswitch v5.00
 *
 * Gleicher Quellcode fuer alle Platinen. Der Chip wird automatisch aus dem
 * in der Arduino IDE gewaehlten Board erkannt:
 *   ESP32C3 Dev Module -> BOARD_C3    (ESP32-C3 SuperMini)
 *   ESP32S3 Dev Module -> BOARD_S3    (Waveshare ESP32-S3-Zero)
 *   ESP32 Dev Module   -> BOARD_ESP32 (klassischer ESP32: Platine Basis mit
 *                         D1 Mini oder Platine TXRX, v5.00)
 * Alternativ BOARD_C3, BOARD_S3 bzw. BOARD_ESP32 vor dem Include definieren.
 *
 * Basis und TXRX haben denselben Chip. Welche der beiden Platinen verbaut
 * ist, waehlt man zur Laufzeit (WebUI Reiter RC-System, Karte "Platine",
 * oder Lua Global > "Platine (Neustart)", NVS-Schluessel "board"). Die
 * Ausgangspins wechseln nach einem Neustart. Deshalb stehen die Pins in
 * BOARD_OUT_PINS[Variante][Ausgang]; die .ino kopiert die aktive Zeile beim
 * Start nach OutPin[].
 *
 * Arduino-IDE-Einstellungen:
 *   C3/S3: USB CDC On Boot: Enabled (Debug-Ausgaben ueber USB, UART0 = CRSF)
 *   alle:  Partition Scheme mit OTA (z.B. "Default 4MB with spiffs")
 *
 * Pinbelegung laut docs/Hardware-Beschreibung_C3_S3.docx und
 * docs/Firmware-Spezifikation.md, Kapitel 2.
 */
#pragma once
#include <Arduino.h>

#if !defined(BOARD_C3) && !defined(BOARD_S3) && !defined(BOARD_ESP32)
  #if defined(CONFIG_IDF_TARGET_ESP32C3)
    #define BOARD_C3
  #elif defined(CONFIG_IDF_TARGET_ESP32S3)
    #define BOARD_S3
  #elif defined(CONFIG_IDF_TARGET_ESP32)
    #define BOARD_ESP32
  #else
    #error "Unbekanntes Board: bitte ESP32C3, ESP32S3 oder ESP32 Dev Module waehlen"
  #endif
#endif

#if (defined(BOARD_C3) || defined(BOARD_S3)) && !ARDUINO_USB_CDC_ON_BOOT
  #error "Bitte 'USB CDC On Boot: Enabled' einstellen - UART0 wird fuer CRSF (UART A) gebraucht"
#endif

// Je Board:
//   BOARD_CHIP, BOARD_VARIANT_COUNT, BOARD_VARIANT_NAMES[] (Name je Platine,
//   bei C3/S3 nur eine), BOARD_OUT_PINS[Variante][8], OutPwm[8],
//   BOARD_LED_TEXT/BOARD_WIFI_TEXT (Anzeige im Web), UART A/B/C, BOOT-Taster,
//   Status-LED, optional WIFI_BOOT_PIN (LOW beim Start = WLAN-AP an).
//   BOOT_BTN_PIN nur, wo es einen BOOT-Taster gibt (C3/S3).
#if defined(BOARD_C3)
  #define BOARD_CHIP "ESP32-C3"
  static constexpr uint8_t BOARD_VARIANT_COUNT = 1;
  static constexpr const char* BOARD_VARIANT_NAMES[1] = {"C3"};

  // Ausgaenge S0..S7 (Kanal 1..8)
  static constexpr uint8_t BOARD_OUT_PINS[1][8] = {{0, 1, 2, 3, 4, 5, 6, 7}};
  // ESP32-C3 hat nur 6 LEDC-Kanaele: Ausgang 7/8 schalten nur digital
  // (an ab 50 % Duty, Blinken weiter per Software), siehe outWrite().
  static constexpr bool OutPwm[8] = {true, true, true, true, true, true, false, false};

  // UART A (Eingang vom Empfaenger) = UART0
  #define UART_A_SERIAL Serial0
  static constexpr uint8_t UART_A_RX = 20;
  static constexpr uint8_t UART_A_TX = 21;
  // UART B (weiter zum naechsten Modul) = UART1
  #define UART_B_SERIAL Serial1
  static constexpr uint8_t UART_B_RX = 8;    // zugleich Onboard-LED des SuperMini
  static constexpr uint8_t UART_B_TX = 10;
  // UART C: nicht vorhanden

  static constexpr uint8_t BOOT_BTN_PIN = 9;
  // Keine steuerbare Status-LED (Onboard-LED IO8 ist UART-B-RX)
  static constexpr int STATUS_LED_PIN = -1;
  #define STATUS_LED_WS2812 0
  #define STATUS_LED_RGB_DEFAULT false
  #define BOARD_LED_TEXT  "keine (IO8 ist UART B RX)"
  #define BOARD_WIFI_TEXT "BOOT-Taster 2 s oder Dauerhaft an"

#elif defined(BOARD_S3)
  #define BOARD_CHIP "ESP32-S3"
  static constexpr uint8_t BOARD_VARIANT_COUNT = 1;
  static constexpr const char* BOARD_VARIANT_NAMES[1] = {"S3"};

  static constexpr uint8_t BOARD_OUT_PINS[1][8] = {{11, 4, 5, 6, 10, 9, 8, 7}};
  static constexpr bool    OutPwm[8] = {true, true, true, true, true, true, true, true};

  #define UART_A_SERIAL Serial0
  static constexpr uint8_t UART_A_RX = 44;
  static constexpr uint8_t UART_A_TX = 43;
  #define UART_B_SERIAL Serial1
  static constexpr uint8_t UART_B_RX = 12;
  static constexpr uint8_t UART_B_TX = 13;
  #define UART_C_SERIAL Serial2
  static constexpr uint8_t UART_C_RX = 1;
  static constexpr uint8_t UART_C_TX = 2;

  static constexpr uint8_t BOOT_BTN_PIN = 0;
  // WS2812 auf dem S3-Zero
  static constexpr int STATUS_LED_PIN = 21;
  #define STATUS_LED_WS2812 1
  static constexpr uint8_t STATUS_LED_BRIGHTNESS = 40;  // von 255, ca. 15 %
  // v4.10: LED des S3-Zero erwartet RGB statt GRB (Rot/Gruen vertauscht),
  // Vorgabe fuer Lua "LED Farbfolge" (NVS "ledrgb")
  #define STATUS_LED_RGB_DEFAULT true
  #define BOARD_LED_TEXT  "WS2812 GPIO 21"
  #define BOARD_WIFI_TEXT "BOOT-Taster 2 s oder Dauerhaft an"

#elif defined(BOARD_ESP32)
  // v5.00: klassischer ESP32, Platinen aus ESP32-Multiswitch-Basis_Ziege-One
  // (bis v3.17 eigene Ordner MultiSwitch_ESP32_V3 und _TXRX)
  #define BOARD_CHIP "ESP32"
  #define BOARD_SELECTABLE 1
  static constexpr uint8_t BOARD_VARIANT_COUNT = 2;
  static constexpr const char* BOARD_VARIANT_NAMES[2] = {"Basis", "TXRX"};
  // 0 = Basis (ESP32 D1 Mini), 1 = TXRX (ULN2803A / TBD62083)
  static constexpr uint8_t BOARD_OUT_PINS[2][8] = {
      {18, 19, 21, 22, 23, 25, 26, 27},
      {27, 25,  5, 23, 19, 18, 26, 33}};
  // 16 LEDC-Kanaele: alle Ausgaenge mit PWM
  static constexpr bool OutPwm[8] = {true, true, true, true, true, true, true, true};

  // UART A (Eingang vom Empfaenger) = UART2 wie bis v3.17, UART0 bleibt USB/Debug
  #define UART_A_SERIAL Serial2
  static constexpr uint8_t UART_A_RX = 16;
  static constexpr uint8_t UART_A_TX = 17;
  // UART B/C: auf Basis/TXRX nicht herausgefuehrt

  // Kein BOOT-Taster (D1 Mini hat nur Reset), WLAN ueber GPIO13 oder Web/Lua
  // GPIO13 beim Start auf GND = WLAN-AP an (wie bis v3.17)
  #define WIFI_BOOT_PIN 13
  // einfache LED an GPIO2 (wie bis v3.17)
  static constexpr int STATUS_LED_PIN = 2;
  #define STATUS_LED_WS2812 0
  #define STATUS_LED_RGB_DEFAULT false
  #define BOARD_LED_TEXT  "GPIO 2"
  #define BOARD_WIFI_TEXT "GPIO 13 beim Start auf GND oder Dauerhaft an"
#endif

#ifndef BOARD_SELECTABLE
  #define BOARD_SELECTABLE 0
#endif
