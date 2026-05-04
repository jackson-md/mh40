#ifndef __APP_SENSOR_IF_H__
#define __APP_SENSOR_IF_H__

/****************************************************************************

Copyright (c) 2021 Qualcomm Technologies International, Ltd.
  %%version

FILE
        sensor_if.h  -  Sensor Hub Interface

CONTAINS
        Interface elements between the Sensor Hub firmware and VM Applications,
        that are not in sys.[ch]

DESCRIPTION
        This file is seen by the stack, and VM applications.
*/
#include <stdint.h>

/*! Public type for sensor */
typedef uint16 Sensor;

/*! Public type for bus */
typedef uint8 Bus;

/*! Public type for bus transfer handle */
typedef uint8 BusTransferHandle;

/*! Public type for sensor times */
typedef uint32 SensorTime;

/*! Public type for sensor interrupt handler */
typedef void (*sensor_interrupt_handler_t)(uintptr_t context, uint32 bits);

/*! Enum of sensor message settings */
typedef enum
{
    SENSOR_MESSAGES_ALL,                /*!< Send all messages to the registered task */
    SENSOR_MESSAGES_SOME,               /*!< Send message when FIFO goes from empry to not empty */
    SENSOR_MESSAGES_NONE,               /*!< Send no messages to the registered task */
    SENSOR_MESSAGES_NEXT                /*!< Send message when next data is added to FIFO, reverts to SENSOR_MESSAGES_SOME once message sent */
} sensor_messages_settings_t;

/*! Enum of sensor configuration keys */
typedef enum
{
    SENSOR_CONFIG_I2C_ADDR,             /*!< I2C address - read only */
    SENSOR_CONFIG_I2C_RATE_KHZ,         /*!< I2C clock rate in KHz - read only */

    SENSOR_CONFIG_FIFO_ELEMENT_TYPE,    /*!< Type of each element in FIFO - read only */
    SENSOR_CONFIG_FIFO_MAX_ELEMENTS,    /*!< Maximum number of elements in FIFO - read only */
    SENSOR_CONFIG_RAW_BUFFER_SIZE,      /*!< Size in bytes of buffer to store raw data read from sensor - read only */

    SENSOR_CONFIG_PERIOD_US,            /*!< Enable periodic reads with specified period in microseconds, 0 disables periodic reads */

    SENSOR_CONFIG_ANCHOR_SENSOR,        /*!< Specify sensor to derive timing from */
    SENSOR_CONFIG_RELATIVE_US,          /*!< Enable reading relative to another specified sensor */

    SENSOR_CONFIG_MESSAGES,

    /*! Start of common driver configuration keys */
    SENSOR_CONFIG_DATA_RATE = 0x0800,   /*!< Configure data rate of sensor */
    SENSOR_CONFIG_INTERRUPT_PIO,        /*!< Enable interrupt driven sensor reading on specified PIO.  This will disable periodic reading */

    SENSOR_CONFIG_DRIVER = 0x1000       /*!< Start of driver specific configuration keys */
} sensor_config_key_t;

/*! Enum of bus configuration keys */
typedef enum
{
    BUS_CONFIG_I2C_SDA_OFF_STATE,       /*!< NOT IMPLEMENTED - Default state of I2C SDA pin when bus is off (0 - low, 1 - high) */
    BUS_CONFIG_I2C_SCK_OFF_STATE,       /*!< NOT IMPLEMENTED - Default state of I2C SCK pin when bus is off (0 - low, 1 - high) */
    BUS_CONFIG_POWER_PIO,               /*!< NOT IMPLEMENTED - PIO for bus power control */
    BUS_CONFIG_POWER_PIO_OFF_STATE      /*!< NOT IMPLEMENTED - Default state of power control pin when bus is off (0 - low, 1 - high) */
} bus_config_key_t;

