#ifndef __APP_VM_IF_H__
#define __APP_VM_IF_H__

/* Copyright (c) 2016 - 2019 Qualcomm Technologies International, Ltd. */
/*   %%version */
/****************************************************************************
FILE
        vm_if.h  -  VM Interface

CONTAINS
        Interface elements between the VM and VM Applications, that are
        not in sys.[ch]

DESCRIPTION
        This file is seen by the stack, and VM applications, and
        contains event numbers that are common between them.

        Events can be read (and cleared) using EventCounter. If they are
        enabled, using EventEnable, then that event can also cause VmWait
        to return before the specified timeout.
*/

 
#include "app/bluestack/bluetooth.h"
 
/*!
  @brief Values used with #VM_STREAM_UART_CONFIG key of StreamConfigure().
*/
typedef enum
{
    VM_STREAM_UART_THROUGHPUT,  /*!< UART throughput.*/
    VM_STREAM_UART_LATENCY      /*!< UART latency.*/
} vm_stream_uart_config;

/*!
  @brief Keys for StreamConfigure().
*/
typedef enum
{
    /*! UART configuration. Value is a member of #vm_stream_uart_config.
     *  The default configuration is #VM_STREAM_UART_THROUGHPUT. */
    VM_STREAM_UART_CONFIG,

    /*! SCO enabled. */
    VM_STREAM_SCO_ENABLED,

    /*! #MESSAGE_USB_ALT_INTERFACE enabled. */
    VM_STREAM_USB_ALT_IF_MSG_ENABLED,

    /*! #MESSAGE_USB_ATTACHED and #MESSAGE_USB_DETACHED enabled. */
    VM_STREAM_USB_ATTACH_MSG_ENABLED,

    /*! USB Enable/Disable packet counter byte. (Not supported on BC4-EXT.) */
    VM_STREAM_USB_PACKET_COUNTER_ENABLED,

    /*! Enable large size buffer for a L2CAP channel for a PSM. */
    VM_STREAM_L2CAP_ADD_LARGE_BUFFER_ON_PSM,

    /*! Remove large size buffer for a L2CAP channel for a PSM. */
    VM_STREAM_L2CAP_REMOVE_LARGE_BUFFER,

    /*! This key is not used. */
    VM_STREAM_UART_MAX_INTERBYTE_SPACING,

    /*! Control the time the system waits after the last UART RX activity before
        it considers going to sleep. The time is expressed in milliseconds. */
    VM_STREAM_UART_SLEEP_TIMEOUT
} vm_stream_config_key;

/*!
  @brief Values used with #VM_SOURCE_MESSAGES and #VM_SINK_MESSAGES for SourceConfigure() and SinkConfigure() respectively.

  For instance SourceConfigure(source, VM_SOURCE_MESSAGES,
  VM_MESSAGES_SOME) will arrange that the task associated has at most
  one #MESSAGE_MORE_DATA queued for delivery at any one time. The
  default is VM_MESSAGES_ALL.
*/

typedef enum
{
    VM_MESSAGES_ALL,                /*!< Send all messages to the registered task */
    VM_MESSAGES_SOME,               /*!< Send at most one message at a time to the registered task */
    VM_MESSAGES_NONE                /*!< Send no messages to the registered task */
} vm_messages_settings;

