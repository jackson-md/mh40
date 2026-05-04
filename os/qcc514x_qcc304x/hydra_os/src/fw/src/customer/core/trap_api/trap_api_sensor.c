/* Copyright (c) 2020 - 2021 Qualcomm Technologies International, Ltd. */
/*   %%version */
/**
 * \file
 */

#include "hydra/hydra_types.h"
#include "sensor_hub/sensor_hub.h"
#include "pioint/pioint.h"

#include "trap_api/sensor.h"
#include "trap_api/trap_api_private.h"

#if TRAPSET_SENSOR

Bus BusI2c(uint8 hw_instance, uint8 scl_pio, uint8 sda_pio)
{
    return sensor_hub_bus_i2c(hw_instance, scl_pio, sda_pio);
}

bool BusPowerOn(Bus bus_id)
{
    return sensor_hub_bus_power_on(bus_id);
}

bool BusPowerOff(Bus bus_id)
{
    return sensor_hub_bus_power_off(bus_id);
}

bool BusAttachSensor(Bus bus_id, Sensor sensor_id)
{
    return sensor_hub_bus_attach_sensor(bus_id, sensor_id);
}

Task MessageSensorTask(Sensor sensor_id, Task task)
{
    return sensor_hub_sensor_set_task(sensor_id, task);
}

void SensorRead(Sensor sensor_id, uint16 variant)
{
    sensor_hub_sensor_read(sensor_id, variant);
}

bool SensorTransfer(Sensor sensor_id, const void *tx_data, uint16 tx_size, void *rx_data, uint16 rx_size)
{
    return sensor_hub_sensor_transfer(sensor_id, tx_data, tx_size, rx_data, rx_size);
}

BusTransferHandle SensorTransferBegin(Sensor sensor_id)
{
    return sensor_hub_sensor_transfer_begin(sensor_id);
}

void SensorAddWrite(Sensor sensor_id, BusTransferHandle h, const void *data_ptr, uint16 data_size)
{
    sensor_hub_sensor_add_write(sensor_id,h, data_ptr, data_size);
}

void SensorAddRead(Sensor sensor_id, bus_transfer_handle h, void *data_ptr, uint16 data_size)
{
    sensor_hub_sensor_add_read(sensor_id,h, data_ptr, data_size);
}

void SensorTransferEnd(Sensor sensor_id, BusTransferHandle h)
{
    sensor_hub_sensor_transfer_end(sensor_id, h);
}

Sensor SensorCreate(const sensor_functions_t *functions, uint16 sub_class_size)
{
    return sensor_hub_sensor_create(functions, sub_class_size);
}

void *SensorSubClassData(Sensor sensor_id)
{
    return sensor_hub_sensor_subclass_data(sensor_id);
}

bool SensorGetConfiguration(Sensor sensor_id, sensor_config_key_t key, uint32 *value)
{
    return sensor_hub_sensor_get_configuration(sensor_id, key, value);
}

bool SensorConfigure(sensor sensor_id, sensor_config_key_t key, uint32 value)
{
    return sensor_hub_sensor_configure(sensor_id, key, value);
}

void *SensorDataMap(Sensor sensor_id)
{
    return sensor_hub_sensor_data_map(sensor_id);
}

uint16 SensorDataSize(Sensor sensor_id)
{
    return sensor_hub_sensor_data_size(sensor_id);
}

void SensorDataFlush(Sensor sensor_id)
{
    sensor_hub_sensor_data_flush(sensor_id);
}

void SensorRegisterInterrupt(Sensor sensor_id, uint8 pio, uint8 triggers, pioint_handler_with_context_t handler)
{
    pioint_configure_with_context(PBANK(pio), POFFM(pio), handler, sensor_id, triggers);
}

void SensorUnregisterInterrupt(Sensor sensor_id, uint8 pio, pioint_handler_with_context_t handler)
{
    pioint_configure_with_context(PBANK(pio), 0, handler, sensor_id, PIOINT_RISING | PIOINT_FALLING);
}

bool SensorEnablePioTimestamp(uint8 pio, uint8 triggers)
{
    return pioint_enable_strobe_timestamp(pio, triggers);
}

bool SensorDisablePioTimestamp(uint8 pio)
{
    return pioint_disable_strobe_timestamp(pio);
}

uint32 SensorGetPioTimestamp(uint8 pio)
{
    return pioint_get_strobe_timestamp(pio);
}

uint8 SensorGetInterruptLevel(uint8 pio)
{
    return pio_get_level(pio);
}

void SensorSetTimestamp(Sensor sensor_id, uint32 timestamp)
{
    sensor_hub_sensor_set_timestamp(sensor_id, timestamp);
}

uint32 SensorGetTimestamp(Sensor sensor_id)
{
    return sensor_hub_sensor_get_timestamp(sensor_id);
}

void SensorOffIndication(Sensor sensor_id)
{
    sensor_hub_bus_sensor_off_indication(sensor_id);
}

bool SensorIsOn(Sensor sensor_id)
{
    return sensor_hub_sensor_is_on(sensor_id);
}

#endif /* TRAPSET_SENSOR */
