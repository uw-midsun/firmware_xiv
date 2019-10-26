# Gettting Started

## Prerequisites

You will want to download and install the following:

* [Vagrant](https://www.vagrantup.com/downloads.html): The installer should prompt you to add `vagrant` to your system path so that it is available in terminals. If it is not found, please try logging out and logging back in
* [VirtualBox](https://www.virtualbox.org/wiki/Downloads)
* [VirtualBox Extension Pack](https://www.virtualbox.org/wiki/Downloads): select the version that matches your version of VirtualBox
* [git](https://git-scm.com/downloads)

**Note**: You may also need go into your BIOS to ensure that VT-x/AMD-v (also known as *Intel(R) Virtualization Technology*) settings are enabled.

## Installation

The idea is that [Vagrant](https://www.vagrantup.com) uses [VirtualBox](https://www.virtualbox.org) to provision a Virtual Machine for our pre-configured image, [box](https://github.com/uw-midsun/box). This Virtual Machine contains a fully integrated development environment, including everything you'll need to develop software for our team.

```bash
# Clone the repo
git clone https://github.com/uw-midsun/box.git && cd box

# Start the vagrant box and setup USB passthrough
vagrant up && vagrant reload

# Access the box
vagrant ssh
```

Congratulations! If you're now at a prompt that says something along the lines of `vagrant@midsunbox`, you're in our development environment.

We expose a shared folder between your host operating system and the virtual environment, located at `box/shared` to allow you to use your favorite text editor from your host operating system.

```bash
# You should still be ssh'd into the box
# Access the shared folder - found at box/shared
cd ~/shared

# Clone the firmware repo
git clone https://github.com/uw-midsun/firmware.git && cd firmware

# Try to build the firmware
make build_all
```

While we're at it, let's configure some settings:

```bash
# Configure git
git config --global user.name 'Your Name'
git config --global user.email 'youremail@example.com'

git config --global push.default simple
git config --global core.autocrlf input
```

Please use the email that's associated with your GitHub account.

## Usage

```bash
# Bring up the virtual machine
# This step is only necessary after you reboot your computer
vagrant up

# Access the box
vagrant ssh

# Move to the firmware folder
cd shared/firmware

# See https://github.com/uw-midsun/firmware#usage for some common commands
make [cmd] ...
```

You're done! From now on, we'll assume you're using our development environment.
