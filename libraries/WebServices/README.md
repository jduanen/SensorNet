Web Services Library

This library includes support for OTA firmware updates, and (optionally) a
Web-Sockets-based Web Server and a (local-file-system-based) configuration
file feature.

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
  