/*!
  @brief Keys for TransformConfigure().
  @see   hid_set_mode
  @see   hid_type 
*/
typedef enum
{
    VM_TRANSFORM_CHUNK_CHUNK_SIZE,              /*!< Chunk size.*/
    VM_TRANSFORM_SLICE_FROM_START,              /*!< How many bytes to discard from start of packet */
    VM_TRANSFORM_SLICE_FROM_END,                /*!< How many bytes to discard from end of packet */
    VM_TRANSFORM_RTP_SBC_ENCODE_PACKET_SIZE,    /*!< RTP SBC Encode packet size.*/
    VM_TRANSFORM_RTP_SBC_ENCODE_MANAGE_TIMING,  /*!< RTP SBC Encode Manage Timing.*/
    VM_TRANSFORM_RTP_SBC_ENCODE_STATS,          /*!< RTP SBC Encode Statistics.*/
    VM_TRANSFORM_RTP_SCMS_ENABLE,               /*!< Enable SCMS content protection on RTP stream.*/
    VM_TRANSFORM_RTP_SCMS_SET_BITS,             /*!< Set SCMS bits (bit 0=L-bit, bit 1=Cp-bit)*/
    VM_TRANSFORM_RTP_MP3_ENCODE_PACKET_SIZE,    /*!< RTP MP3 Encode packet size.*/
    VM_TRANSFORM_RTP_MP3_ENCODE_MANAGE_TIMING,  /*!< RTP MP3 Encode Manage Timing.*/
    VM_TRANSFORM_RTP_MP3_ENCODE_STATS,          /*!< RTP MP3 Encode Statistics.*/
    VM_TRANSFORM_HID_KEYBOARD_IDLE_RATE,        /*!< HID Keyboard idle rate */
    VM_TRANSFORM_HID_TYPE,                      /*!< HID device type : key,mouse or any */
    VM_TRANSFORM_HID_SET_MODE,                  /*!< HID mode : boot or report */
    VM_TRANSFORM_HID_ADD_PRE,                   /*!< HID add pre : register a handle for 
                                                       tranform pre-fix processing
                                                       and octet to add in the beginning.

                                                       Use the "hid_add_pre_info" structure 
                                                       to pass information. It is upto the 
                                                       application to release the allocated 
                                                       memory. */
    VM_TRANSFORM_HID_DISCARD_PRE,                /*!< HID discard pre : octets to discard in the begining */
    VM_TRANSFORM_RTP_PAYLOAD_HEADER_SIZE,        /*!< Payload header size.*/
    VM_TRANSFORM_RTP_DECODE_DONT_REMOVE_PAYLOAD_HEADER, /*!< Is payload header required for RTP decode? */
    VM_TRANSFORM_RTP_ENCODE_PACKET_SIZE,        /*!< RTP Encode packet size.*/
    VM_TRANSFORM_RTP_ENCODE_MANAGE_TIMING,      /*!< RTP Encode Manage Timing */
    VM_TRANSFORM_RTP_ENCODE_STATS,              /*!< RTP Encode statistics */
    VM_TRANSFORM_RTP_ENCODE_FRAME_PERIOD,       /*!< Frame period per packet in us */
    VM_TRANSFORM_RTP_ENCODE_EARLY_LIMIT,        /*!< Early limit time in us*/
    VM_TRANSFORM_RTP_CODEC_TYPE,                /*!< RTP Codec type*/
    VM_TRANSFORM_HID_REPORT_MAX_SIZE,            /*!< HID report maximum size */
    VM_TRANSFORM_COPY_DROP_DATA,                /*!< Whether to drop data when Sink is full */
    VM_TRANSFORM_COPY_FRAGMENT_DATA,            /*!< Whether data fragmentation is allowed */
    VM_TRANSFORM_PACKETISE_MODE,                /*!< The packetise format used */
    VM_TRANSFORM_PACKETISE_CPENABLE,            /*!< Configure whether copy protect is enabled */
    VM_TRANSFORM_PACKETISE_CODEC,               /*!< The codec to be used for the packetiser */
    VM_TRANSFORM_PACKETISE_SCMS,                /*!< Configure SCMS-T protection level */
    VM_TRANSFORM_PACKETISE_SAMPLE_RATE,         /*!< Sample rate of the data in the packetiser (only SBC codec) */
    VM_TRANSFORM_PACKETISE_MTU,                 /*!< MTU of the packetiser packets */
    VM_TRANSFORM_PACKETISE_TIME_BEFORE_TTP,     /*!< Time to transmit packets before TTP (msec) */
    VM_TRANSFORM_PACKETISE_LATEST_TIME_BEFORE_TTP, /*!< Latest time possible to transmit packets (msec) */
    VM_TRANSFORM_CLK_CONVERT_START_OFFSET,      /*!< This is the position of first byte of first occurrence
                                                     of timing information with in the source data packet.
                                                     - 0xFFFF for this offset would mean that there
                                                       is no timing information in the source data.
                                                     - Since the timing information is 4 bytes in size, valid range for
                                                       start offset would be from 0 to 0xFFFB. */
    VM_TRANSFORM_CLK_CONVERT_REPETITION_OFFSET, /*!< This value is defined as the offset from start offset (SO) to the
                                                     first byte of second timing information (if any). The position
                                                     of all the remaining timing information (if any) will be at repetition offset (RO)
                                                     from the previous. Offset of any timing information from the start of
                                                     packet can be calculated as timing_info_start_offset[i] = SO + (i* RO) where
                                                     - i is from 0 to repetitions (NR)
                                                     - i = 0 refers to the first timing information at SO
                                                     - Since the timing information data is 4 bytes, valid range for RO would be from 4 to 0xFFFC.
                                                     */
    VM_TRANSFORM_CLK_CONVERT_NUM_REPETITIONS,   /*!< Number of timing informations after the first timing information to be converted.
                                                      - NR = 0 represents only one timing information exist in source data.
                                                      - Valid range for this can be 0 to 0xFFFE.
                                                      - NR = 0xFFFF  would mean to convert all the timing informations
                                                        present in packet.*/
    VM_TRANSFORM_HASH_PREFIX_RTP_HEADER,        /*!< Prefix an RTP header to source data.
                                                     Default value 0 which means do not
                                                     prefix and 1 would mean prefix */
    VM_TRANSFORM_HASH_RTP_PAYLOAD_TYPE,         /*!< Payload type for RTP header to be prefixed */
    VM_TRANSFORM_HASH_RTP_SSRC_LOWER,           /*!< lower 16 bits of SSRC for RTP header to be prefixed */
    VM_TRANSFORM_HASH_RTP_SSRC_UPPER,           /*!< Upper 16 bits of SSRC for RTP header to be prefixed */
    VM_TRANSFORM_HASH_SOURCE_OFFSET,            /*!< The byte offset in the source data to start hashing */
    VM_TRANSFORM_HASH_SOURCE_SIZE,              /*!< The number of bytes on which hash would be calculated.
                                                     A value 0xFFFF for this would mean hash up to the end of
                                                     the packet */
    VM_TRANSFORM_HASH_SOURCE_MODIFY_OFFSET,     /*!< The byte offset of source data at which two octets
                                                     would be overwritten by the lower 16 bits of calculated hash.
                                                     A value of 0xFFFF for this would mean that the hash (16 bits)
                                                     would be written at the end of the source data. */
    VM_TRANSFORM_PACKETISE_TTP_DELAY,                /*!< Time adjustment(msec) to be applied to TTP of sent/received packets (only TWSPLUS mode).  
                                                          Set a positive value to delay the received TTP or negative value to advance the received TTP.*/
    VM_TRANSFORM_PACKETISE_TTP_DELAY_SSRC_TRIGGER_1, /*!< Trigger for first SSRC based TTP adjustment (only TWSPLUS mode) */
    VM_TRANSFORM_PACKETISE_TTP_DELAY_SSRC_1,         /*!< Time adjustment(msec) to be applied to TTP when first SSRC trigger has been met 
                                                          (only TWSPLUS mode). Set a positive value to delay the received TTP  or negative 
                                                          value to advance the received TTP.*/
    VM_TRANSFORM_PACKETISE_TTP_DELAY_SSRC_TRIGGER_2, /*!< Trigger for second SSRC based TTP adjustment (only TWSPLUS mode) */
    VM_TRANSFORM_PACKETISE_TTP_DELAY_SSRC_2,         /*!< Time adjustment(msec) to be applied to TTP when second SSRC trigger has been met 
                                                          (only TWSPLUS mode). Set a positive value to delay the received TTP  or negative 
                                                          value to advance the received TTP.*/
    VM_TRANSFORM_PACKETISE_TTP_DELAY_SSRC_TRIGGER_1_HI, /*!< Upper 16-bits of trigger for first SSRC based TTP adjustment (only TWSPLUS mode) */
    VM_TRANSFORM_PACKETISE_TTP_DELAY_SSRC_TRIGGER_2_HI, /*!< Upper 16-bits of trigger for second SSRC based TTP adjustment (only TWSPLUS mode) */
    VM_TRANSFORM_PACKETISE_RTP_SSRC,                /*!< RTP SSRC lower 16 bits (only TWSPLUS mode) */
    VM_TRANSFORM_PACKETISE_RTP_SSRC_HI,             /*!< RTP SSRC upper 16 bits (only TWSPLUS mode) */
    VM_TRANSFORM_PACKETISE_RTP_SPLIT_MODE_CHANNELS, /*!< Enable/disable, and which channels to render in split mode */
    VM_TRANSFORM_PACKETISE_TTP_SSRC_PAIR_COUNT,     /*!< The number of SSRC pairs in the array pointed to by TTP_SSRC_PAIR_ADDR_[HI|LO]. The array
                                                         must be maintained for the lifetime of this value being non-zero. This array is searched
                                                         instead of TTP_DELAY_SSRC_[1|2] if set */
    VM_TRANSFORM_PACKETISE_TTP_SSRC_PAIR_ADDR_HI,   /*!< Upper 16-bits of SSRC pair array address */
    VM_TRANSFORM_PACKETISE_TTP_SSRC_PAIR_ADDR_LO,   /*!< Lower 16-bits of SSRC pair array address */
    VM_TRANSFORM_PACKETISE_SAMPLE_RATE_HI           /*!< Upper 16-bits of sample rate of the data in the packetiser (only SBC codec) */
} vm_transform_config_key;

