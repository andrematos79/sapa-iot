# SAPA IoT — Requirements

## 1. Development Environment

* Arduino IDE 2.3.7 or later
* ESP32 board package installed
* Board: ESP32 Dev Module
* USB serial driver compatible with the ESP32 board
* Serial Monitor baud rate: 115200 bps

## 2. Hardware Requirements

* ESP32 Dev Module
* SSR module or relay interface
* Industrial contactor
* Push button
* Status LEDs
* 5 V power supply for control circuit
* 220 V AC power line for amplifier control
* Protected electrical enclosure
* DS3231 RTC module with CR2032 battery (planned for v0.6.0)

## 3. Current Pin Mapping

| Function                | ESP32 GPIO |
| ----------------------- | ---------: |
| SSR / Contactor control |    GPIO 23 |
| Push button             |    GPIO 22 |
| Green LED               |    GPIO 19 |
| Red LED                 |    GPIO 18 |
| Yellow LED              |     GPIO 5 |

## 4. Software Dependencies

Current firmware uses only standard ESP32/Arduino libraries:

```cpp
#include <WiFi.h>
#include <WebServer.h>
```

No external Arduino library is required for v0.5.1.

## 5. Network Requirements

The system operates in Standalone Access Point mode.

* SSID: SAPA_IOT
* Password: sapa1234
* Local access URL: http://192.168.4.1

Internet access is not required.

## 6. Functional Requirements

* The ESP32 shall create its own Wi-Fi network.
* The system shall provide a local web interface.
* The system shall control the amplifier through SSR/contactor output.
* The system shall support automatic and manual operating modes.
* The system shall support multi-window weekly scheduling.
* The system shall allow schedule configuration through the web interface.
* The system shall support browser-based time synchronization.
* The system shall provide event logging for operational traceability.
* The system shall provide an About/System Information page.
* The system shall display software version and developer signature.

## 7. Safety Requirements

* On boot, the output shall start in a safe OFF state.
* Manual commands shall be logged.
* Automatic schedule commands shall be logged.
* The system shall avoid depending on external routers or Windows hotspot services.
* High-voltage wiring shall be isolated from the ESP32 control circuit.
* Final installation shall be performed inside a protected enclosure.

## 8. Planned Requirements for v0.6.0

* DS3231 RTC integration.
* Automatic time recovery after power loss.
* Reliable post-blackout schedule recovery.
* Real timestamp in logs.
* Power outage detection report.
* Calculation of downtime duration.
* Improved industrial audit report.

## 9. Planned Requirements for Future Versions

* CSV log export.
* Temperature sensor integration.
* Amplifier room temperature monitoring.
* Historical reports.
* OTA firmware update.
* Optional MQTT integration.
* Optional dashboard integration.
