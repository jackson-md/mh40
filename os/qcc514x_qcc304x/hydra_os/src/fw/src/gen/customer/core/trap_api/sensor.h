#ifndef __SENSOR_H__
#define __SENSOR_H__
#include <app/sensor/sensor_if.h>
#include <app/bitserial/bitserial_if.h>

      
#if TRAPSET_SENSOR

/**
 *  \brief Creates Sensor Bus for I2C devices.
 *           This function is called to create an I2C bus for use with the sensor
 *  hub.
 *           The I2C clock speed is determined automatically from the sensors
 *  attached to the bus.
 *         
 *  \param hw_instance The BITSERIAL hardware instance to be used by this bus.
 *  \param scl_pio The PIO to be used for SCL signal.
 *  \param sda_pio The PIO to be used for SDA signal.
 *  \return A unique ID for this bus, 0 if Bus couldn't be created.
 * 
 * \ingroup trapset_sensor
 */
Bus BusI2c(uint8 hw_instance, uint8 scl_pio, uint8 sda_pio);

/**
 *  \brief Powers on Sensor Bus.
 *           This function initiates the power on sequence for the specified bus.
 *         
 *  \param bus_id The Bus ID.
 *  \return TRUE if power on sequence started, FALSE if power on wasn't started (normally
 *  due to incorrect bus ID, or bus is in the wrong state).
 * 
 * \ingroup trapset_sensor
 */
bool BusPowerOn(Bus bus_id);

/**
 *  \brief Powers off Sensor Bus.
 *           This function initiates the power off sequence for the specified bus.
 *         
 *  \param bus_id The Bus ID.
 *  \return TRUE if power off sequence started, FALSE if power off wasn't started (normally
 *  due to incorrect bus ID, or bus is in the wrong state).
 * 
 * \ingroup trapset_sensor
 */
bool BusPowerOff(Bus bus_id);

/**
 *  \brief Triggers a read of the specified sensor.
 *           This function can be used to initate a manual sensor read.  The
 *  result will be avaiable via the usual sensor output FIFO.
 *         
 *  \param sensor_id The Sensor ID.
 *  \param variant Variant of sensor read.
 * 
 * \ingroup trapset_sensor
 */
void SensorRead(Sensor sensor_id, uint16 variant);

/**
 *  \brief Triggers a read of the specified sensor.
 *           This function is used to perform an immediate sensor transfer. 
 *  Generally used by sensor drivers to configure a devices registers.
 *           This function will block until the results are available.
 *         
 *  \param sensor_id The Sensor ID.
 *  \param tx_data               Pointer to data to transmit, NULL if nothing to transmit.
 *             
 *  \param tx_size               Number of octets to transmit.
 *             
 *  \param rx_data               Pointer to buffer to receive data, NULL if nothing to receive.
 *             
 *  \param rx_size               Number of octets to receive.
 *             
 *  \return TRUE if transfer was successful, FALSE if transfer failed.
 * 
 * \ingroup trapset_sensor
 */
bool SensorTransfer(Sensor sensor_id, const void * tx_data, uint16 tx_size, void * rx_data, uint16 rx_size);

/**
 *  \brief Begin a sequence of asynchronous transfers
 *           This function is used to being a sequence of read and/or write
 *  transfers on the bus.  Reads and write are prepared by calling
 *           SensorAddWrite() and SensorAddRead().  Once the sequence has been
 *  prepared a call to SensorTransferEnd() passes the sequence to the
 *           framework to be actioned.
 *         
 *  \param sensor_id The Sensor ID.
 *  \return A handle that used in subsequent calls to
 *  SensorAddWrite()/SensorAddRead()/SensorTransferEnd()
 * 
 * \ingroup trapset_sensor
 */
BusTransferHandle SensorTransferBegin(Sensor sensor_id);

/**
 *  \brief Add write to sequence of transfers
 *  \param sensor_id The Sensor ID.
 *  \param h Transfer handle returned from SensorTransferBegin()
 *  \param data_ptr Pointer to data to transmit.  Note that since the transfer is asynchronous this
 *  pointer must remain valid until the transfer has completed.
 *  \param data_size Number of octets to transmit
 * 
 * \ingroup trapset_sensor
 */