typedef struct
{
    uint32 ssrc;                /*!< Trigger SSRC */
    int16  ttp_adjust;          /*!< TTP adjustment to apply (msec) */
} vm_transform_ttp_ssrc_pair;

typedef enum
{
    VM_TRANSFORM_RTP_CODEC_APTX,                /*!< Set CODEC to APTX */
    VM_TRANSFORM_RTP_CODEC_SBC,                 /*!< Set CODEC to SBC */
    VM_TRANSFORM_RTP_CODEC_ATRAC,               /*!< Set CODEC to ATRAC */
    VM_TRANSFORM_RTP_CODEC_MP3,                 /*!< Set CODEC to MP3 */
    VM_TRANSFORM_RTP_CODEC_AAC                  /*!< Set CODEC to AAC */
}vm_transform_rtp_codec_type;

typedef enum
{
    VM_TRANSFORM_PACKETISE_MODE_TWS,            /*!< Use TWS packet format between devices */
    VM_TRANSFORM_PACKETISE_MODE_TWSPLUS,        /*!< Use TWS+ packet format between devices */
    VM_TRANSFORM_PACKETISE_MODE_RTP             /*!< Use RTP packet format for ShareMe */
} vm_transform_packetise_mode;

typedef enum
{
    VM_TRANSFORM_PACKETISE_SCMS_COPY_ALLOWED = 0,
    VM_TRANSFORM_PACKETISE_SCMS_COPY_ONCE = 1,
    VM_TRANSFORM_PACKETISE_SCMS_COPY_PROHIBITED = 3
} vm_transform_packetise_scms;

