var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener("load", onLoad);
function initWebSocket() {
  console.log("Trying to open a WebSocket connection...");
  websocket = new WebSocket(gateway);
  websocket.onopen    = onOpen;
  websocket.onclose   = onClose;
  websocket.onmessage = onMessage;
}
function onOpen(event) {
  console.log("Connection opened");
  initView();
}
function onClose(event) {
  console.log("Connection closed");
  setTimeout(initWebSocket, 2000);
}
function onLoad(event) {
  initWebSocket();
}
function onMessage(event) {
  var state;
  var elem;
  const msgObj = JSON.parse(event.data);
  console.log("msgObj: " + JSON.stringify(msgObj));

  elem = document.getElementById("rssi");
  elem.innerHTML = msgObj.RSSI;
  elem.value = msgObj.RSSI;

  elem = document.getElementById("waterTemp");
  elem.innerHTML = msgObj.waterTemp;
  elem.value = msgObj.waterTemp;

  elem = document.getElementById("ambientTemp");
  elem.innerHTML = msgObj.ambientTemp;
  elem.value = msgObj.ambientTemp;

  elem = document.getElementById("ssid");
  elem.value = msgObj.ssid;

  elem = document.getElementById("password");
  if (msgObj.passwd != null) {
    elem.value = rot47(msgObj.passwd);
  } else {
    elem.value = "";
  }

  elem = document.getElementById("mqttServer");
  elem.value = msgObj.mqttServer;

  elem = document.getElementById("mqttPort");
  elem.value = msgObj.mqttPort;

  elem = document.getElementById("rate");
  elem.value = msgObj.rate;

  elem = document.getElementById("precision");
  elem.value = msgObj.precision;

  document.getElementById("save").disabled = false;
}
function toggleCheckbox(element) {
  element.innerHTML = element.checked ? "ON" : "OFF";
  var jsonMsg = JSON.stringify({"msgType": element.id, "state": element.innerHTML});
  websocket.send(jsonMsg);
}
function setRate() {
  var rate = document.getElementById("rate").value;
  var jsonMsg = JSON.stringify({"msgType": "rate", "rate": rate});
  websocket.send(jsonMsg);
}
function setPrecision() {
  var precision = document.getElementById("precision").value;
  var jsonMsg = JSON.stringify({"msgType": "precision", "precision": precision});
  websocket.send(jsonMsg);
}
function saveConfiguration() {
  var jsonMsg = JSON.stringify({"msgType": "saveConf",
                                "ssid": document.getElementById("ssid").value,
                                "passwd": rot47(document.getElementById("password").value),
                                "mqttServer": document.getElementById("mqttServer").value,
                                "mqttPort": document.getElementById("mqttPort").value,
                                "rate": document.getElementById("rate").value,
                                "precision": document.getElementById("precision").value});
  document.getElementById("save").disabled = true;
  websocket.send(jsonMsg);
}
function reboot() {
  var jsonMsg = JSON.stringify({"msgType": "reboot"});
  document.getElementById("reboot").disabled = true;
  websocket.send(jsonMsg);
  // TODO wait for a bit and try to reload the index.html page
  console.log("XXXXX");
  setTimeout(function() { console.log("Rebooting"); }, 15000);
  window .location.reload();
  console.log("YYYYY");
}
function escapeHTML(s) {
  return s.replace(/&/g, '&amp;')
      .replace(/</g, '&lt;')
      .replace(/>/g, '&gt;')
      .replace(/'/g, '&apos;')
      .replace(/"/g, '&quot;')
      .replace(/\//g, '&sol;');
}
function rot13(str) {
  return str.split('').map(char => String.fromCharCode(char.charCodeAt(0) + (char.toLowerCase() < 'n' ? 13 : -13))).join('');
}
function rot47(x) {
  var s = [];
  for(var i = 0; (i < x.length); i++) {
    var j = x.charCodeAt(i);
    if ((j >= 33) && (j <= 126)) {
      s[i] = String.fromCharCode(33 + ((j + 14) % 94));
    } else {
      s[i] = String.fromCharCode(j);
    }
  }
  return s.join('');
}