/*! Union of all FIFO element types */
typedef union
{
    struct { int16 x,y,z; } xyz_int16;                              /*!< X,Y,Z */
    struct { int16 x[2],y[2],z[2]; } xyz2_int16;                    /*!< X1,Y1,Z1, X2,Y2,Z2 */
    struct { int16 x[2],y[2],z[2]; int16 temp; } xyz2_int16_temp;   /*!< X1,Y1,Z1, X2,Y2,Z2, temperature */
    struct { SensorTime ts; int16 x,y,z; } ts_xyz_int16;                            /*!< timestamp, X,Y,Z */
    struct { SensorTime ts; int16 x[2],y[2],z[2]; } ts_xyz2_int16;                   /*!< timestamp, X1,Y1,Z1, X2,Y2,Z2 */
    struct { SensorTime ts; int16 x[2],y[2],z[2]; int16 temp; } ts_xyz2_int16_temp; /*!< timestamp, X1,Y1,Z1, X2,Y2,Z2, temperature */
    uint8 custom[1];    /*!< Custom type */
} fifo_item_t;

#define SENSOR_FIFO_TYPE_CUSTOM(size)           (0 | ((size) << 8))

/*! Generate type constant for specified FIFO type and ID (type size is encoded as part of the constant) */
#define SENSOR_FIFO_TYPE(id, type)              ((id) | (sizeof(((fifo_item_t *)0)->type) << 8))

#define SENSOR_FIFO_TYPE_XYZ_INT16              SENSOR_FIFO_TYPE(0x01, xyz_int16)
#define SENSOR_FIFO_TYPE_XYZx2_INT16            SENSOR_FIFO_TYPE(0x02, xyz2_int16)
#define SENSOR_FIFO_TYPE_XYZx2_INT16_TEMP       SENSOR_FIFO_TYPE(0x03, xyz2_int16_temp)
#define SENSOR_FIFO_TYPE_TS_XYZ_INT16           SENSOR_FIFO_TYPE(0x81, ts_xyz_int16)
#define SENSOR_FIFO_TYPE_TS_XYZx2_INT16         SENSOR_FIFO_TYPE(0x82, ts_xyz2_int16)
#define SENSOR_FIFO_TYPE_TS_XYZx2_INT16_TEMP    SENSOR_FIFO_TYPE(0x83, ts_xyz2_int16_temp)

/*! Extract type size from type constant */
#define SENSOR_TYPE_SIZEOF(type) ((type) >> 8)

/*! Sensor driver callback functions */
typedef struct
{
    void (*pre_power_on)(Bus bus_id, Sensor sensor_id);     /*!< Called before bus is powered on (no transfers allowed, used for power supply control) */
    void (*post_power_on)(Bus bus_id, Sensor sensor_id);    /*!< Called after bus is powered on (transfer can happen now to configure sensor) */
    void (*pre_power_off)(Bus bus_id, Sensor sensor_id);    /*!< Called before bus is power off (transfers alloweed to shutdown sensor) */
    void (*post_power_off)(Bus bus_id, Sensor sensor_id);   /*!< Called after bus is powered off (no transfers allowed, used for power supply control) */
    bool (*get_config)(Sensor sensor_id, sensor_config_key_t key, uint32 *value);   /*!< Called to read configuration key */
    bool (*set_config)(Sensor sensor_id, sensor_config_key_t key, uint32 value);    /*!< Called to set configuration key */
    bool (*get_read_time)(Sensor sensor_id, SensorTime now, SensorTime *next_read_time);    /*!< Called to get next read time, used for custom timing.  Set to NULL for standard timing options */
    BusTransferHandle (*read_setup)(Sensor sensor_id, uint8 *raw_data_ptr, uint16 variant); /*!< Called to setup sensor read for specified variant */
    bool (*read_complete)(Sensor sensor_id, const uint8 *raw_data_ptr, fifo_item_t *fi_ptr); /*!< Called to process raw data once sensor read has completed */
} sensor_functions_t;

#define SENSOR_INTERRUPT_RISING     (0x01)
#define SENSOR_INTERRUPT_FALLING    (0x02)
#define SENSOR_INTERRUPT_HIGH       (0x01)
#define SENSOR_INTERRUPT_LOW        (0x02)

#endif  /* __APP_SENSOR_IF_H__ */
