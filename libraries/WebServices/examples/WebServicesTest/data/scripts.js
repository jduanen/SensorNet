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

  document.getElementById('ssid').value = msgObj.ssid;

  elem = document.getElementById("password");
  if (msgObj.passwd != null) {
    elem.value = rot47(msgObj.passwd);
  } else {
    elem.value = "";
  }

  document.getElementById('flag').checked = (msgObj.flag == "true");

  elem = document.getElementById('intVal');
  elem.value = msgObj.intVal;
}
function toggleCheckbox(element) {
  element.innerHTML = element.checked ? "ON" : "OFF";
  setValues();
}
function setValues() {
  var flag = document.getElementById('flag').checked;
  var intVal = document.getElementById('intVal').value;
  var jsonMsg = JSON.stringify({"msgType": "setValues", "flag": flag, "intVal": intVal});
  websocket.send(jsonMsg);
}
function saveConfiguration() {
  var jsonMsg = JSON.stringify({'msgType': 'saveConf',
                                'ssid': document.getElementById('ssid').value,
                                'passwd': rot47(document.getElementById('password').value),
                                'flag': document.getElementById('flag').checked,
                                'intVal': document.getElementById('intVal').value
                              });
  document.getElementById('save').disabled = true;
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
