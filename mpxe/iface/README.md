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
