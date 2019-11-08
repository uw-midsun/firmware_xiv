#pragma once
//
// This is a driver to read and write binary data to an SDHC card.
// The suggested usage is to initialize the SD card, and then use
// FatFs to write to it.
//
// This module requires that the SPI port which the card is mounted
// on is already initialized. As well, soft timers and interrupts
// must be initialized
//
#include <stdbool.h>
#include <stdint.h>
#include "gpio.h"
#include "spi.h"
#include "status.h"

// The block size on the SD card
#define SD_BLOCK_SIZE (512)

// For SDHC and SDXC cards, the address provided to these functions should be
// the block address

// Initialize the SD card on a given SPI port
StatusCode sd_card_init(SpiPort spi);

// Read block from the SD card. |dest| is where the read blocks will be written
// into. Make sure that this buffer is large enough for the content
StatusCode sd_read_blocks(SpiPort spi, uint8_t *dest, uint32_t readAddr, uint32_t numberOfBlocks);

// Write blocks to the SD card from |src| to a location on the SD card specified
// by |writeAddr|
StatusCode sd_write_blocks(SpiPort spi, uint8_t *src, uint32_t writeAddr, uint32_t numberOfBlocks);

// Determines whether the SD card is ready in on a given SPI port
StatusCode sd_is_initialized(SpiPort spi);
