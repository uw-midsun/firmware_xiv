# MPXEI
This project is an Interface for MPXE.

## Goals
While MPXE itself is great for integration testing, the principle could also be used as a validation tool. The dream scenario would be to create a kind of cyborg system where some boards are connected via physical CAN bus to a laptop that's emulating a bunch of other boards, while also displaying an interface for controlling said other boards.

### Inputs
The inputs MPXEI must enable are:
- starting a project
    - specifying a sim for a project
- stopping a project
- each project should have various inputs defined per sim
    - each sim should define a list of controls it can take
        - sliders
        - buttons
        - numerical fields
- generic CAN input
- specific CAN input from the DBC file

### Outputs
The outputs MPXEI should display are:
- CAN bus output
- log output per sim
- field output per sim (e.g. current GPIO state, or last message out of an MCP2515)

### Steps
To get log output to js:
- add 'log_callbacks' and 'store_callbacks' attributes to ProjectManager that's a list of callbacks to call per log and per store
- have a global queue in server.py that take input from registering a log callback in pm, and have tx_handler consume that queue to send to js
To get input from js, these procedures are required. Can be implemented by the data model:

js sends: `{command: cmd, arguments: {...}}`, python returns: `{data: {...}}`

js needs to keep a map of all running projects, key: `fd`, val: `{name, fields, controls}`

Procedures:
- cmd: `'init'`, args: `{}`, ret: `{init, ['proj_names'], [sim: {name, [field: {name, control_type}]}], [can_msg: {name, [fields]}]}`
- cmd: `'start'`, args: `{'name', 'sim'}`, ret: `{start, proj_fd, name, sim}`
- cmd: `'stop'`, args: `{fd}`, ret: `{stop, proj}`
- cmd: `'set_field'`, args: `{fd, field, value}`
- cmd: `'send_raw'`, args: `{id, data, dlc}`
    - radio button for choosing size of each field?
- cmd: `'send_msg'`, args: `{name, [{field: value}]}`