typedef enum
{
    VM_TRANSFORM_PACKETISE_CODEC_SBC,           /*!< Set codec to SBC */
    VM_TRANSFORM_PACKETISE_CODEC_APTX,          /*!< Set codec to AptX */
    VM_TRANSFORM_PACKETISE_CODEC_AAC,           /*!< Set codec to AAC */
    VM_TRANSFORM_PACKETISE_CODEC_APTX_ADAPTIVE, /*!< Set codec to AptX Adaptive*/
    VM_TRANSFORM_PACKETISE_CODEC_MAX            /*!< Internal use only */
} vm_transform_packetise_codec;

typedef enum
{
    VM_TRANSFORM_PACKETISE_RTP_SPLIT_MODE_DISABLE = 0,      /*!< Disable split mode decoding */
    VM_TRANSFORM_PACKETISE_RTP_SPLIT_MODE_PLAY_LEFT = 1,    /*!< Enable and play left channel data */
    VM_TRANSFORM_PACKETISE_RTP_SPLIT_MODE_PLAY_RIGHT = 2,   /*!< Enable and play right channel data */
    VM_TRANSFORM_PACKETISE_RTP_SPLIT_MODE_PLAY_BOTH = 3,    /*!< Enable and play both channels */
    VM_TRANSFORM_PACKETISE_RTP_SPLIT_MODE_MAX = 3           /*!< Internal use only */
} vm_transform_packetise_rtp_split_mode;

typedef enum
{
    PCM_CLK_256,    /*!< Set PCM CLK rate to 256kHz.*/
    PCM_CLK_128,    /*!< Set PCM CLK rate to 128kHz.*/
    PCM_CLK_512,    /*!< Set PCM CLK rate to 512kHz.*/
    PCM_CLK_OFF     /*!< Disable PCM CLK output.*/
}vm_pcm_clock_setting;

