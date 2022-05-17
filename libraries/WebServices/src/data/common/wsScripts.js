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
  console.log("FIXME: initView");
}
function onMessage(event) {
  console.log("msgObj: " + event.data);
  const msgObj = JSON.parse(event.data);
  document.getElementById("libVersion").textContent = escapeHTML(msgObj.libVersion);
  document.getElementById("ipAddr").textContent = escapeHTML(msgObj.ipAddr);
  document.getElementById("connected").textContent = escapeHTML(msgObj.connected);
  document.getElementById("rssi").textContent = escapeHTML(msgObj.RSSI);
}
function saveConfiguration() {
  var ssid = document.getElementById("ssid").value;
  if ((typeof ssid == "undefined") || (ssid == "")) {
    console.log("Undefined SSID, not saving");
    return;
  }
  var passwd = document.getElementById("password").value;
  if ((typeof passwd == "undefined") || (passwd == "")) {
    console.log("Undefined password, not saving");
    return;
  }
  //// TODO do some simple obfuscation of the password -- e.g., rot13 so at least it's not in the clear
  var jsonMsg = JSON.stringify({"msgType": "saveConf", "ssid": ssid, "passwd": passwd});
  console.log("Save configuration: " + jsonMsg);  //// TMP TMP TMP
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
