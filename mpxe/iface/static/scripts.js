var projects = {}
var options = {} // hold return value of init call

var ws = new WebSocket("ws://192.168.24.24:8081/");
ws.onmessage = function (event) {
    console.log("data:", event.data);
};

function send_cmd(cmd, args) {
    ws.send(JSON.stringify({
        cmd: cmd,
        args: args,
    }));
}

function init() {
    send_cmd("init", {});
}


