# firmware
[![Build Status](https://travis-ci.com/uw-midsun/firmware_xiv.svg?branch=master)](https://travis-ci.com/uw-midsun/firmware_xiv)
   
This repository contains all firmware for the [University of Waterloo](https://uwaterloo.ca/)'s [Midnight Sun Solar Rayce Car](http://www.uwmidsun.com/) team's 14th car.

## Getting Started

Assuming you have our [Vagrant box](https://github.com/uw-midsun/box) installed:

```bash
# Clone the repo
git clone https://github.com/uw-midsun/firmware.git firmware
cd firmware

# Basic commands to verify that building and testing work
make build_all PLATFORM=x86
make build_all PLATFORM=stm32f0xx
make test_all PLATFORM=x86
```

## Usage

```bash
# Commands are of the format:
# make cmd VAR=val ...

# Flashes the specified project (defaults to stm32f0xx)
# Append PROBE=stlink-v2 for discovery boards
make program PROJECT=test_project

# Run all tests within the library or project
# Append TEST=module for a specific test
make test LIBRARY=ms-common

# Flashes the project/test and attaches an instance of GDB
make gdb TEST=can LIBRARY=ms-common

# Creates a new project or library
make new PROJECT=new_project_name

# Nukes the build directory - use when things aren't working
make clean

# Linting and formatting - used to help enforce our coding style
make format
make lint
```

We use [GNU Make](https://www.gnu.org/software/make/manual/) for our build system. See [Managing Projects with GNU Make, 3.Xth Edition](http://wanderinghorse.net/computing/make/book/ManagingProjectsWithGNUMake-3.1.3.pdf) for a fantastic supplement to the manual.

Extensive documentation of our supported commands can be found in our [Makefile](Makefile). Note that commands such as `test` and `gdb` will automatically build the project if any changes have been made. You do not need to explicitly build projects except for [continuous integration](#continuous-integration).

### Optional x86 Extended Debugging

If you have Clang/LLVM/Bear installed and want to debug on x86 more easily/more in depth.

#### Static Analysis

To create a compile commands database, run

```bash
make clean
bear make build_all PLATFORM=x86
```

Then to perform static analysis, run

```bash
clang-tidy $PATH_TO_C_FILE -checks=*
```

#### Address Sanitation, Memory Analysis and Stack Pointers

To build in debug with memory and address sanitation and extended stack traces on faults, run

```bash
make clean
make build_all PLATFORM=x86 COMPILER=clang COPTIONS=asan
```

If you run any of the resulting binaries and a memory error of any kind occurs there will be detailed information on the cause.

#### Thread Sanitation

To build in debug with thread sanitation run

```bash
make clean
make build_all PLATFORM=x86 COMPILER=clang COPTIONS=tsan
```

If you run any of the resulting binaries and there is any multithreaded code this will find any race conditions.

## Continuous Integration

We use [Travis CI](https://travis-ci.org/uw-midsun) to run our continuous integration tests, which consists of linting project code, and compiling and running unit tests against each supported platform. The build matrix is used to run tests on all possible permutations of our build targets (including linting, which is listed as a target to prevent linting the same code multiple times).

To add a new target to the build matrix, simply add a new line under ``env``.

More information can be found by reading our [.travis.yml](.travis.yml) file.

## Dependencies

- GNU ARM Embedded toolchain
- GNU Make 4.0 or above
- [Unity&mdash;Throw the Switch](http://www.throwtheswitch.org/unity/): our C unit testing framework

### Optional Dependencies

- [Clang/LLVM toolchain](http://releases.llvm.org/download.html)
- [Bear (Build EAR)](https://github.com/rizsotto/Bear)

## Contributions

Before submitting an issue or a pull request to the project, please take a moment to review our code style guide first.

## License

The firmware is made available under the [MIT License](https://opensource.org/licenses/MIT).
