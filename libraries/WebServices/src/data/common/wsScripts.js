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
function initView() {
  var jsonMsg = JSON.stringify({"msgType": "query"});
  websocket.send(jsonMsg);
  var wifiMode = "%WIFI_MODE%";
  var elem;
  switch (wifiMode) {
    case "STA":
      elem = document.getElementById("ipAddr");
      if (elem == null) {
        var p1 = "<p class=\"staticState\">IP Address: <span style=\"color:blue\" id=\"ipAddr\">%IP_ADDR%</span></p>";
        document.getElementById("wifiMode").insertAdjacentHTML('afterend', p1);
      }
      elem = document.getElementById("connection");
      if (elem == null) {
        var p2 = "<p class=\"staticState\">Connection: <span style=\"color:blue\" id=\"connection\">%CONNECTION%</span></p>";
        document.getElementById("ipAddr").insertAdjacentHTML('afterend', p2);
      }
      break;
    case "A_P":
      elem = document.getElementById("wifiApSsid");
      if (elem == null) {
        var p1 = "<p class=\"staticState\">AP SSID: <span style=\"color:blue\" id=\"wifiApSsid\">%WIFI_AP_SSID%</span></p>";
        document.getElementById("wifiMode").insertAdjacentHTML('afterend', p1);
      }
      break;
    default:
      console.log("WARNING: invalid WIFI mode: " + wifiMode);
  }
}
function onMessage(event) {
  var state;
  var elem;
  const msgObj = JSON.parse(event.data);
  console.log("msgObj: " + JSON.stringify(msgObj));
  elem = document.getElementById("ssid");
  elem.value = msgObj.ssid;
  elem = document.getElementById("password");
  if (msgObj.passwd != null) {
    elem.value = rot47(msgObj.passwd);
  } else {
    elem.value = "";
  }
  document.getElementById("save").disabled = false;
}
function toggleCheckbox(element) {
  element.innerHTML = element.checked ? "ON" : "OFF";
  var jsonMsg = JSON.stringify({"msgType": element.id, "state": element.innerHTML});
  websocket.send(jsonMsg);
}
function saveConfiguration() {
  var ssid = document.getElementById("ssid").value;
  var passwd = document.getElementById("password").value;
  var jsonMsg = JSON.stringify({"msgType": "saveConf", "ssid": ssid, "passwd": rot47(passwd)});
  document.getElementById("save").disabled = true;
  websocket.send(jsonMsg);
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
