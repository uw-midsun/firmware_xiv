/**
 *******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *notice, this list of conditions and the following disclaimer in the
 *documentation and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

#include "sd_binary.h"
#include <string.h>
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "spi.h"

// The size of a command frame for the SD card
#define SD_SEND_SIZE 6

// The amount of times to send a dummy byte before and/or after raising the CS
// line
#define SD_DUMMY_COUNT_CONST 8

// Amount of times to retry when doing initialization
#define SD_NUM_RETRIES 100

// The default byte to send
#define SD_DUMMY_BYTE (0xFF)

// Misc definitions for SD card things
#define SD_R1_NO_ERROR (0x00)
#define SD_R1_IN_IDLE_STATE (0x01)
#define SD_R1_ILLEGAL_COMMAND (0x04)

#define SD_TOKEN_START_DATA_SINGLE_BLOCK_READ (0xFE)
#define SD_TOKEN_START_DATA_SINGLE_BLOCK_WRITE (0xFE)
#define SD_TOKEN_START_DATA_MULTI_BLOCK_WRITE (0xFC)
#define SD_TOKEN_STOP_DATA_MULTI_BLOCK_WRITE (0xFD)

#define SD_CMD_GO_IDLE_STATE (0)
#define SD_CMD_SEND_IF_COND (8)
#define SD_CMD_STATUS (13)
#define SD_CMD_SET_BLOCKLEN (16)
#define SD_CMD_READ_SINGLE_BLOCK (17)
#define SD_CMD_WRITE_SINGLE_BLOCK (24)
#define SD_CMD_WRITE_MULTI_BLOCK (25)
#define SD_CMD_SD_APP_OP_COND (41)
#define SD_CMD_APP_CMD (55)
#define SD_CMD_READ_OCR (58)

#define SD_DATA_OK (0x05)
#define SD_DATA_CRC_ERROR (0x0B)
#define SD_DATA_WRITE_ERROR (0x0D)
#define SD_DATA_OTHER_ERROR (0xFF)

typedef enum {
  SD_RESPONSE_R1 = 0,
  SD_RESPONSE_R1B,
  SD_RESPONSE_R2,
  SD_RESPONSE_R3,
  SD_RESPONSE_R4R5,
  SD_RESPONSE_R7,
  NUM_SD_RESPONSES
} SdResponseType;

typedef struct SdResponse {
  uint8_t r1;
  uint8_t r2;
  uint8_t r3;
  uint8_t r4;
  uint8_t r5;
} SdResponse;

static uint8_t prv_read_byte(SpiPort spi) {
  uint8_t result = 0x00;
  spi_rx(spi, &result, 1, 0xFF);
  return result;
}

static void prv_write_dummy(SpiPort spi, uint8_t times) {
  for (uint8_t i = 0; i < times; i++) {
    prv_read_byte(spi);
  }
}

static uint8_t prv_write_read_byte(SpiPort spi, uint8_t byte) {
  uint8_t result = 0x00;
  spi_rx(spi, &result, 1, byte);
  return result;
}

static uint8_t prv_wait_byte(SpiPort spi) {
  uint8_t timeout = 0x08;
  uint8_t readvalue;

  do {
    readvalue = prv_write_read_byte(spi, SD_DUMMY_BYTE);
    timeout--;
  } while ((readvalue == SD_DUMMY_BYTE) && timeout);

  return readvalue;
}

static SdResponse prv_send_cmd(SpiPort spi, uint8_t cmd, uint32_t arg, uint8_t crc,
                               SdResponseType expected) {
  uint8_t frame[SD_SEND_SIZE];

  // Split the cmd parameter into 8 byte ints
  frame[0] = (cmd | 0x40);
  frame[1] = (uint8_t)(arg >> 24);
  frame[2] = (uint8_t)(arg >> 16);
  frame[3] = (uint8_t)(arg >> 8);
  frame[4] = (uint8_t)(arg);
  frame[5] = (uint8_t)(crc);

  prv_write_dummy(spi, SD_DUMMY_COUNT_CONST);

  spi_cs_set_state(spi, GPIO_STATE_LOW);

  prv_write_dummy(spi, SD_DUMMY_COUNT_CONST);

  spi_tx(spi, frame, SD_SEND_SIZE);

  SdResponse res = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  switch (expected) {
    case SD_RESPONSE_R1:
      res.r1 = prv_wait_byte(spi);
      break;
    case SD_RESPONSE_R1B:
      res.r1 = prv_wait_byte(spi);
      res.r2 = prv_write_read_byte(spi, SD_DUMMY_BYTE);
      spi_cs_set_state(spi, GPIO_STATE_HIGH);
      delay_ms(1);
      spi_cs_set_state(spi, GPIO_STATE_LOW);
      while (prv_write_read_byte(spi, SD_DUMMY_BYTE) != 0xFF) {
      }
      break;
    case SD_RESPONSE_R2:
      res.r1 = prv_wait_byte(spi);
      res.r2 = prv_write_read_byte(spi, SD_DUMMY_BYTE);
      break;
    case SD_RESPONSE_R3:
    case SD_RESPONSE_R7:
      res.r1 = prv_wait_byte(spi);
      res.r2 = prv_write_read_byte(spi, SD_DUMMY_BYTE);
      res.r3 = prv_write_read_byte(spi, SD_DUMMY_BYTE);
      res.r4 = prv_write_read_byte(spi, SD_DUMMY_BYTE);
      res.r5 = prv_write_read_byte(spi, SD_DUMMY_BYTE);
      break;
    default:
      break;
  }
  return res;
}

static StatusCode prv_sd_get_data_response(SpiPort spi) {
  volatile uint8_t dataresponse;
  uint16_t timeout = 0xFFFF;
  while ((dataresponse = prv_read_byte(spi)) == 0xFF && timeout) {
    timeout--;
  }

  // Consumes the busy response byte
  prv_read_byte(spi);

  // Masks the bits which are not part of the response and
  // parses the response
  if ((dataresponse & 0x1F) == SD_DATA_OK) {
    spi_cs_set_state(spi, GPIO_STATE_HIGH);
    spi_cs_set_state(spi, GPIO_STATE_LOW);

    // Wait for IO line to return to 0xFF
    while (prv_read_byte(spi) != 0xFF) {
    }
    return STATUS_CODE_OK;
  }

  return status_code(STATUS_CODE_INTERNAL_ERROR);
}

static void prv_pulse_idle(SpiPort spi) {
  spi_cs_set_state(spi, GPIO_STATE_HIGH);
  prv_write_read_byte(spi, SD_DUMMY_BYTE);
}

StatusCode sd_card_init(SpiPort spi) {
  volatile SdResponse response = { 0, 0, 0, 0, 0 };
  volatile uint16_t counter = 0;
  // Send CMD0 (SD_CMD_GO_IDLE_STATE) to put SD in SPI mode and
  // wait for In Idle State Response (R1 Format) equal to 0x01
  prv_write_dummy(spi, 10);
  do {
    counter++;
    response = prv_send_cmd(spi, SD_CMD_GO_IDLE_STATE, 0, 0x95, SD_RESPONSE_R1);
    spi_cs_set_state(spi, GPIO_STATE_HIGH);
    if (counter >= SD_NUM_RETRIES) {
      return status_msg(STATUS_CODE_TIMEOUT, "Fail to init SD card before timeout\n");
    }
    delay_ms(20);
  } while (response.r1 != SD_R1_IN_IDLE_STATE);

  // Send CMD8 (SD_CMD_SEND_IF_COND) to check the power supply status
  // and wait until response (R7 Format) equal to 0xAA and
  response = prv_send_cmd(spi, SD_CMD_SEND_IF_COND, 0x1AA, 0x87, SD_RESPONSE_R7);
  spi_cs_set_state(spi, GPIO_STATE_HIGH);
  prv_write_read_byte(spi, SD_DUMMY_BYTE);
  if (response.r1 == SD_R1_IN_IDLE_STATE) {
    // initialise card V2
    do {
      // Send CMD55 (SD_CMD_APP_CMD) before any ACMD command: R1 response (0x00:
      // no errors)
      response = prv_send_cmd(spi, SD_CMD_APP_CMD, 0, 0x65, SD_RESPONSE_R1);
      spi_cs_set_state(spi, GPIO_STATE_HIGH);
      prv_write_read_byte(spi, SD_DUMMY_BYTE);

      // Send ACMD41 (SD_CMD_SD_APP_OP_COND) to initialize SDHC or SDXC cards:
      // R1 response (0x00: no errors)
      response = prv_send_cmd(spi, SD_CMD_SD_APP_OP_COND, 0x40000000, 0x77, SD_RESPONSE_R1);
      spi_cs_set_state(spi, GPIO_STATE_HIGH);
      prv_write_read_byte(spi, SD_DUMMY_BYTE);
    } while (response.r1 == SD_R1_IN_IDLE_STATE);

    if ((response.r1 & SD_R1_ILLEGAL_COMMAND) == SD_R1_ILLEGAL_COMMAND) {
      do {
        // Send CMD55 (SD_CMD_APP_CMD) before any ACMD command: R1 response
        // (0x00: no errors) */
        response = prv_send_cmd(spi, SD_CMD_APP_CMD, 0, 0x65, SD_RESPONSE_R1);
        spi_cs_set_state(spi, GPIO_STATE_HIGH);
        prv_write_read_byte(spi, SD_DUMMY_BYTE);
        if (response.r1 != SD_R1_IN_IDLE_STATE) {
          return status_msg(STATUS_CODE_INTERNAL_ERROR, "SD card is not in idle state\n");
        }
        // Send ACMD41 (SD_CMD_SD_APP_OP_COND) to initialize SDHC or SDXC cards:
        // R1 response (0x00: no errors)
        response = prv_send_cmd(spi, SD_CMD_SD_APP_OP_COND, 0x40000000, 0x77, SD_RESPONSE_R1);
        spi_cs_set_state(spi, GPIO_STATE_HIGH);
        prv_write_read_byte(spi, SD_DUMMY_BYTE);
      } while (response.r1 == SD_R1_IN_IDLE_STATE);
    }

    // Send CMD58 (SD_CMD_READ_OCR) to initialize SDHC or SDXC cards: R3
    // response (0x00: no errors)
    response = prv_send_cmd(spi, SD_CMD_READ_OCR, 0x00000000, 0xFF, SD_RESPONSE_R3);
    spi_cs_set_state(spi, GPIO_STATE_HIGH);
    prv_write_read_byte(spi, SD_DUMMY_BYTE);
    if (response.r1 != SD_R1_NO_ERROR) {
      return status_msg(STATUS_CODE_INTERNAL_ERROR, "Could not init SDHC or SDXC\n");
    }
  } else {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "SD card is not in idle state\n");
  }
  return STATUS_CODE_OK;
}

