#!/usr/bin/env python3
"""Selects a programmer for OpenOCD if more than one is connected."""
import sys
import usb

def scrape_devices(probe, devices):
    """Scrapes device list and associated discovered devices with the specified probe type.

    Args:
        probe: String to associate discovered devices with.
        devices: Device iterator to scrape.

    Returns:
        Device information as a list of tuples of probe, name, and serial.
    """
    options = []
    for dev in devices:
        manufacturer = usb.util.get_string(dev, dev.iManufacturer)
        product = usb.util.get_string(dev, dev.iProduct)
        serial = usb.util.get_string(dev, dev.iSerialNumber)
        options.append((probe, '{} {}'.format(manufacturer, product), serial))
    return options

def get_options():
    """Retrieve connected programmers and print OpenOCD command"""
    cmsis_dap = usb.core.find(find_all=True, idProduct=0xda42, idVendor=0x1209)
    stlink_v2 = usb.core.find(find_all=True, idProduct=0x3748, idVendor=0x0483)
    options = scrape_devices('CMSIS-DAP', cmsis_dap) + scrape_devices('STLink-V2', stlink_v2)

    if len(options) > 1:
        for i, (probe, name, serial) in enumerate(options):
            print('{}: {} - {}'.format(i, probe, serial), file=sys.stderr)

        print('Select device: ', end='', file=sys.stderr)
        index = int(input())
        probe, name, serial = options[index]

        cmd = {
            'CMSIS-DAP': 'cmsis_dap_serial',
            'STLink-V2': 'hla_serial'
        }[probe]
        print('Selected {} ({})'.format(name, serial), file=sys.stderr)
        print('{} {}'.format(cmd, serial))

def main():
    """Main entry point"""
    if len(sys.argv) > 1:
        # Assume that if it's being specified, it's probably our programmer
        print('cmsis_dap_serial {}'.format(sys.argv[1]))
    else:
        get_options()

if __name__ == '__main__':
    main()
