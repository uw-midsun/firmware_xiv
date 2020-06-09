#include "ads1259_adc.h"
#include "ads1259_adc_defs.h"

#include "delay.h"
#include "soft_timer.h"



#define ADS_RX_BYTES 3

static SoftTimerId s_timer_id;


static const uint8_t s_register_lookup[NUM_REGISTERS] = {
    [ADS1259_ADDRESS_CONFIG0] = ADS1259_CONFIG0_SETTINGS,
    [ADS1259_ADDRESS_CONFIG1] = ADS1259_CONFIG1_SETTINGS,
    [ADS1259_ADDRESS_CONFIG2] = ADS1259_CONFIG2_SETTINGS,
    [ADS1259_ADDRESS_OFC0] = ADS1259_OFC0_RESET,
    [ADS1259_ADDRESS_OFC1] = ADS1259_OFC1_RESET,
    [ADS1259_ADDRESS_OFC2] = ADS1259_OFC2_RESET,
    [ADS1259_ADDRESS_FSC0] = ADS1259_FSC0_RESET,
    [ADS1259_ADDRESS_FSC1] = ADS1259_FSC1_RESET,
    [ADS1259_ADDRESS_FSC2] = ADS1259_FSC2_RESET,
};

static const uint8_t s_conversion_time_ms_lookup[NUM_ADS1259_DATA_RATE] {
    [ADS1259_DATA_RATE_10] = 100,
    [ADS1259_DATA_RATE_17] = 61,
    [ADS1259_DATA_RATE_50] = 21,
    [ADS1259_DATA_RATE_60] = 17,
    [ADS1259_DATA_RATE_400] = 3,
    [ADS1259_DATA_RATE_1200] = 2,
    [ADS1259_DATA_RATE_3600] = 1,
    [ADS1259_DATA_RATE_14400] = 1,
};


static void prv_stop_rdatac(Ads1259Storage* storage) {
    uint8_t payload[] = { ADS1259_STOP_READ_DATA_CONTINUOUS };
    delay_us(100); // (SOFT-173 TODO) Must wait at leas 2^16 clk cycles before sending first command - change?
    spi_tx(storage->spi_port, payload, sizeof(payload));
}

static StatusCode prv_configure_registers(Ads1259Storage* storage) {
    uint8_t payload[] =  { ADS1259_WRITE_REGISTER | ADS1259_ADDRESS_CONFIG0, 3, ADS1259_CONFIG0_SETTINGS, 
        ADS1259_CONFIG1_SETTINGS, ADS1259_CONFIG2_SETTINGS, ADS1259_OFC0_RESET, 
        ADS1259_OFC1_RESET, ADS1259_OFC2_RESET, ADS1259_FSC0_RESET, ADS1259_FSC1_RESET, ADS1259_FSC2_RESET };
    spi_tx(storage->spi_port, payload, sizeof(payload));
    for(int reg = 0x0; reg < NUM_REGISTERS; reg++) {
        status_ok_or_return(prv_check_register(storage, reg, s_register_lookup[reg]));
    }
    return STATUS_CODE_OK;
}

static StatusCode prv_check_register(Ads1259Storage* storage, uint8_t register_address, uint8_t register_val) {
    uint8_t payload[] =  { ADS1259_READ_REGISTER | register_address };
    spi_exchange(storage->spi_port, payload, sizeof(payload), storage->data,
                        sizeof(uint8_t));
    if(!(register_val == storage->data))
        return STATUS_CODE_UNINITIALIZED;
    return STATUS_CODE_OK;
}

static void prv_ads1259_send_command(Ads1259Storage* storage, uint8_t command) {
    uint8_t payload[] = { command };
    spi_tx(storage->spi_port, payload, size_of(payload));
}
static void prv_conversion_callback(SoftTimerId timer_id, void* context) {
    Ads1259Storage* storage = (Ads1259Storage*)context;
    spi_rx(storage->spi_port, storage->data,
            sizeof(uint8_t)*(ADS_RX_BYTES + ADS1259_CHECK_SUM_ENABLE)); //this can be improved
}

StatusCode ads1259_init(Ads1259Settings* settings, Ads1259Storage* storage) {
    storage->spi_port = settings->spi_port;

    const SpiSettings spi_settings = {
    .baudrate = settings->spi_baudrate,
    .mode = SPI_MODE_0,
    .mosi = settings->mosi,
    .miso = settings->miso,
    .sclk = settings->sclk,
    .cs = settings->cs,
    };

    status_ok_or_return(spi_init(settings->spi_port, &spi_settings));
    prv_stop_rdatac(storage);
    status_ok_or_return(prv_configure_registers(storage));
    //need to send start command for conversions to begin?
    //calibrate?
    
}

StatusCode ads1259_get_conversion_data(Ads1259Storage* storage) {
    void *context = (void *)storage;
    uint8_t payload[] = { ADS1259_READ_DATA_BY_OPCODE };

    status_ok_or_return(soft_timer_start_millis(s_conversion_time_ms_lookup[ADS1259_DATA_RATE_SPS], 
        prv_conversion_callback, context, s_timer_id)) ;
    
    if(ADS1259_CHECK_SUM_ENABLE) {
        uint8_t sum = (uint8_t)(storage->data[ADS_RX_LSB] + storage->data[ADS_RX_LSB] + 
        storage->data[ADS_RX_LSB] + ADS1259_CHECKSUM_OFFSET);
        if(ADS1259_OUT_OF_RANGE_FLAG_ENABLE) {
            if(!(storage->data[ADS_RX_CHK_SUM]>>7 & 1))
                return STATUS_CODE_OUT_OF_RANGE;
            if(sum<<1 != storage->data[ADS_RX_CHK_SUM]<<1) { // implement error handler
                return STATUS_CODE_INTERNAL_ERROR;
            }

        } else if(sum != storage->data[ADS_RX_CHK_SUM]) {
            return STATUS_CODE_INTERNAL_ERROR;
        }
    }
}