StatusCode sd_wait_data(SpiPort spi, uint8_t data) {
  uint16_t timeout = 0xFFF;
  uint8_t readvalue;

  // Check if response matches data

  do {
    readvalue = prv_read_byte(spi);
    timeout--;
  } while ((readvalue != data) && timeout);

  if (timeout == 0) {
    // After time out
    return status_msg(STATUS_CODE_TIMEOUT, "Timed out while waiting\n");
  }

  return STATUS_CODE_OK;
}

StatusCode sd_read_blocks(SpiPort spi, uint8_t *dest, uint32_t ReadAddr, uint32_t NumberOfBlocks) {
  uint32_t offset = 0;
  SdResponse response;

  // Send CMD16 (SD_CMD_SET_BLOCKLEN) to set the size of the block and
  // Check if the SD acknowledged the set block length command: R1 response
  // (0x00: no errors)
  response = prv_send_cmd(spi, SD_CMD_SET_BLOCKLEN, SD_BLOCK_SIZE, 0xFF, SD_RESPONSE_R1);
  prv_pulse_idle(spi);

  if (response.r1 != SD_R1_NO_ERROR) {
    prv_pulse_idle(spi);
    return status_msg(STATUS_CODE_INTERNAL_ERROR,
                      "Failed to read because SD card responded with an error\n");
  }

  // Data transfer
  while (NumberOfBlocks--) {
    // Send CMD17 (SD_CMD_READ_SINGLE_BLOCK) to read one block
    // Check if the SD acknowledged the read block command: R1 response (0x00:
    // no errors)
    response = prv_send_cmd(spi, SD_CMD_READ_SINGLE_BLOCK, (ReadAddr + offset) / SD_BLOCK_SIZE,
                            0xFF, SD_RESPONSE_R1);
    if (response.r1 != SD_R1_NO_ERROR) {
      prv_pulse_idle(spi);
      return status_msg(STATUS_CODE_INTERNAL_ERROR,
                        "Failed to read because SD card responded with an error\n");
    }

    // Now look for the data token to signify the start of the data
    if (status_ok(sd_wait_data(spi, SD_TOKEN_START_DATA_SINGLE_BLOCK_READ))) {
      // Read the SD block data : read 512 bytes of data
      spi_rx(spi, dest + offset, SD_BLOCK_SIZE, 0xFF);

      // Set next read address
      offset += SD_BLOCK_SIZE;
      // get CRC bytes (not really needed by us, but required by SD)
      prv_write_dummy(spi, 3);
    } else {
      prv_pulse_idle(spi);
      return status_msg(STATUS_CODE_TIMEOUT, "SD card timeout\n");
    }

    // Sets the CS line to high to end the read transaction
    spi_cs_set_state(spi, GPIO_STATE_HIGH);
    prv_write_read_byte(spi, SD_DUMMY_BYTE);
  }

  prv_pulse_idle(spi);
  return STATUS_CODE_OK;
}