typedef enum
{
    MCLK_DISABLE,   /*!< Stop generating/using an MCLK signal */
    MCLK_ENABLE_OUTPUT, /*!< Generate an internal MCLK signal and also output it on the configured pin for an external codec */
    MCLK_ENABLE     /*!< Generate an internal MCLK signal */
} vm_mclk_enable;

enum {
    INVALID_SENSOR = -32766,        /*!< Returned by VmGetTemperatureBySensor() when called with an invalid sensor number.*/
    INVALID_TEMPERATURE = -32767    /*!< Returned by VmGetTemperature() and VmGetTemperatureBySensor() when the temperature cannot be determined.*/
};

typedef enum
{
    TSENSOR_MAIN,   /*!< Main temperature sensor.*/
    TSENSOR_PMU     /*!< PMU temperature sensor.*/
} vm_temp_sensor;


/*!
  @brief Keys for TransformQuery().
*/
typedef enum
{
    VM_TRANSFORM_QUERY_PACKETISE_SSRC,  /*!< Get the current SSRC value from the RTP stream
                                             if present. If the packetiser is not configured
                                             to receive RTP packets, the query will return FALSE */
    VM_TRANSFORM_QUERY_BITRATE  /*!< Get the transform's bitrate statistics. Provide a pointer to
                                     a previously allocated vm_transform_query_bitrate_t structure. */
} vm_transform_query_key;

/*!
  @brief Structures used for TransformQuery responses
*/
typedef struct
{
    uint32 timestamp;           /*!< timestamp of last update (local clock in us) */
    uint32 last_ttp;            /*!< ttp of the last audioframe handled (us) */
    uint32 bytes_processed;     /*!< Cumulative number of bytes processed */
    uint32 packets_processed;   /*!< Cumulative number of packets processed */
} vm_transform_query_bitrate_t;


/*!
  @brief Enumerated type indicating reset source. This is the return type of
         VmGetResetSource().

         When a system reset occurs on BC7 chips a value is stored which
         relates to the cause of the reset.  This value can be retrieved by a
         VM app and is represented by the following enumerated values.
*/
typedef enum
{
    /*! Power on or RST# asserted. */
    RESET_SOURCE_POWER_ON,

    /*! Firmware reset. */
    RESET_SOURCE_FIRMWARE,

    /*! BREAK signal received on the UART.
     *  When a host computer is linked to the device by its UART, it can request
     *  a reset by issuing a BREAK request, i.e. holding the UART's Rx line low
     *  for a set period of time.  This period is given by a non-zero value of
     *  PSKEY_HOSTIO_UART_RESET_TIMEOUT. */
    RESET_SOURCE_UART_BREAK,

    /*! The charger has been plugged into the device.
     *  This is only valid on CSR8670 and CSR8670-like chips and is enabled
     *  using PSKEY_RESET_ON_CHARGER_ATTACH. */
    RESET_SOURCE_CHARGER,

    /*! Reset caused by other hardware.
     *  Any value not covered by this definition cannot be determined and is
     *  deemed an unexpected reset */
    UNEXPECTED_RESET,
        
    /*! Reset caused by panic. */
    RESET_SOURCE_PANIC,

    /*! SDIO reset. */
    RESET_SOURCE_SDIO,

    /* Reset due to toolcmd request. */
    RESET_SOURCE_TOOLCMD_FULL_RESET,
    RESET_SOURCE_TOOLCMD_SUBSYS_RESET,

    /* Reset due to issue in the App Subsystem */
    RESET_SOURCE_APP_SUBSYS_RESET,
    RESET_SOURCE_APP_PANIC,
    RESET_SOURCE_APP_WATCHDOG,

    /*! Reset from Janitor watchdog. */
    RESET_SOURCE_WATCHDOG,
    RESET_SOURCE_DORMANT_WAKEUP,

    /*! Reset from host interface subsystem. */
    RESET_SOURCE_HOST_RESET,

    /*! Reset from a transaction bridge. */
    RESET_SOURCE_TBRIDGE_RESET,

    /*! Reset from debug reset register write. */
    RESET_SOURCE_DEBUG_RESET,

    /*! Curator reason not mapped. */
    UNEXPECTED_RESET_REASON_RECEIVED

} vm_reset_source;

