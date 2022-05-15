var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onLoad);
function initWebSocket() {
  console.log('Trying to open a WebSocket connection...');
  websocket = new WebSocket(gateway);
  websocket.onopen    = onOpen;
  websocket.onclose   = onClose;
  websocket.onmessage = onMessage;
}
function onOpen(event) {
  console.log('Connection opened');
  initView();
}
function onClose(event) {
  console.log('Connection closed');
  setTimeout(initWebSocket, 2000);
}
function onLoad(event) {
  initWebSocket();
}
function initView() {
  var jsonMsg = JSON.stringify({"msgType": "query"});
  websocket.send(jsonMsg);
  console.log("FIXME: initView");
}
function onMessage(event) {
  var state;
  var elem;
  const msgObj = JSON.parse(event.data);
  console.log('msgObj: ', msgObj);
}
function saveConfiguration() {
  var ssid = document.getElementById("ssid").value;
  var passwd = document.getElementById("password").value;
  var jsonMsg = JSON.stringify({"msgType": "saveConf", "ssid": ssid, "passwd": passwd});
  console.log("Save configuration: " + jsonMsg);
  websocket.send(jsonMsg);
}
