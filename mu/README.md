# MU
MU, or eMUlator, is a firmware system emulator meant to enable automated integration tests and facilitate prototyping, all on an x86 platform rather than on hardware.

## Motivation
Our codebase has had a lot of time and effort put into making projects run on x86, especially around emulating interrupts and the CAN bus. However, there hasn't been any work put into running multiple projects together. 

We could simply run multiple projects in different terminals, but we wouldn't be able to simulate events like GPIO pin switches or variable ADC readings.

MU aims to provide a python API for starting, stopping, instrumenting, and manipulating projects running on x86.

## Goals
### **Direct data access**
Internal driver state should be directly accessible to allow for assertions like "when this ADC reading is taken on this project, this GPIO pin should turn off on this other project". This would be easier if things were wired up on hardware with debug LEDs or other instruments, but in firmware the state is hidden and inaccessible on x86.

### **Simple driver-level implementation**
Instrumenting each driver (e.g. GPIO, ADC, any external IC APIs) would inevitably require re-writing parts of it. This process should be kept as simple as possible to avoid negating the benefits of automated integration tests by requiring hours of work to setup. This is currently achieved by using protobufs directly as the data structure, so updating and exporting them can happen seamlessly.

### **Flexible python interface**
Many hardware interactions need to be simulated, such as some boards being turned on and off by other boards, or some ICs being used in multiple projects for different purposes. Because of this, a flexible API is required to allow glue scripts to emulate the hardware interactions that firmware is oblivious to.

### **Not mimicing IC internals**
Although it would be neat to view every I2C and SPI command sent and received, it's infeasible to directly mock external chips like our pedal board ADC or battery AFEs because of their high complexity. No matter how closely you read the datasheet misunderstandings will always slip through, and attempting to directly emulate these would cause more trouble than it's worth. Thus, MU is for testing that high level sequencing works as expected, not that hardware interactions work.

## Architecture
There are two major parts to make this work: the python `mu/harness` that manages running projects, and the `libraries/mu-gen` library that drivers call directly to interface with the harness. Additionally, google's Protocol Buffers (protobufs) are used for cross-language communication.

### **Harness**
The python harness provides an interface for starting, stopping, logging, and getting/setting data from projects. It makes use of python's subprocess built-in library for running projects, and the poll library for communicating via protobufs and logs.

### **C Library**
The C library provides an api to manage Stores for drivers. A Store is a protobuf message type that's used to store a driver's internal state, and is exported via named pipe (unique per project) to the harness whenever there's a change in state. The library also runs a separate thread to listen for updates on `stdin` from the harness and updates the appropriate store.

There aren't any instances of both the harness and the library writing to the same value at once. This mimics the hardware: either the microcontroller is reading a value and something external is setting it, or the microcontroller is setting a value that something else acts on. Thus, getting and setting store values is lock free.

### **Protobufs**
Because C provides no runtime type information, stores (e.g. GPIO, ADC) are encapsulated in a `StoreInfo` protobuf message whenever they're passed between the library and the harness, which includes some type information. To allow for polymorphism between message types, each driver registers a set of generically-typed function pointers with the library to handle each store type.

## Usage
A number of requirements must be installed, check [this confluence page] for details

[this confluence page]: https://uwmidsun.atlassian.net/wiki/spaces/ELEC/pages/1713471489/MU+requirements+installation

Run `make mu`.

## Future Enhancements:
- Add support for the rest of the `libraries/ms-common` APIs
- Build out a webapp interface to facilitate prototyping / experimentation