/*!
    @brief Bit masks representing the source of the voltage regulator enable
           signal on (re)boot. This type is used by VmGetPowerSource() to
           construct the bit-pattern which is its return value.

           On CSR8670 and CSR8670-like chips, there are three ways to keep the
           chip's power regulators enabled.  These are represented by the
           following enumerated values.

           Any combination of these is possible, dependent upon prevailing
           conditions and PSKEY settings.
*/
typedef enum
{
    /*! The charger has been plugged in to the device. */
    POWER_ENABLER_CHARGER_ATTACHED = 1 << 0,

    /*! The Reset Protection Circuit has been triggered.
     *  This circuit determines whether the power regulator should be kept on
     *  during a reset, allowing the chip to reboot.  This would occur, for
     *  example, after an ESD strike. */
    POWER_ENABLER_RESET_PROTECTION = 1 << 1,

    /*! The Vreg enable signal was active during the boot sequence. */
    POWER_ENABLER_VREG_EN = 1 << 2,

    /*! The battery has been attached to the device. */
    POWER_ENABLER_VBAT_ATTACHED = 1 << 3,

    /*! The device came on as a result of a PBR request. */
    POWER_ENABLER_POST_BOOT_RESET = 1 << 4
} vm_power_enabler;

/*!
    @brief  Dfu from SQIF operation status
*/
typedef enum
{
    DFU_SQIF_STATUS_SUCCESS = 0x0001,  /*!< dfu from sqif status success.*/
    DFU_SQIF_STATUS_ERROR  = 0x0002  /*!< dfu from sqif file validation failure.*/
} vm_dfu_sqif_status;

/*!
    @brief Wake source IDs.
*/
typedef enum
{
    WAKE_SOURCE_CAPACITIVE_SENSOR = 0x0001  /*!< CapacitiveSensor wake source ID.*/
} vm_wake_source_type;

/*!
    @brief
        To disable the VM software watchdog, Application has to call
        VmSoftwareWdKick trap with three disable codes(53281, 60681, 0)
        in a sequence.
        Eg: 
            VmSoftwareWdKick(VM_SW_WATCHDOG_DISABLE_CODE1)
            VmSoftwareWdKick(VM_SW_WATCHDOG_DISABLE_CODE2)
            VmSoftwareWdKick(VM_SW_WATCHDOG_DISABLE_CODE3)
*/
#define VM_SW_WATCHDOG_DISABLE_CODE1 ((uint16)0xD021)
#define VM_SW_WATCHDOG_DISABLE_CODE2 ((uint16)0xED09)
#define VM_SW_WATCHDOG_DISABLE_CODE3 ((uint16)0x0000)


/*!
    @brief Enumerated type representing run time profiles to use with
           VmRequestRunTimeProfile() and VmGetRunTimeProfile() on supported
           chips.
        The profile affects the application subsystem only and does not reconfigure 
        other blocks of the system such as the radio link or sleep.
        Note that all profiles might not be available on all products. 
        However a profile can still be requested.     
    @sa VmRequestRunTimeProfile(), VmGetRunTimeProfile()
    @sa Other functions that can affect overall power consumption such as 
        VmDeepSleepEnable(), VmTransmitPowerSetMaximum(), etc. 
*/
typedef enum
{
    /*! Power Saving profile. 
        Enables additional VM power saving options. There can be a reduction 
        in execution and run-time performance. 
        This profile is recommended when runtime performance is not required or 
        additional power saving is desired. It will not be applied immediately
        if the runtime framework requires additional performance.
    */
    VM_POWER_SAVING,

    /*! Balance between power saving and performance. 
        Enables some VM power saving options. All runtime performance options 
        are not enabled.
        This profile is recommended for general or typical use. It will not be 
        applied immediately if the runtime framework requires additional performance.
    */
    VM_BALANCED,

    /*! Performance Profile.
        Enables additional VM performance options such as the runtime execution 
        speed and flash interface configuration.
        This can be used when additional processing power is required, such as
        lower latency app execution or time critical code execution not met by 
        the \c VM_POWER_SAVING and \c VM_BALANCED profiles. It comes at the 
        expense of power consumption. This profile is usually applied immediately.
    */
    VM_PERFORMANCE
} vm_runtime_profile;

