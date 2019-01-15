#ifndef __MY_TOOLBOX_SENSOR_DHT_H__
#define __MY_TOOLBOX_SENSOR_DHT_H__

#ifdef __cplusplus
extern "C" {
#endif

/* error type */
typedef enum 
{
    DHT_OK = 0,
    DHT_ERR_INVALID_PIN,
    DHT_ERR_TIMEOUT,
    DHT_ERR_RESPONSE_TIMEOUT,
    DHT_ERR_GETREADY_TIMEOUT,
    DHT_ERR_START_READ_BIT_TIMEOUT,
    DHT_ERR_READ_BIT_TIMEOUT,
    DHT_ERR_CHECKSUM
} dht_err_t;

/* measure */
typedef struct 
{
    float   humidity;
    float   temperature;
    uint8_t crc;
} dht_measure_t;

dht_err_t dht_read(int pin, dht_measure_t* measure);

#ifdef __cplusplus
}
#endif

#endif