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

#include "dht.h"

using namespace dht;

// all timings used for data transmission
const uint DHT_START_SIGNAL_LOW_TIME_US = 20000;
const uint DHT_START_SIGNAL_HIGH_TIME_US = 40;
const uint DHT_RESPONSE_TIMEOUT_US = 80;
const uint DHT_GETREADY_TIMEOUT_US = 80;
const uint DHT_START_READ_BIT_TIMEOUT_US = 50;
const uint DHT_READ_BIT_TIMEOUT_US = 70;
const uint DHT_BIT_HIGH_THRESHOLD_US = 30;

Sensor::Sensor(gpio_num_t pin): m_pin{pin} {};

// wait on pin until state change or timeout reached
// return the time waited in microsecond if parameter duration is not nullptr
bool Sensor::waitOnPin(bool state, uint timeout, uint* duration) {
    uint timer = 0;
    while (gpio_get_level(m_pin) == state) {
        if (timer > timeout) {
            return false;
        }
        timer += 1;
        esp_rom_delay_us(1);
    }
    if (duration) {
        *duration = timer;
    }
    return true;
}

Measure Sensor::collect() {
    Measure m;
    uint timer[40];

    // start signal
    gpio_set_direction(m_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(m_pin, 0);
    esp_rom_delay_us(DHT_START_SIGNAL_LOW_TIME_US);
    gpio_set_level(m_pin, 1);
    esp_rom_delay_us(DHT_START_SIGNAL_HIGH_TIME_US);

    // switch direction to reception mode
    gpio_set_direction(m_pin, GPIO_MODE_INPUT);

    // wait for response message for ~80ms
    if (!waitOnPin(0, DHT_RESPONSE_TIMEOUT_US)) {
        m.error = Error::ResponseTimeout;
        return m;
    }

    // wait for get ready message for ~80ms
    if (!waitOnPin(1, DHT_GETREADY_TIMEOUT_US)) {
        m.error = Error::GetReadyTimeout;
        return m;
    }

    // Now start data transmission, a complete data transmission is 40bit.
        for (int i=0; i<40; i++) {
        // When DHT is sending data, every bit of data begins with the 50us low-voltage-level
        if (!waitOnPin(0, DHT_START_READ_BIT_TIMEOUT_US)) {
            m.error = Error::StartReadBitTimeout;
            return m;
        }
        // measure the length of the following high-voltage-level signal
        if (!waitOnPin(1, DHT_READ_BIT_TIMEOUT_US, &timer[i])) {
            m.error = Error::ReadBitTimeout;
            return m;
        }
    }

    // Transform timers to 0 or 1
    unsigned int data[5] = {0, 0, 0, 0, 0};
    uint8_t byte_index = 0; // used to index byte in data[]
    uint8_t bit_index = 7;  // used to set/unset bit in byte, starts from higher bit
    for (int i=0; i<40; i++) {
        // have we read a 1 ?
        if (timer[i] > DHT_BIT_HIGH_THRESHOLD_US) {
            data[byte_index] |= (1 << bit_index);
        }
        if (bit_index == 0) {  // next byte
            byte_index += 1;
            bit_index = 7;
        } else { // lower bit in same byte
            bit_index -= 1;
        }
    }

    // Update Measure and return
    m.humidity = data[0] + 0.1 * data[1];
    m.temperature = data[2] + 0.1 * data[3];
    uint crc = data[0] + data[1] + data[2] + data[3];
    if (data[4] != crc) {
        m.error = Error::Checksum;
    } 
    return m;
}