static StatusCode prv_sd_write_block(SpiPort spi, uint8_t *src, uint32_t WriteAddr) {
  SdResponse response;

  // Send CMD16 (SD_CMD_SET_BLOCKLEN) to set the size of the block and
  // Check if the SD acknowledged the set block length command: R1 response
  // (0x00: no errors)
  response = prv_send_cmd(spi, SD_CMD_SET_BLOCKLEN, SD_BLOCK_SIZE, 0xFF, SD_RESPONSE_R1);
  prv_pulse_idle(spi);
  if (response.r1 != SD_R1_NO_ERROR) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "SD card error\n");
  }

  // Send CMD24 (SD_CMD_WRITE_SINGLE_BLOCK) to write blocks  and
  // Check if the SD acknowledged the write block command: R1 response (0x00: no
  // errors)

  response =
      prv_send_cmd(spi, SD_CMD_WRITE_SINGLE_BLOCK, WriteAddr / SD_BLOCK_SIZE, 0xFF, SD_RESPONSE_R1);
  if (response.r1 != SD_R1_NO_ERROR) {
    prv_pulse_idle(spi);
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "SD card error\n");
  }

  prv_write_dummy(spi, SD_DUMMY_COUNT_CONST);

  // Send the data token to signify the start of the data
  uint8_t dat = SD_TOKEN_START_DATA_SINGLE_BLOCK_WRITE;
  spi_tx(spi, &dat, 1);

  // Write the block data to SD
  spi_tx(spi, src, SD_BLOCK_SIZE);

  // Put CRC bytes (not really needed by us, but required by SD)
  uint8_t crc = 0x00;
  spi_tx(spi, &crc, 1);
  spi_tx(spi, &crc, 1);

  if (!status_ok(prv_sd_get_data_response(spi))) {
    // Quit and return failed status
    prv_pulse_idle(spi);
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "SD card error\n");
  }

  prv_pulse_idle(spi);
  return STATUS_CODE_OK;
}

