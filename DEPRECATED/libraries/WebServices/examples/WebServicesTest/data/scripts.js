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

  document.getElementById('flag').checked = msgObj.flag;

  elem = document.getElementById('intVal');
  elem.value = msgObj.intVal;

  document.getElementById('str').value = escapeHTML(msgObj.str);

  setCustomColors(msgObj.tuples);

  document.getElementById("save").disabled = false;
}
function toggleCheckbox(element) {
  element.innerHTML = element.checked ? "ON" : "OFF";
}
function setValues() {
  var flag = document.getElementById('flag').checked;
  var intVal = document.getElementById('intVal').value;
  var str = document.getElementById('str').value;
  var jsonMsg = JSON.stringify({"msgType": "setValues", "flag": flag, "intVal": intVal, "str": str, 'tuples': getCustomColors()});
  websocket.send(jsonMsg);
}
function saveConfiguration() {
  var jsonMsg = JSON.stringify({'msgType': 'saveConf',
                                'ssid': document.getElementById('ssid').value,
                                'passwd': rot47(document.getElementById('password').value),
                                'flag': document.getElementById('flag').checked,
                                'intVal': document.getElementById('intVal').value,
                                'str': document.getElementById('str').value,
                                'tuples': getCustomColors()
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
function setCustomColors(tuples) {
  for (var i = 0; (i < NUMBER_OF_TUPLES); i++) {
    var elemId = 'cb' + i;
    var bkg = "linear-gradient(" + rgbIntToHex(tuples[i][0]) + ", " + rgbIntToHex(tuples[i][1]) + ")";
    document.getElementById(elemId).style.background = bkg;
  }
}
function getCustomColors() {
  var customColors = [];
  for (var i = 0; (i < NUMBER_OF_TUPLES); i++) {
    var btn = document.getElementById('cb' + i);
    if (btn.style.background == "") {
      btn.style.background = "linear-gradient(#ff0000, #0000ff)";
    }
    var s = [];
    for (m of btn.style.background.matchAll(/rgb\((\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)/g)) {
      s.push(rgbToInt(m[1], m[2], m[3]));
    }
    customColors.push(s);
  }
  return customColors;
}
function colorsButtonClick(element) {
  var startColor = parseInt(document.getElementById('startColor').value.substr(1), 16);
  var endColor = parseInt(document.getElementById('endColor').value.substr(1), 16);
  element.style.background = "linear-gradient(#" + startColor.toString(16).padStart(6, 0) + ", #" + endColor.toString(16).padStart(6, 0) + ")";
//  var jsonMsg = JSON.stringify({'msgType': 'tuples', 'tuples': getCustomColors()});
//  websocket.send(jsonMsg);
}
function clearCustomColors() {
  for (var i = 0; (i < NUMBER_OF_TUPLES); i++) {
    document.getElementById('cb' + i).style.background = "linear-gradient(#00ff00, #00ff00)";
  }
}
function rgbToInt(r, g, b) {
  return ((parseInt(r) << 16) + (parseInt(g) << 8) + parseInt(b));
}
function rgbIntToHex(c) {
  return "#" + c.toString(16).padStart(6, 0);
}
