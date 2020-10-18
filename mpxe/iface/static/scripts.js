var init_ret = null
var handlers = {}
var cmds = {}
var running_projs = {}

var logs = Array(10).fill('')
var can_logs = Array(10).fill('')

function control_factory(control_type, elem_id, init_val, fd) {
    if (control_type == 'slider') {
        return ''
    } else if (control_type == 'toggle') {
        return ''
    } else if (control_type == 'number') {
        return ''
    }
}

function display_factory(display_type, elem_id, init_val, fd) {
    if (display_type == 'boolean') {
        return `<p id="${elem_id}_${fd}">${init_val ? '1' : '0'}</p>`
    } else if (display_type == 'text') {
        return ''
    } else if (display_type == 'number') {
        return ''
    }
}

var ws = new WebSocket('ws://192.168.24.24:8081/')
ws.onmessage = function (event) {
    data = JSON.parse(event.data)
    handlers[data['update_type']](data['data'])
}

handlers['init_ret'] = (data) => {
    console.log('handling init_ret')
    init_ret = data
    let proj_form_str = `
        <form>
            <label for="projs">Projects:</label>
            <select name="projs" id=projs selected="controller_board_blinking_leds">
            </select>
        </form>
    `
    let sim_form_str = `
        <form>
            <label for="sims">Sims:</label>
            <select name="sims" id=sims>
            </select>
        </form>
    `
    let body = document.getElementsByTagName('body')[0]
    body.insertAdjacentHTML('beforeend', proj_form_str)
    body.insertAdjacentHTML('beforeend', sim_form_str)
    proj_select = document.getElementById('projs')
    sim_select = document.getElementById('sims')
    init_ret['projs'].forEach(proj => {
        opt_str = `<option value="${proj}">${proj}</option>`
        proj_select.insertAdjacentHTML('beforeend', opt_str)
    })
    init_ret['sims'].forEach(sim => {
        opt_str = `<option value="${sim}">${sim}</option>`
        sim_select.insertAdjacentHTML('beforeend', opt_str)
    })
    let running_projs_str = '<div id="running_projs"><b>Running Projs<b></div>'
    body.insertAdjacentHTML('beforeend', running_projs_str)
}

handlers['start_ret'] = (data) => {
    console.log('handling start_ret')
    running_projs[data['fd']] = data
    let running_projs_div = document.getElementById('running_projs')
    let proj_div_str = `<div id="running_${data['fd']}">
        <p>Project: ${data['name']}_${data['fd']}</p>
        <p>Sim: ${data['sim_name']}</p>
    </div>`
    running_projs_div.insertAdjacentHTML('beforeend', proj_div_str)
    proj_div = document.getElementById(`running_${data['fd']}`)
    controls = data['controls']
    displays = data['displays']
    running_projs[data['fd']]['control_ids'] = []
    running_projs[data['fd']]['display_ids'] = []
    if (Object.keys(controls).length !== 0) {
        for ([k, v] of Object.entries(controls)) {
            let add_control = (control_type, control_id, control_value) => {
                control = control_factory(control_type, control_id, control_value, data['fd'])
                running_projs[data['fd']]['control_ids'].push(control_id)
                proj_div.insertAdjacentHTML('beforeend', control)
            }
            if (Array.isArray(v)) {
                v.forEach((elem, ind) => {
                    add_control(elem['type'], `${k}_${ind}`, v['value'])
                })
            } else {
                add_control(elem['type'], k, v['value'])
            }
        }
    }
    if (Object.keys(displays).length !== 0) {
        for ([k, v] of Object.entries(displays)) {
            let add_display = (display_type, display_id, display_value) => {
                display = display_factory(display_type, display_id, display_value, data['fd'])
                running_projs[data['fd']]['display_ids'].push(display_id)
                proj_div.insertAdjacentHTML('beforeend', display)
            }
            if (Array.isArray(v)) {
                v.forEach((elem, ind) => {
                    add_display(elem['type'], `${k}_${ind}`, v['value'])
                })
            } else {
                add_display(elem['type'], k, v['value'])
            }
        }
    }
    // console.log(data)
}

handlers['stop_ret'] = (data) => {
    console.log('handling stop_ret')
    console.log(data)
}

handlers['log'] = (data) => {
    log_str = `[${data['name']}_${data['fd']}]${data['log']}`
    logs.unshift(log_str)
    logs.pop()
    update_logs()
}

handlers['displays'] = (data) => {
    proj = running_projs[data['fd']]
    for([k, v] of Object.entries(data['displays'])) {
        let update_display = (e_id, val) => {
            let e = document.getElementById(e_id)
            e.innerHTML = val
        }
        if (Array.isArray(v)) {
            v.forEach((elem, ind) => {
                update_display(`${k}_${ind}_${data['fd']}`, elem['value'])
            })
        } else {
            update_display(`${k}_${data['fd']}`, v['value'])
        }
    }
}

handlers['can'] = (data) => {
    can_log_str = `[${data['name']}#${data['id']}]${data['data']}`
    can_logs.unshift(can_log_str)
    can_logs.pop()
    update_can_logs()
}

function update_logs() {
    logs_ul = document.getElementById('logs')
    logs_ul.innerHTML = ''
    logs.forEach(log => {
        logs_ul.insertAdjacentHTML('beforeend', `<li>${log}</li>`)
    })
}

function update_can_logs() {
    can_logs_ul = document.getElementById('can_logs')
    can_logs_ul.innerHTML = ''
    can_logs.forEach(can_log => {
        can_logs_ul.insertAdjacentHTML('beforeend', `<li>${can_log}</li>`)
    })
}

cmds['init'] = () => {
    send_cmd('init', {})
    // add div with 10 recent logs
    // add div with 10 recent CAN messages
    let start_btn_str = '<button onclick="cmds[\'start\']()">Start</button>'
    let stop_btn_str = '<button onclick="cmds[\'stop\']()">Stop</button>'
    let body = document.getElementsByTagName('body')[0]
    body.insertAdjacentHTML('beforeend', start_btn_str)
    body.insertAdjacentHTML('beforeend', stop_btn_str)
    let logs_str = '<p>Logs</p><ul id="logs"></ul>'
    let can_logs_str = '<p>CAN logs</p><ul id="can_logs"></ul>'
    body.insertAdjacentHTML('beforeend', logs_str)
    body.insertAdjacentHTML('beforeend', can_logs_str)
    update_logs()
    update_can_logs()
}

cmds['start'] = () => {
    let projs = document.getElementById('projs')
    let sims = document.getElementById('sims')
    let selected_proj = projs.options[projs.selectedIndex].text
    let selected_sim = sims.options[sims.selectedIndex].text
    // console.log(`start sim: ${selected_sim}, proj: ${selected_proj}`)
    send_cmd('start', {name: selected_proj, sim_name: selected_sim})
}

function send_cmd(cmd, args) {
    ws.send(JSON.stringify({
        cmd: cmd,
        args: args,
    }))
}
