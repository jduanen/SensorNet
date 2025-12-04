# ESPHome YAML Config File Conventions

* for each instance of a device type, put an instance config file into 'esphome/'
  - an instance config file contains
    * substitutions that are specific to the instance

    * include the base device config file from 'esphome/packages/'
      - base config files names
        * include the device type
        * are lower case and hypen-separated
        * end in "-base.yaml"
    * e.g.,
```
substitutions:
  device_name: "short, lowercase, underbar-separated, ending in instance number"  # e.g., temp_0
  friendly_name: "short string"  # e.g., "Temp Living Room"
  comment: "Full sentance description of device type and instance"  # e.g., "Temperature sensor in living room"
  log_level: logLevel  # e.g., WARN, DEBUG, INFO, VERBOSE

packages:
  hardware: !include packages/device-type-base.yaml
```
* all strings with spaces are enclosed in double-quotes
  - not necessary for YAML, but it makes the field value more obvious
* all device (base) config files include the desired packages from 'esphome/common'
* common packages
  - base.yaml: for devices that don't use WiFi -- base configs common to all SensorNet devices
    * top-level fields defined: 'esphome', 'logger', and 'api'
    * expects
      - substitutions: 'device_name', 'friendly_name', 'comment', and 'log_level'
      - secrets: 'api_encryption_key'
    * need to merge 'baud_rate' into 'logger' if needed
      - e.g., 'baud_rate: 0' for devices that use the UART pins (e.g., radiation, PMS, etc.)
  - wifi.yaml: for devices that use WiFi, but don't offer a Web Server
    * includes 'common/base.yaml', so device config files just have to include this package
    * defines the WiFi signal strength sensor
      - is merged with any declaration of 'sensor' in device config files
    * packages: 'common/base.yaml'
    * top-level fields defined: 'packages', 'wifi', 'ota', 'sensor'
    * expects
      - substitutions: 'friendly_name'
      - secrets: 'wifi_ssid', 'wifi_password', 'wifi_ap_password', and 'ota_password'
  - wifi_ws.yaml
    * substitutions defined: 'web_server_port: 80'
      - is default port value, can be overridden in device config file
    * includes 'common/wifi.yaml', so device config files just have to include this package
    * top-level fields defined: 'web_server'
    * expects and defines as given in 'common/wifi.yaml', nothing added by this package
