/* Copyright (c) 2021 Qualcomm Technologies International, Ltd. */
/*   %%version */

/**
 * \file
 * Sensor Hub for P1
*/
#ifndef __SENSOR_HUB_H__
#define __SENSOR_HUB_H__

#include "hydra/hydra_types.h"
#include "bitserial/bitserial_if.h"
#include "pl_timers/pl_timers.h"

#include <message_.h>
#include <app/sensor/sensor_if.h>

typedef uint8 bus;
typedef uint16 sensor;
typedef uint8 bus_transfer_handle;

extern void sensor_hub_init(void);

extern bus sensor_hub_bus_i2c(uint8 hw_instance, uint8 pio_scl, uint8 pio_sda);
extern bool sensor_hub_bus_power_on(bus bus_id);
extern bool sensor_hub_bus_power_off(bus bus_id);
extern void sensor_hub_bus_sensor_off_indication(sensor sensor_id);
extern bool sensor_hub_bus_attach_sensor(bus bus_id, sensor sensor_id);

extern void sensor_hub_sensor_read(sensor sensor_id, uint16 variant);
extern bool sensor_hub_sensor_transfer(sensor sensor_id, const uint8 *tx_data, uint16 tx_size, uint8 *rx_data, uint16 rx_size);
extern bus_transfer_handle sensor_hub_sensor_transfer_begin(sensor sensor_id);
extern void sensor_hub_sensor_add_write(sensor sensor_id, bus_transfer_handle h, const void *data_ptr, uint16 data_size);
extern void sensor_hub_sensor_add_read(sensor sensor_id, bus_transfer_handle h, void *data_ptr, uint16 data_size);
extern void sensor_hub_sensor_transfer_end(sensor sensor_id, bus_transfer_handle h);
extern sensor sensor_hub_sensor_create(const sensor_functions_t *functions, uint16 sub_class_size);
extern void *sensor_hub_sensor_subclass_data(sensor sensor_id);
extern bool sensor_hub_sensor_get_configuration(sensor sensor_id, sensor_config_key_t key, uint32 *value);
extern bool sensor_hub_sensor_configure(sensor sensor_id, sensor_config_key_t key, uint32 value);

extern void *sensor_hub_sensor_data_map(sensor sensor_id);
extern uint16 sensor_hub_sensor_data_size(sensor sensor_id);
extern void sensor_hub_sensor_data_flush(sensor sensor_id);

extern Task sensor_hub_sensor_set_task(sensor sensor_id, Task task);
extern void sensor_hub_sensor_set_timestamp(sensor sensor_id, uint32 timestamp);
extern uint32 sensor_hub_sensor_get_timestamp(sensor sensor_id);
extern bool sensor_hub_sensor_is_on(sensor sensor_id);

#endif /* __SENSOR_HUB_H__ */