StatusCode sd_write_blocks(SpiPort spi, uint8_t *src, uint32_t WriteAddr, uint32_t NumberOfBlocks) {
  if (!NumberOfBlocks) {
    return prv_sd_write_block(spi, src, WriteAddr);
  }

  uint32_t offset = 0;
  SdResponse response;

  // Send CMD16 (SD_CMD_SET_BLOCKLEN) to set the size of the block and
  // Check if the SD acknowledged the set block length command: R1 response
  // (0x00: no errors)
  response = prv_send_cmd(spi, SD_CMD_SET_BLOCKLEN, SD_BLOCK_SIZE, 0xFF, SD_RESPONSE_R1);
  prv_pulse_idle(spi);
  if (response.r1 != SD_R1_NO_ERROR) {
    return status_code(STATUS_CODE_INTERNAL_ERROR);
  }

  // Data transfer
  response = prv_send_cmd(spi, SD_CMD_WRITE_MULTI_BLOCK, (WriteAddr + offset) / SD_BLOCK_SIZE, 0xFF,
                          SD_RESPONSE_R1);
  if (response.r1 != SD_R1_NO_ERROR) {
    prv_pulse_idle(spi);
    return status_code(STATUS_CODE_INTERNAL_ERROR);
  }

  prv_write_dummy(spi, SD_DUMMY_COUNT_CONST);

  while (NumberOfBlocks--) {
    // Send CMD24 (SD_CMD_WRITE_SINGLE_BLOCK) to write blocks  and
    // Check if the SD acknowledged the write block command: R1 response (0x00:
    // no errors)

    // Send the data token to signify the start of the data
    uint8_t dat = SD_TOKEN_START_DATA_MULTI_BLOCK_WRITE;
    spi_tx(spi, &dat, 1);

    // Write the block data to SD
    spi_tx(spi, src + offset, SD_BLOCK_SIZE);

    // Set next write address
    offset += SD_BLOCK_SIZE;

    // Put CRC bytes (not really needed by us, but required by SD)
    uint8_t crc = 0x00;
    spi_tx(spi, &crc, 1);
    spi_tx(spi, &crc, 1);

    if (!status_ok(prv_sd_get_data_response(spi))) {
      // Quit and return failed status
      prv_pulse_idle(spi);
      return status_code(STATUS_CODE_INTERNAL_ERROR);
    }
  }

  // Write the block data to SD
  uint8_t end_transmission = SD_TOKEN_STOP_DATA_MULTI_BLOCK_WRITE;
  spi_tx(spi, &end_transmission, 1);

  prv_write_dummy(spi, SD_DUMMY_COUNT_CONST);

  // Catch the last busy response
  volatile uint8_t dataresponse;
  uint16_t timeout = 0xFFFF;
  while ((dataresponse = prv_read_byte(spi)) == 0x00 && timeout) {
    timeout--;
  }

  prv_pulse_idle(spi);
  return STATUS_CODE_OK;
}

StatusCode sd_is_initialized(SpiPort spi) {
  SdResponse res = prv_send_cmd(spi, SD_CMD_STATUS, 0, SD_DUMMY_BYTE, SD_RESPONSE_R2);
  prv_write_dummy(spi, 1);

  if (res.r1 == 0 && res.r2 == 0) {
    return STATUS_CODE_OK;
  }

  return status_code(STATUS_CODE_INTERNAL_ERROR);
}
