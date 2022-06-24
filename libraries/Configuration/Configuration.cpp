/*
* Library that handles application configuration
*/

//// TODO

//// TODO template for configstate initialization

/*
  bool dirty = false;
  //// FIXME DRY this up
  if (config.containsKey("?") && !config["?"].isNull()) {
      configState.? = String(config["?"]);
  } else {
    config["?"] = configState.?;
    dirty = true;
  }
  ...
  if (dirty) {
    cs.set(config);
    cs.save();
  }
  Serial.println("Config:");
  serializeJsonPretty(config, Serial);
  Serial.println("");
*/

/*
  config = cs.get();
  JsonObject obj = config.as<JsonObject>();
  if (obj.isNull()) {
    Serial.println("No config file, initialize with an empty one");
    cs.initialize();
    deserializeJson(config, "{}");
    cs.set(config);
    cs.save();
  }
*/

