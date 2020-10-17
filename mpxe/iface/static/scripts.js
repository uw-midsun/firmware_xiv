var projects = {}
var options = {} // hold return value of init call

var ws = new WebSocket("ws://192.168.24.24:8082/");
ws.onmessage = function (event) {
    console.log("data:", event.data)
};

function send() {
    ws.send("js->python");
}

function init() {
    ws.send({cmd: "init", args: {}})
}
