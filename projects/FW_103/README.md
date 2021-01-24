Datasheet scavenger hunt. Hint: ctrl + f is your friend!

# What bit in what register should be set on the MCP2515 CAN controller to enable one-shot mode?
 Bit 3, Register 10-1 (Address 0xXF), 1 = enabled, 0 = disabled. Source: https://ww1.microchip.com/downloads/en/DeviceDoc/MCP2515-Stand-Alone-CAN-Controller-with-SPI-20001801J.pdf


# In the MCP2515, what are the 3 bytes you’d send over spi if you wanted to write 0x17 to register 0x0f?
Byte 1: 0000 0010 (instruction)
Byte 2: 0001 0111 (0x17)
Byte 3: 0000 1111 (0x0f)
Source: https://ww1.microchip.com/downloads/en/DeviceDoc/MCP2515-Stand-Alone-CAN-Controller-with-SPI-20001801J.pdf Page 68, Figure 12-4


# What is the digital output code (16 bits) for the MCP3427 when the input voltage is 0
Input Voltage (CHn+ - CHn-)*PGA = 0
Digital Output Code = 0000000000000000


# What is the first Opcode byte for to start a conversion on the  ADS1259
Byte: 0000 100x (where x is any)
Hex Representation: 0x08 or 0x09 (since x is any)
Source: https://www.ti.com/lit/ds/symlink/ads1259.pdf?ts=1611514736643&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FADS1259 page 32


# What is the address (hex) for GPIO in the MCP23008?
Address: 0x09
Source: https://ww1.microchip.com/downloads/en/DeviceDoc/21919e.pdf. page 6


# What are the fault codes for the when a fault is detected in bits 15-12 LTC6811?
 Fault code (binary): 0b1xxx (where x is any)

===========================================================================================================================

## Schematic scavenger hunt. Note: MSXII schematics are used because MSXIV schematics are undergoing constant revision.

# On the MSXII AFE board temperature mux, how many inputs are there? How many pins are needed to talk to the controller board?
Weirdly, I could only find the MSXIV schematic (thus contradicting the above statement: https://university-of-waterloo-solar-car-team.365.altium.com/designs/9D181E70-999D-4D1E-8C98-3B5285CC6775?variant=%5BNo%20Variations%5D#design)

32 Inputs from thermistors
3 pins to talk to the board (SYNC, DIN, SCLK)

# On the MSXII pedal board, does the controller board talk to the ADC via SPI, CAN, I2C, or UART?
Taking the latest schem. here: https://university-of-waterloo-solar-car-team.365.altium.com/designs/69B56210-BCA1-4737-AD11-6778172C6E7C#design

I2C

# What communication protocol is used by the BMS carrier to communicate with the AFE and current sense board?
 SPI

# What port and pin are used for the CAN interrupt for the charger interface board?
 Port 3, Pin 7 (PA8_nCAN_INTERRUPT

Source: https://university-of-waterloo-solar-car-team.365.altium.com/designs/00B79E88-10D2-422B-B663-5EBDA10B38AD?variant=%5BNo%20Variations%5D#design

# What port and pin control the horn in the steering wheel board?
 Port 1, Pin 21 (PB1_HORN)

Source: https://university-of-waterloo-solar-car-team.365.altium.com/designs/697BEC10-E03D-4856-A1CE-A7881A4CAA18?variant=%5BNo%20Variations%5D#design

# How is data transmitted from the MicroSD_Reader to other parts of the telemetry board?
 Using the SPI protocol (https://uwmidsun.atlassian.net/wiki/spaces/ELEC/pages/64749569/SD+Cards)