void SensorAddWrite(Sensor sensor_id, BusTransferHandle h, const void * data_ptr, uint16 data_size);

/**
 *  \brief Add read to sequence of transfers
 *  \param sensor_id The Sensor ID.
 *  \param h Transfer handle returned from SensorTransferBegin()
 *  \param data_ptr Pointer to buffer to receive data.  Note that since the transfer is
 *  asynchronous this pointer must remain valid until the transfer has completed.
 *  \param data_size Number of octets to receive
 * 
 * \ingroup trapset_sensor
 */
void SensorAddRead(Sensor sensor_id, BusTransferHandle h, void * data_ptr, uint16 data_size);

/**
 *  \brief Finalise transfer sequence
 *  \param sensor_id The Sensor ID.
 *  \param h Transfer handle returned from SensorTransferBegin().
 * 
 * \ingroup trapset_sensor
 */
void SensorTransferEnd(Sensor sensor_id, BusTransferHandle h);

/**
 *  \brief Create a new instance of a sensor.
 *           This function create a new sensor instance.  The newly created sensor
 *  needs to be attached to bus by calling BusAttachSensor() before the sensor can
 *  be used.
 *         
 *  \param functions Pointer to table of functions.
 *  \param sub_class_size Size in bytes of data required for sensor instance specific data.
 *  \return Sensor ID for newly created sensor or 0 if creation failed.
 * 
 * \ingroup trapset_sensor
 */
Sensor SensorCreate(const sensor_functions_t * functions, uint16 sub_class_size);

/**
 *  \brief Return sensor instance data.
 *           This function returns a pointer to the sensor instance specific data
 *  that was allocated by SensorCreate().
 *         
 *  \param sensor_id The Sensor ID.
 *  \return Pointer to allocated data, or NULL if sensor ID is not valid.
 * 
 * \ingroup trapset_sensor
 */
void * SensorSubClassData(Sensor sensor_id);

/**
 *  \brief Read sensor configuration key.
 *  \param sensor_id The Sensor ID.
 *  \param key The configuration key to read.
 *  \param value Pointer to uint32 to store value read.
 *  \return TRUE if read was successful, FALSE otherwise.
 * 
 * \ingroup trapset_sensor
 */
bool SensorGetConfiguration(Sensor sensor_id, sensor_config_key_t key, uint32 * value);

/**
 *  \brief Write sensor configuration key.
 *  \param sensor_id The Sensor ID.
 *  \param key The configuration key to write.
 *  \param value New configuration key value.
 *  \return TRUE if write was successful, FALSE otherwise.
 * 
 * \ingroup trapset_sensor
 */
bool SensorConfigure(Sensor sensor_id, sensor_config_key_t key, uint32 value);

/**
 *  \brief Return pointer to next output FIFO item.
 *  \param sensor_id The Sensor ID.
 *  \return Pointer to next output FIFO item, NULL if sensor ID is invalid or FIFO is empty.
 * 
 * \ingroup trapset_sensor
 */
void * SensorDataMap(Sensor sensor_id);

/**
 *  \brief Return number of items in output FIFO.
 *  \param sensor_id The Sensor ID.
 *  \return Number of items in output FIFO, 0 if sensor ID is invalid.
 * 
 * \ingroup trapset_sensor
 */
uint16 SensorDataSize(Sensor sensor_id);

/**
 *  \brief Remove item at head of output FIFO.
 *  \param sensor_id The Sensor ID.
 * 
 * \ingroup trapset_sensor
 */
void SensorDataFlush(Sensor sensor_id);

/**
 *  \brief Register interrupt handler on the specified PIO.
 *  \param sensor_id The Sensor ID.
 *  \param pio PIO that interrupt is connected to.
 *  \param triggers Trigger type, low/high/rising/falling.
 *  \param handler Pointer to function that is called when interrupt occurs.
 * 
 * \ingroup trapset_sensor
 */
