

var ws = new WebSocket("ws://192.168.24.24:8082/");
ws.onmessage = function (event) {
    console.log("data:", event.data)
};
function send() {
    ws.send("js->python");
}
