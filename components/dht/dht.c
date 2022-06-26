/******************************************************************************
  DHT Temperature & Humidity Sensor

  Tested with DHT11

  Communication process -  Serial Interface (Single-Wire Two-Way)
    1. Sends start signal to DHT
    2. DHT sends out response signal
    3. DHT sends out get ready message
    3. DHT starts data transmission

  Start signal
    When the communication between MCU and DHT11 begins, the programme of MCU will
    set Data Single-bus voltage level from high to low and this process must take at
    least 18ms to ensure DHT’s detection of MCU's signal, then MCU will pull up voltage
    and wait 20-40us for DHT’s response.

  DHT response message
    Once DHT detects the start signal, it will send out a low-voltage-level response signal, which lasts 80us.

  DHT get ready message
    DHT sets voltage level from low to high and keeps it for 80us.

  Data transmission
    - A complete data transmission is 40bit, and the sensor sends higher data bit first
    - Data format: 8bit integral RH data + 8bit decimal RH data + 8bit integral T data + 8bit decimal T data + 8bit check sum
    - Every bit of data begins with the 50us low-voltage-level
      - The length of the following high-voltage-level signal determines whether data bit is "0" or "1"
      - A 0 bit max 30 usecs, a 1 bit at least 68 usecs.

  If the data transmission is right, the check-sum should be the last 8bit of 
   "8bit integral RH data + 8bit decimal RH data + 8bit integral T data + 8bit decimal T data
******************************************************************************/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "dht.h"

// all timings used for data transmission
#define DHT_START_SIGNAL_LOW_TIME_US    20000
#define DHT_START_SIGNAL_HIGH_TIME_US   40
#define DHT_RESPONSE_TIMEOUT_US         80
#define DHT_GETREADY_TIMEOUT_US         80
#define DHT_START_READ_BIT_TIMEOUT_US   50
#define DHT_READ_BIT_TIMEOUT_US         70
#define DHT_BIT_HIGH_THRESHOLD_US       30

// wait on pin until state change or timeout reached
// return the time waited in microsecond if parameter duration is not NULL
static inline dht_err_t wait_on_pin(int pin, int state, int timeout, int* duration)
{
    int timer = 0;
    while (gpio_get_level(pin) == state) {
        if (timer > timeout) {
            return DHT_ERR_TIMEOUT;
        }
        timer += 1;
        esp_rom_delay_us(1);
    }
    if (duration) {
        *duration = timer;
    }
    return DHT_OK;
}

dht_err_t dht_read(int pin, dht_measure_t* measure) 
{
    // prepare to send start signal to DHT
    if (gpio_set_direction(pin, GPIO_MODE_OUTPUT) != ESP_OK) {
        return DHT_ERR_INVALID_PIN;
    }

    /** 
     * START of the CRITICAL SECTION
     *
     * as communication with DHT is based on timings, we need to disable the task switching
     * 
     * WARNING: don't forget to reenable it before return in case of errror
     */
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&mux);

    // start signal
    gpio_set_level(pin, 0);
    esp_rom_delay_us(DHT_START_SIGNAL_LOW_TIME_US);
    gpio_set_level(pin, 1);
    esp_rom_delay_us(DHT_START_SIGNAL_HIGH_TIME_US);

    // switch direction to reception mode
    gpio_set_direction(pin, GPIO_MODE_INPUT);

    // wait for response message
    if (wait_on_pin(pin, 0, DHT_RESPONSE_TIMEOUT_US, NULL) != DHT_OK) {
        portEXIT_CRITICAL(&mux);
        return DHT_ERR_RESPONSE_TIMEOUT;
    }

    // wait for get ready message
    if (wait_on_pin(pin, 1, DHT_GETREADY_TIMEOUT_US, NULL) != DHT_OK) {
        portEXIT_CRITICAL(&mux);
        return DHT_ERR_GETREADY_TIMEOUT;
    }  

    // Now start data transmission, a complete data transmission is 40bit.
    uint8_t data[5] = {0, 0, 0, 0, 0};
    uint8_t byte_index = 0; // used to index byte in data[]
    uint8_t bit_index = 7;  // used to set/unset bit in byte, starts from higher bit

    for (int i=0; i<40; i++) {
        // When DHT is sending data, every bit of data begins with the 50us low-voltage-level
        if (wait_on_pin(pin, 0, DHT_START_READ_BIT_TIMEOUT_US, NULL) != DHT_OK) {
            portEXIT_CRITICAL(&mux);
            return DHT_ERR_START_READ_BIT_TIMEOUT;
        }  

        // measure the length of the following high-voltage-level signal
        int timer = 0;
        if (wait_on_pin(pin, 1, DHT_READ_BIT_TIMEOUT_US, &timer) != DHT_OK) {
            portEXIT_CRITICAL(&mux);
            return DHT_ERR_READ_BIT_TIMEOUT;
        }  

        // have we read a 1 ?
        if (timer > DHT_BIT_HIGH_THRESHOLD_US) {
            data[byte_index] |= (1 << bit_index);
        }

        if (bit_index == 0) { 
            // next byte
            byte_index += 1;
            bit_index = 7;
        } else {
            // lower bit in same byte
            bit_index -= 1;
        }
    }

    portEXIT_CRITICAL(&mux);
    /*
     * END of the CRITICAL SECTION
     */

    uint8_t cksum = data[0] + data[1] + data[2] + data[3];
    if (data[4] != cksum) {
        return DHT_ERR_CHECKSUM;
    }

    measure->humidity = data[0] + 0.1 * data[1];
    measure->temperature = data[2] + 0.1 * data[3];
    measure->crc = data[4];

    return DHT_OK;
}