void SensorRegisterInterrupt(Sensor sensor_id, uint8 pio, uint8 triggers, sensor_interrupt_handler_t handler);

/**
 *  \brief Unegister interrupt handler on the specified PIO.
 *  \param sensor_id The Sensor ID.
 *  \param pio PIO that interrupt was connected to.
 *  \param handler Pointer to function that was called when interrupt occurs.
 * 
 * \ingroup trapset_sensor
 */
void SensorUnregisterInterrupt(Sensor sensor_id, uint8 pio, sensor_interrupt_handler_t handler);

/**
 *  \brief Get current interrupt level
 *  \param pio PIO that interrupt was connected to.
 *  \return 1 if interrupt is high, 0 if interrrupt is low.
 * 
 * \ingroup trapset_sensor
 */
uint8 SensorGetInterruptLevel(uint8 pio);

/**
 *  \brief Inform sensor hub that sensor is now powered off.
 *           During bus power off, each sensor driver must indicate when the
 *  sensor has powered off, it does this using this function.
 *         
 *  \param sensor_id The Sensor ID.
 * 
 * \ingroup trapset_sensor
 */
void SensorOffIndication(Sensor sensor_id);

/**
 *  \brief Attach sensor to bus.
 *           Before a sensor can be used it must be attached to a bus, this
 *  function is used to do this.
 *         
 *  \param bus_id The Bus ID.
 *  \param sensor_id The Sensor ID.
 *  \return TRUE if sensor was attached, FALSE if not.
 * 
 * \ingroup trapset_sensor
 */
bool BusAttachSensor(Bus bus_id, Sensor sensor_id);

/**
 *  \brief Enable timestamping on a specific PIO.
 *  \param pio The PIO to enable timestamping on.
 *  \param triggers Trigger type, rising/falling.  See SENSOR_INTERRUPT_XYZ macros.
 *  \return TRUE if timestamping was enabled, FALSE otherwise.  Failure will occur if all
 *  PIO timestamping HW resource are already in use.
 * 
 * \ingroup trapset_sensor
 */
bool SensorEnablePioTimestamp(uint8 pio, uint8 triggers);

/**
 *  \brief Disable timestamping on a specific PIO.
 *  \param pio The PIO to disable timestamping on.
 *  \return TRUE if timestamping was disabled, FALSE is timestamping wasn't previously
 *  enabled.
 * 
 * \ingroup trapset_sensor
 */
bool SensorDisablePioTimestamp(uint8 pio);

/**
 *  \brief Get timestamp of last PIO edge transition.
 *  \param pio The PIO to read timestamp for.
 *  \return The last timestamp, 0 if PIO didn't have timestampint enabled.
 * 
 * \ingroup trapset_sensor
 */
uint32 SensorGetPioTimestamp(uint8 pio);

/**
 *  \brief Get timestamp of last sensor read.
 *           A sensor has a timestamp associated with it, this timestamp is either
 *  the time the last sensor read was scheduled or when SensorSetTimestamp() was
 *  called to manually update the timestamp.
 *         
 *  \param sensor_id The Sensor ID.
 *  \return Timestamp of last sensor read.
 * 
 * \ingroup trapset_sensor
 */
uint32 SensorGetTimestamp(Sensor sensor_id);

/**
 *  \brief Manually set timestamp of last sensor read.
 *           This function is generally used by an interrupt handler to update the
 *  timestamp of when the interrupt occurred.
 *         
 *  \param sensor_id The Sensor ID.
 *  \param timestamp Timestamp in microseconds.
 * 
 * \ingroup trapset_sensor
 */
void SensorSetTimestamp(Sensor sensor_id, uint32 timestamp);

/**
 *  \brief Check if sensor is on.
 *  \param sensor_id The Sensor ID.
 *  \return TRUE if sensor is on, FALSE if it is off.
 * 
 * \ingroup trapset_sensor
 */
bool SensorIsOn(Sensor sensor_id);
#endif /* TRAPSET_SENSOR */
#endif