/*!
    @brief Component IDs. Used in VmGetFwVersion()
*/
typedef enum
{
    FIRMWARE_ID = 0x0000, /*!< Firmware id.*/
    APPLICATION_ID = 0x0001 /*!< Application id.*/
} component_id;

/*! @brief Mask used in VmReadSecurity or VmEnableSecurity functions
    to know which bit will be read/write
*/
typedef enum
{
    SECURITY_ENABLE = 0x0001, /*!< Enable the use of the customer set OTP
    security key. This includes SQIF encryption, Curator filesystem external
    authentication and USB debug locking with an eFuse key*/
    SECURITY_DEBUG_TRANSPORT = 0x0002, /*!< Enable locking of the TrB and SPI
    debug interfaces. If SECURITY_ENABLE is set this will also enable the use of
    the customer-set eFuse security key for unlocking the USB debug interface.*/
    SECURITY_USBDBG_DISABLED = 0x0004, /*!< Once set, this will permanently
    disable all access to the USB debugger. It is intended to be set at the
    final stage of the production line by customers.*/
    SECURITY_DEBUG_ALLOWED = 0x0008 /*!< Allow USB debug unlocking via the USB
    hub filter driver. Designed to be blown by developers that only have access
    to USB debug - should never be set on production devices.*/
} security_bits;

/*! @brief Structure used to represent debug statistics for SCO connection.
 */
typedef struct sco_statistics
{
    /*! Total SCO packets successfully dispatched/scheduled. */
    uint16 success_dispatched;
    /*! Total SCO reserved packets dispatched late. */
    uint16 late_dispatched;
    /*! eSCO retransmissions (TX slots). */
    uint16 tx_slot;
    /*! eSCO retransmissions (RX slots). */
    uint16 rx_slot;
    /*! Total good SCO packets received. */
    uint16 good_packets_received;
} sco_statistics_t;

/*! @brief Structure used to represent the next sniff clock.
 */
typedef struct next_sniff_clock
{
    /*! Next sniff instant with 1 us resolution */
    uint32 next_sniff_clock;
    /*! 0 if the link is in active mode and the remaining sniff related
     * parameters are not valid. */
    uint16 sniff_interval;
    /*! Next sniff subrate instant with 1 us resolution */
    uint32 next_subrate_clock;
    /*! Subrate value being used in the transmit direction */
    uint8 tx_subrate;
    /*! Subrate value being used in the receive direction */
    uint8 rx_subrate;
} next_sniff_clock_t;

/*! @brief Structure used to represent the TX enhanced parameters.

Packet_types: 
              0x00 NULL packets
              0x01 POLL packets
              0x02 FHS packets
              0x03 DM1 packets
              0x04 DH1 packets
              0x0A DM3 packets
              0x0B DH3 packets
              0x0E DM5 packets
              0x0F DH5 packets
              0x09 AUX1 packets
              0x24 2-DH1 packets
              0x2A 2-DH3 packets
              0x2E 2-DH5 packets
              0x28 3-DH1 packets
              0x2B 3-DH3 packets
              0x2F 3-DH5 packets
              0x05 HV1 packets
              0x06 HV2 packets
              0x07 HV3 packets
              0x08 DV packets
              0x17 EV3 packets
              0x1C EV4 packets
              0x1D EV5 packets
              0x36 2-EV3 packets
              0x3C 2-EV5 packets
              0x37 3-EV3 packets
              0x3D 3-EV5 packets
              0x40 LE1M packets
              0x41 LE2M packets
              0x42 LE Coded 125k packets
              0x43 LE Coded 500k packets
              0x52 QHS-P2 packets
              0x53 QHS-P3 packets
              0x54 QHS-P4 packets
              0x55 QHS-P5 packets
              0x56 QHS-P6 packets
              Others Reserved
              
Payload types:
                0x00 All 0s
                0x01 All 1s
                0x02 Alternate Bits 10101010...
                0x03 Alternate Nibbles 11110000...
                0x04 PRBS9
                0x05 PRBS15
                0x06 Alternate Bits inverted 01010101...
                0x07 Alternate Nibbles inverted 00001111...
                0x08 Pattern defined in Payload_Pattern
                Others Reserved
 */

