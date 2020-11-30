#include "i2c_write.h"

#include <math.h>
#include <stdlib.h>

#include "dispatcher.h"
#include "gpio.h"
#include "i2c.h"
#include "log.h"
#include "wait.h"

#define I2C1_SDA \
    {.port = GPIO_PORT_B, .pin = 9}
#define I2C1_SCL \
    {.port = GPIO_PORT_B, .pin = 8}
#define I2C2_SDA \
    {.port = GPIO_PORT_B, .pin = 11}
#define I2C2_SCL \
    {.port = GPIO_PORT_B, .pin = 10}

static uint8_t message_count = 0;
static uint8_t i2c_command[8];
static uint8_t *i2c_metadata;

static StatusCode prv_i2c_write_data_callback(uint8_t data[8], void *context, bool *tx_result);

static StatusCode prv_i2c_write_data_callback(uint8_t data[8], void *context, bool *tx_result) {
    LOG_DEBUG("REAL CALLBACK!\n");
    if(message_count == 0) {
        if(data[0] != BABYDRIVER_MESSAGE_I2C_WRITE_COMMAND || (data[1] != 0 && data[1] != 1)) {
            return STATUS_CODE_INVALID_ARGS;
        }

        i2c_metadata = malloc(ceil(data[3]/7)*7*sizeof(uint8_t));
        LOG_DEBUG("FIRST MESSAGE\n");
        *tx_result = false;
        message_count++;
        for(uint8_t i = 0; i < 8; i++) {
            i2c_command[i] = data[i];
        }

        return STATUS_CODE_OK;
    } else if(message_count == ceil(i2c_command[3]/7)) {
        if(data[0] != BABYDRIVER_MESSAGE_I2C_WRITE_DATA) {
            return STATUS_CODE_INVALID_ARGS;
        }

        LOG_DEBUG("FINAL CALLBACK!\n");
        *tx_result = true;

        uint8_t metadata_index = (message_count - 1)*7;
        for(uint8_t i = 0; i < 7; ++i) {
            i2c_metadata[metadata_index + i] = data[i + 1];
        }

        message_count = 0;
        I2CPort port = i2c_command[1];
        uint8_t addr = i2c_command[2];
        size_t tx_len = i2c_command[3];
        uint8_t is_reg = i2c_command[4];
        uint8_t reg = i2c_command[5];

        if(port == I2C_PORT_1) {
            I2CSettings i2c1_settings = {
                .speed = I2C_SPEED_FAST,
                .sda = I2C1_SDA,
                .scl = I2C1_SCL,
            };
            i2c_init(I2C_PORT_1, &i2c1_settings);
        } else {
            I2CSettings i2c2_settings = {
                .speed = I2C_SPEED_FAST,
                .sda = I2C2_SDA,
                .scl = I2C2_SCL,
            };
            i2c_init(I2C_PORT_2, &i2c2_settings);
        }

        if(is_reg) {
            status_ok_or_return(i2c_write_reg(port, addr, reg, i2c_metadata, tx_len));
        } else {
            status_ok_or_return(i2c_write(port, addr, i2c_metadata, tx_len));
        }

        free(i2c_metadata);

        return STATUS_CODE_OK;
    } else {
        if(data[0] != BABYDRIVER_MESSAGE_I2C_WRITE_DATA) {
            return STATUS_CODE_INVALID_ARGS;
        }

        uint8_t metadata_index = (message_count - 1)*7;
        message_count++;

        for(uint8_t i = 0; i < 7; i++) {
            i2c_metadata[metadata_index + i] = data[i+1];
        }

        return STATUS_CODE_OK;
    }
}

StatusCode i2c_write_init(void) {
    LOG_DEBUG("Initialized!\n");
    status_ok_or_return(dispatcher_register_callback(BABYDRIVER_MESSAGE_I2C_WRITE_COMMAND, prv_i2c_write_data_callback, NULL));
    return dispatcher_register_callback(BABYDRIVER_MESSAGE_I2C_WRITE_DATA, prv_i2c_write_data_callback, NULL);
}
