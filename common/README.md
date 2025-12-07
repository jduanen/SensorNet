# ESPHome YAML Config File Conventions

* format conventions
  - two-space indents (no tabs, no trailing whitespace)
  - all strings with spaces are enclosed in double-quotes
    * while not necessary for YAML, it makes the field value more obvious
* YAML config file contents for all device instances
  - starts with substitutions that are specific to the instance
    * e.g., the device's name, function, and location
  - then can typically includes packages that provide declarations common to multiple devices
    * see the common packages described below
  - for multiple instances of the same type, the device config file can contain just two top-level labels: i.e., "substitutions" and "packages"
    * e.g., this is an instance of a device that defines
      - the required substitutions used by common packages
      - the base config for the temperature sensor device type
        * in this example, this includes the credentials needed by the device to securely interact with the HA Server are set up, the credentials required to set up the WiFi interface, and the port number on which a web server interface to the device is to be offered
```
substitutions:
  device_name: "short, lowercase, underbar-separated, ending in instance number"  # e.g., temp_0
  friendly_name: "short string"  # e.g., "Temp Living Room"
  comment: "Full sentance description of device type and instance"  # e.g., "Temperature sensor in living room"
  log_level: logLevel  # e.g., WARN, DEBUG, INFO, VERBOSE

packages:
  wifi_ws_conf: github://jduanen/SensorNet/common/wifi.yaml
  dev_conf: github://jduanen/SensorNet/sensors/Radiation/packages/radiation-base.yaml
```
* config files for singleton device-types can contain the configuration declarations directly, and not import a device config package
* file naming
  - the name for device instance config files have a common prefix that includes the device type, and ends in an index integer
    * e.g., 'radiation-0.yaml'
  - the names for device-type's base device config files include the device type, are lower case and hypen-separated, and end in "-base.yaml"
    * e.g., 'radiation-base.yaml'
  - for system base packages, use '<fileName>_conf' label for '<fileName>.yaml'
  - for device base packages, use the 'dev_conf' label
* device config files get all of their included packages from github (by convention)
  - e.g., the above example gets the base with wifi and web server, as well as the base package for the radiation sensors
* common packages have been defined for most common device cases; they are:
  - 'base.yaml': for devices that don't use WiFi -- base configs common to all SensorNet devices
    * top-level fields defined: "esphome", "logger", and "api"
    * expects
      - 'substitutions': 'device_name', 'friendly_name', 'comment', and 'log_level'
      - 'secrets': 'api_encryption_key'
    * can merge "baud_rate" into the "logger" field, if necessary
      - e.g., 'baud_rate: 0' for devices that use the UART pins (e.g., radiation, PMS, etc.)
  - 'wifi.yaml': for devices that use WiFi, but don't offer a Web Server
    * includes 'common/base.yaml', so device config files just have to include this package
    * defines the WiFi signal strength sensor
      - is merged with any declaration of 'sensor' in device config files
    * 'packages': 'common/base.yaml'
    * top-level fields defined: 'packages', 'wifi', 'ota', 'sensor'
    * expects
      - 'substitutions': 'friendly_name'
      - 'secrets': 'wifi_ssid', 'wifi_password', 'wifi_ap_password', and 'ota_password'
  - 'wifi_ws.yaml'
    * substitutions defined: 'web_server_port: 80'
      - this is the default port value, which can be overridden in device config file
    * includes 'common/wifi.yaml', so device config files just have to include this package
    * top-level fields defined: 'web_server'
    * expects and defines as given in 'common/wifi.yaml', nothing added by this package
* always use esphome to inspect the result of package includes
  - first flush cache and then config: 'esphome clean file-name.yaml'
  - just config: 'esphome -vvvv config file-name.yaml'
  - compile: 'esphome -vvvv compile file-name.yaml'
  - the 'getConfigs.sh' tool will copy all the config files into a common directory (e.g., 'SensorNet/configs/esphome')
  - the 'validateConfigs.sh' tool will make sure that the config files can be compiled -- i.e., they are syntactically correct and all of the required packages can be loaded from github
* notes on package declarations
  - the 'packages' field is a dict
    * keys: arbitrary labels (ignored by ESPHome)
    * values: package references that load and merge external YAML files into the config
  - the declarations of packages can be simplified
    * e.g., 
```
packages:
  hardware: !include packages/air-quality-pms-base.yaml
```
is equivalent to
```
packages:
  hardware:
    !include:
      file: packages/device-type-base.yaml
```
  - packages can be define by local files, urls, or github urls, by using (respectively) the 'file', 'url', and 'github' keywords
    * file: a local file, relative the the caller's location
      - e.g., 'file: common/foo.yaml'
    * url: a URL to a file that can be accessed remotely
      - e.g., 'url: https://github.com/usr/repo/raw/main/foo.yaml'
    * github: shorthand for a github URL
      - e.g., 'github: usr/repo@branch_or_tag' -- expands to full Git URL
  - packages on github are accessed via the 'github' keyword
    * e.g., 'base_conf: github://jduanen/SensorNet/sensors/common/base.yaml'
  - package merging behavior: loaded packages merge recursively
    * dictionaries by key, component lists by id, others by override (main config wins)
* ?