typedef struct tx_enhanced_params
{
    /*! Channel to transmit on. 
     *    0-78 used for BR/EDR
     *    0-39 used for LE
     *    0-39 used for QHS  */
    uint8 channel;
    /*! Type of packet to transmit. */
    uint8 packet_type;
    /*! Length of the payload, in bytes. */
    uint16 payload_length;
    /*! Bits pattern sent. */
    uint8 payload_type;
    /*! Pattern to be used in test packets when payload_type = 0x08 */
    uint32 payload_pattern;
    /*! Transmit mode
     *    0x00 Burst packets 
     *    0x01 Continuous wave mode */
    uint8 transmit_type;
    /*! Logical transport address. 
     *      0x01 to 0x07 BR/EDR 
     *      Unused LE and QHS  */
    uint8 lt_addr;
    /*! Hopping mode.
     *  Device starts at channel, then hops over the range of 
     * channels using the pattern: 
     *  next_channel = (current_channel + n) MOD max_channel
     * Note: When n=0, the same channel is always used
     */
    unsigned int hopping : 4;
    /*! Number of edge channels to remove from hopping.
     * Remove n band edge channels
     * Note: when n=0, no band edge channels are removed
     */
    unsigned int edge_band_removal : 2;
    /*! Number of edge channels to remove from hopping.
     * 0x0 Use all channels
     * 0x1 Use odd channels
     * 0x2 Use even channels */
    unsigned int odd_even_channels : 2;
   /*! Pointer to Bluetooth Address 
    *  For BR/EDR Packets 0x000000000000 to 0xFFFFFFFFFFFF 
    * 
    * For LE Packets Ignored. Access Address from Test Specification used.
    * 
    * For QHS Packets 0x000000000000 to 0x0000FFFFFFFF
    * QHS 32-bit Access Address used to create Coded Access Address
    * The LAP used for initializing the CRC is the lower 24-bits of 
    * this field identical to BR/EDR */
    BD_ADDR_T* bt_addr;
    /*! Maximum number of packets to transmit after TX mode is initiated. 
    *  After this count the test mode will automatically end
    * If the count is set to 0 then the device will transmit indefinitely,
    * unless explicitly stopped with the Test2RfStop() */
    uint32 max_packets;
    /*! Number of off slots between TX packets. 
     *  Note - field only for burst mode (not continuous)*/
    uint8 off_slots;
    /*! The transmit power table lookup value to use. 
            Range 0 and 11 may vary with the device. */
    uint8 power;
} tx_enhanced_params_t;

/*! @brief Structure used to represent the RX enhanced parameters.
 */

typedef struct rx_enhanced_params
{
    /*! Channel to receive on. */
    uint8 channel;
    /*! Type of packet expected. */
    uint8 packet_type;
    /*! Receiver mode */
    uint8 receive_type;
    /*! Logical transport address. */
    uint8 lt_addr;
    /*! Hopping mode.*/
    unsigned int hopping : 4;
    /*! Number of edge channels to remove from hopping.*/
    unsigned int edge_band_removal : 2;
    /*! Number of edge channels to remove from hopping.*/
    unsigned int odd_even_channels : 2;
    /*! Pointer to Bluetooth Address */
    BD_ADDR_T* bt_addr;
    /*! Maximum number of packets to receive after RX mode is initiated. */
    uint32 max_packets;
    /*! Number of off slots between RX packets. */
    uint8 off_slots;
    /*! The receiver gain table lookup value to use. */
    uint8 rx_gain;
} rx_enhanced_params_t;
 

/*! @brief Structure used to represent the RX enhanced statistics.
 */
typedef struct rx_stats_enhanced
{
    /*! Number of packets received (no access code error) */
    uint32 packets_received;
    /*! Number of packets with access code errors (not received) */
    uint32 access_code_errors;
    /*! Number of packets with HEC errors */
    uint32 hec_errors;
    /*! Number of packets with CRC errors */
    uint32 crc_errors;
    /*! Packet Bit error count */
    uint32 total_bit_errors;
    /*! Average RSSI in two’s complement format */
    uint32 rssi; 
 } rx_stats_enhanced_t;

#endif  /* __APP_VM_IF_H__ */
