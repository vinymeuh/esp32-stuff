#ifndef __MY_TOOLBOX_SENSOR_DHT_H__
#define __MY_TOOLBOX_SENSOR_DHT_H__

#include "driver/gpio.h"

namespace dht {

enum class Error {
    None = 0,
    Checksum,
    ResponseTimeout,
    GetReadyTimeout,
    StartReadBitTimeout,
    ReadBitTimeout
};

struct Measure {
    Measure() : humidity{0.0}, temperature{0.0}, error{Error::None}  {};

    float humidity;
    float temperature;
    Error error;
};

class Sensor {
public:
    Sensor(gpio_num_t pin);

    dht::Measure collect();
private:
    gpio_num_t m_pin;

    bool waitOnPin(bool state, uint timeout, uint* duration);
    bool waitOnPin(bool state, uint timeout) {
        return waitOnPin(state, timeout, nullptr);
    };
};

} // namespace dht
#endif
