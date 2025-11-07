Web Services Library

This library includes support for OTA firmware updates, and (optionally) a
Web-Sockets-based Web Server and a (local-file-system-based) configuration
file feature.

##Principles of Operation

**TBD**

##Uploading Files to LittleFS on ESP8266

* Install LittleFS Uploader Plugin in Arduino IDE (1.x only)
  - https://github.com/earlephilhower/arduino-esp8266littlefs-plugin/releases
  - unzip and install jar file in "<sketchbook>/tools/ESP8266FS/tool/esp8266fs.jar"
  - https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html
* Arduino IDE (1.x)
  - create sketch with "data/" folder containing files to upload
  - make sure Serial Monitor is closed
  - Tools=>ESP8266 LittleFS Data Upload

**TBD**

* Notes
  - OTA Updates
    * GUI-based
    * 
  - Config State
    * display in GUI
      - common: ipaddr, RSSI, ssid, library version
      - application-specific: app version, ?
    * change from cmd or GUI
      - common: ssid, passwd, report interval
      - application-specific: ?
    * read from file and initialize with saved values
    * save to file on change
  - WebServer
    * Web-Sockets-based GUI
    * contents
      - System Information
        * library version, app verion
        * RSSI
        * connected SSID
        * IP address
      - System Controls
        * firmware update
        * ssid, passwd
      - Application Information
        * ?
      - Application Controls
        * ?
      - Update
  - allow these two features to be enabled independently
    * WebSocket-based GUI for SensorNet common and app-specific display/control
    * Local config file
  - OTA FW update is always enabled by default
    * offered at a standard URL
  - have a standard info/controls page for common info and separate page for app-specific info/controls
    * allow addition of multiple pages
  - this assumes that something else (typically, the SensorNet lib) has set up the WiFi network
  - if you call setup() with a null configPath arg, then the config file feature is not enabled
  - if you call setup() with a null rootPagePath arg, then no common info/controls page is offered
  - call ?() to add app-specific info/controls page
  - have separate or common local json file for common and application-specific config info?
  


  **N.B.**

  * Removed Hash.h from AsyncElegentOTA package
  * Removed instantiation of AsyncElegentOTA in AsyncElegentOTA.h and put it into WebSevices.h
    - sleazy hack, need to fix this properly
  * have to use old Arduino IDE to upload files to LittleFS
    - doesn't work with the 2.0 IDE

  **TODO**
  * Fix template pre-processor function
  * make title a template value -- application-specific
