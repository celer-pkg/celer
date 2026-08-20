// Copyright (C) 2017 Denis Shienkov <denis.shienkov@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef VECTORCAN_SYMBOLS_P_H
#define VECTORCAN_SYMBOLS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifdef LINK_LIBVECTORCAN

extern "C"
{
#include <vxlapi.h>
}

#else

#include <QtCore/qlibrary.h>
#include <QtCore/qstring.h>
#include <QtCore/qdebug.h>

#ifdef Q_OS_WIN32
#  include <windows.h>
#else
#  error "Unsupported platform"
#endif

// transceiver types: CAN Cab
#define XL_TRANSCEIVER_TYPE_NONE                    0x0000
#define XL_TRANSCEIVER_TYPE_CAN_251                 0x0001
#define XL_TRANSCEIVER_TYPE_CAN_252                 0x0002
#define XL_TRANSCEIVER_TYPE_CAN_DNOPTO              0x0003
#define XL_TRANSCEIVER_TYPE_CAN_SWC_PROTO           0x0005 // Prototype. Driver may latch-up.
#define XL_TRANSCEIVER_TYPE_CAN_SWC                 0x0006
#define XL_TRANSCEIVER_TYPE_CAN_EVA                 0x0007
#define XL_TRANSCEIVER_TYPE_CAN_FIBER               0x0008
#define XL_TRANSCEIVER_TYPE_CAN_1054_OPTO           0x000B // 1054 with optical isolation
#define XL_TRANSCEIVER_TYPE_CAN_SWC_OPTO            0x000C // SWC with optical isolation
#define XL_TRANSCEIVER_TYPE_CAN_B10011S             0x000D // B10011S truck-and-trailer
#define XL_TRANSCEIVER_TYPE_CAN_1050                0x000E // 1050
#define XL_TRANSCEIVER_TYPE_CAN_1050_OPTO           0x000F // 1050 with optical isolation
#define XL_TRANSCEIVER_TYPE_CAN_1041                0x0010 // 1041
#define XL_TRANSCEIVER_TYPE_CAN_1041_OPTO           0x0011 // 1041 with optical isolation
#define XL_TRANSCEIVER_TYPE_LIN_6258_OPTO           0x0017 // Vector LINcab 6258opto with transceiver Infineon TLE6258
#define XL_TRANSCEIVER_TYPE_LIN_6259_OPTO           0x0019 // Vector LINcab 6259opto with transceiver Infineon TLE6259
#define XL_TRANSCEIVER_TYPE_DAIO_8444_OPTO          0x001D // Vector IOcab 8444  (8 dig.Inp.; 4 dig.Outp.; 4 ana.Inp.; 4 ana.Outp.)
#define XL_TRANSCEIVER_TYPE_CAN_1041A_OPTO          0x0021 // 1041A with optical isolation
#define XL_TRANSCEIVER_TYPE_LIN_6259_MAG            0x0023 // LIN transceiver 6259, with transceiver Infineon TLE6259, magnetically isolated, stress functionality
#define XL_TRANSCEIVER_TYPE_LIN_7259_MAG            0x0025 // LIN transceiver 7259, with transceiver Infineon TLE7259, magnetically isolated, stress functionality
#define XL_TRANSCEIVER_TYPE_LIN_7269_MAG            0x0027 // LIN transceiver 7269, with transceiver Infineon TLE7269, magnetically isolated, stress functionality
#define XL_TRANSCEIVER_TYPE_CAN_1054_MAG            0x0033 // TJA1054, magnetically isolated, with selectable termination resistor (via 4th IO line)
#define XL_TRANSCEIVER_TYPE_CAN_251_MAG             0x0035 // 82C250/251 or equivalent, magnetically isolated
#define XL_TRANSCEIVER_TYPE_CAN_1050_MAG            0x0037 // TJA1050, magnetically isolated
#define XL_TRANSCEIVER_TYPE_CAN_1040_MAG            0x0039 // TJA1040, magnetically isolated
#define XL_TRANSCEIVER_TYPE_CAN_1041A_MAG           0x003B // TJA1041A, magnetically isolated
#define XL_TRANSCEIVER_TYPE_TWIN_CAN_1041A_MAG      0x0080 // TWINcab with two TJA1041, magnetically isolated
#define XL_TRANSCEIVER_TYPE_TWIN_LIN_7269_MAG       0x0081 // TWINcab with two 7259, Infineon TLE7259, magnetically isolated, stress functionality
#define XL_TRANSCEIVER_TYPE_TWIN_CAN_1041AV2_MAG    0x0082 // TWINcab with two TJA1041, magnetically isolated
#define XL_TRANSCEIVER_TYPE_TWIN_CAN_1054_1041A_MAG 0x0083 // TWINcab with TJA1054A and TJA1041A with magnetic isolation
// transceiver types: CAN PiggyBack
#define XL_TRANSCEIVER_TYPE_PB_CAN_251              0x0101
#define XL_TRANSCEIVER_TYPE_PB_CAN_1054             0x0103
#define XL_TRANSCEIVER_TYPE_PB_CAN_251_OPTO         0x0105
#define XL_TRANSCEIVER_TYPE_PB_CAN_SWC              0x010B
// 0x010D not supported, 0x010F, 0x0111, 0x0113 reserved for future use!!
#define XL_TRANSCEIVER_TYPE_PB_CAN_1054_OPTO        0x0115
#define XL_TRANSCEIVER_TYPE_PB_CAN_SWC_OPTO         0x0117
#define XL_TRANSCEIVER_TYPE_PB_CAN_TT_OPTO          0x0119
#define XL_TRANSCEIVER_TYPE_PB_CAN_1050             0x011B
#define XL_TRANSCEIVER_TYPE_PB_CAN_1050_OPTO        0x011D
#define XL_TRANSCEIVER_TYPE_PB_CAN_1041             0x011F
#define XL_TRANSCEIVER_TYPE_PB_CAN_1041_OPTO        0x0121
#define XL_TRANSCEIVER_TYPE_PB_LIN_6258_OPTO        0x0129 // LIN piggy back with transceiver Infineon TLE6258
#define XL_TRANSCEIVER_TYPE_PB_LIN_6259_OPTO        0x012B // LIN piggy back with transceiver Infineon TLE6259
#define XL_TRANSCEIVER_TYPE_PB_LIN_6259_MAG         0x012D // LIN piggy back with transceiver Infineon TLE6259, magnetically isolated, stress functionality
#define XL_TRANSCEIVER_TYPE_PB_CAN_1041A_OPTO       0x012F // CAN transceiver 1041A
#define XL_TRANSCEIVER_TYPE_PB_LIN_7259_MAG         0x0131 // LIN piggy back with transceiver Infineon TLE7259, magnetically isolated, stress functionality
#define XL_TRANSCEIVER_TYPE_PB_LIN_7269_MAG         0x0133 // LIN piggy back with transceiver Infineon TLE7269, magnetically isolated, stress functionality
#define XL_TRANSCEIVER_TYPE_PB_CAN_251_MAG          0x0135 // 82C250/251 or compatible, magnetically isolated
#define XL_TRANSCEIVER_TYPE_PB_CAN_1050_MAG         0x0136 // TJA 1050, magnetically isolated
#define XL_TRANSCEIVER_TYPE_PB_CAN_1040_MAG         0x0137 // TJA 1040, magnetically isolated
#define XL_TRANSCEIVER_TYPE_PB_CAN_1041A_MAG        0x0138 // TJA 1041A, magnetically isolated
#define XL_TRANSCEIVER_TYPE_PB_DAIO_8444_OPTO       0x0139 // optically isolated IO piggy
#define XL_TRANSCEIVER_TYPE_PB_CAN_1054_MAG         0x013B // TJA1054, magnetically isolated, with selectable termination resistor (via 4th IO line)
#define XL_TRANSCEIVER_TYPE_CAN_1051_CAP_FIX        0x013C // TJA1051 - fixed transceiver on e.g. 16xx/8970
#define XL_TRANSCEIVER_TYPE_DAIO_1021_FIX           0x013D // Onboard IO of VN1630/VN1640
#define XL_TRANSCEIVER_TYPE_LIN_7269_CAP_FIX        0x013E // TLE7269 - fixed transceiver on 1611
#define XL_TRANSCEIVER_TYPE_PB_CAN_1051_CAP         0x013F // TJA 1051, capacitive isolated
#define XL_TRANSCEIVER_TYPE_PB_CAN_SWC_7356_CAP     0x0140 // Single Wire NCV7356, capacitive isolated
#define XL_TRANSCEIVER_TYPE_PB_CAN_1055_CAP         0x0141 // TJA1055, capacitive isolated, with selectable termination resistor (via 4th IO line)
#define XL_TRANSCEIVER_TYPE_PB_CAN_1057_CAP         0x0142 // TJA 1057, capacitive isolated
// transceiver types: FlexRay PiggyBacks
#define XL_TRANSCEIVER_TYPE_PB_FR_1080              0x0201 // TJA 1080
#define XL_TRANSCEIVER_TYPE_PB_FR_1080_MAG          0x0202 // TJA 1080 magnetically isolated piggy
#define XL_TRANSCEIVER_TYPE_PB_FR_1080A_MAG         0x0203 // TJA 1080A magnetically isolated piggy
#define XL_TRANSCEIVER_TYPE_PB_FR_1082_CAP          0x0204 // TJA 1082 capacitive isolated piggy
#define XL_TRANSCEIVER_TYPE_PB_FRC_1082_CAP         0x0205 // TJA 1082 capacitive isolated piggy with CANpiggy form factor

#define XL_TRANSCEIVER_TYPE_ETH_BCM54810_FIX        0x0230 // Onboard Broadcom PHY on VN5610

// IOpiggy 8642
#define XL_TRANSCEIVER_TYPE_PB_DAIO_8642            0x0280 // Iopiggy for VN8900

// transceiver Operation Modes
#define XL_TRANSCEIVER_LINEMODE_NA          ((quint32)0x0000)
#define XL_TRANSCEIVER_LINEMODE_TWO_LINE    ((quint32)0x0001)
#define XL_TRANSCEIVER_LINEMODE_CAN_H       ((quint32)0x0002)
#define XL_TRANSCEIVER_LINEMODE_CAN_L       ((quint32)0x0003)
#define XL_TRANSCEIVER_LINEMODE_SWC_SLEEP   ((quint32)0x0004) // SWC Sleep Mode.
#define XL_TRANSCEIVER_LINEMODE_SWC_NORMAL  ((quint32)0x0005) // SWC Normal Mode.
#define XL_TRANSCEIVER_LINEMODE_SWC_FAST    ((quint32)0x0006) // SWC High-Speed Mode.
#define XL_TRANSCEIVER_LINEMODE_SWC_WAKEUP  ((quint32)0x0007) // SWC Wakeup Mode.
#define XL_TRANSCEIVER_LINEMODE_SLEEP       ((quint32)0x0008)
#define XL_TRANSCEIVER_LINEMODE_NORMAL      ((quint32)0x0009)
#define XL_TRANSCEIVER_LINEMODE_STDBY       ((quint32)0x000a) // Standby for those who support it
#define XL_TRANSCEIVER_LINEMODE_TT_CAN_H    ((quint32)0x000b) // truck & trailer: operating mode single wire using CAN high
#define XL_TRANSCEIVER_LINEMODE_TT_CAN_L    ((quint32)0x000c) // truck & trailer: operating mode single wire using CAN low
#define XL_TRANSCEIVER_LINEMODE_EVA_00      ((quint32)0x000d) // CANcab Eva
#define XL_TRANSCEIVER_LINEMODE_EVA_01      ((quint32)0x000e) // CANcab Eva
#define XL_TRANSCEIVER_LINEMODE_EVA_10      ((quint32)0x000f) // CANcab Eva
#define XL_TRANSCEIVER_LINEMODE_EVA_11      ((quint32)0x0010) // CANcab Eva

// transceiver Status Flags (not all used, but for compatibility reasons)
#define XL_TRANSCEIVER_STATUS_PRESENT           ((quint32)0x0001)
#define XL_TRANSCEIVER_STATUS_POWER             ((quint32)0x0002)
#define XL_TRANSCEIVER_STATUS_MEMBLANK          ((quint32)0x0004)
#define XL_TRANSCEIVER_STATUS_MEMCORRUPT        ((quint32)0x0008)
#define XL_TRANSCEIVER_STATUS_POWER_GOOD        ((quint32)0x0010)
#define XL_TRANSCEIVER_STATUS_EXT_POWER_GOOD    ((quint32)0x0020)
#define XL_TRANSCEIVER_STATUS_NOT_SUPPORTED     ((quint32)0x0040)

// common event tags
#define XL_RECEIVE_MSG              ((quint16)0x0001)
#define XL_CHIP_STATE               ((quint16)0x0004)
#define XL_TRANSCEIVER_INFO         ((quint16)0x0006)
#define XL_TRANSCEIVER              (XL_TRANSCEIVER_INFO)
#define XL_TIMER_EVENT              ((quint16)0x0008)
#define XL_TIMER                    (XL_TIMER_EVENT)
#define XL_TRANSMIT_MSG             ((quint16)0x000A)
#define XL_SYNC_PULSE               ((quint16)0x000B)
#define XL_APPLICATION_NOTIFICATION ((quint16)0x000F)

// CAN/CAN-FD event tags Rx
#define XL_CAN_EV_TAG_RX_OK         ((quint16)0x0400)
#define XL_CAN_EV_TAG_RX_ERROR      ((quint16)0x0401)
#define XL_CAN_EV_TAG_TX_ERROR      ((quint16)0x0402)
#define XL_CAN_EV_TAG_TX_REQUEST    ((quint16)0x0403)
#define XL_CAN_EV_TAG_TX_OK         ((quint16)0x0404)
#define XL_CAN_EV_TAG_CHIP_STATE    ((quint16)0x0409)

// CAN/CAN-FD event tags Tx
#define XL_CAN_EV_TAG_TX_MSG        ((quint16)0x0440)
#define XL_CAN_EV_TAG_TX_ERRFR      ((quint16)0x0441)

// Bus types
#define XL_BUS_TYPE_NONE        0x00000000
#define XL_BUS_TYPE_CAN         0x00000001

#include <pshpack1.h>
typedef quint64 XLaccess;
typedef HANDLE XLhandle;

// message flags
#ifndef MAX_MSG_LEN
#define MAX_MSG_LEN 8
#endif

// interface version for our events
#define XL_INTERFACE_VERSION_V2 2
#define XL_INTERFACE_VERSION_V3 3
#define XL_INTERFACE_VERSION_V4 4
//current version
#define XL_INTERFACE_VERSION    XL_INTERFACE_VERSION_V3

#define XL_CAN_EXT_MSG_ID   0x80000000U

#define XL_CAN_MSG_FLAG_ERROR_FRAME     0x01
#define XL_CAN_MSG_FLAG_OVERRUN         0x02 // Overrun in Driver or CAN Controller, previous msgs have been lost.
#define XL_CAN_MSG_FLAG_NERR            0x04 // Line Error on Lowspeed
#define XL_CAN_MSG_FLAG_WAKEUP          0x08 // High Voltage Message on Single Wire CAN
#define XL_CAN_MSG_FLAG_REMOTE_FRAME    0x10
#define XL_CAN_MSG_FLAG_RESERVED_1      0x20
#define XL_CAN_MSG_FLAG_TX_COMPLETED    0x40 // Message Transmitted
#define XL_CAN_MSG_FLAG_TX_REQUEST      0x80 // Transmit Message stored into Controller
#define XL_CAN_MSG_FLAG_SRR_BIT_DOM     0x0200 // SRR bit in CAN message is dominant

#define XL_EVENT_FLAG_OVERRUN           0x01 // Used in XLevent.flags

#define XL_CAN_MAX_DATA_LEN             64

// structure for XL_RECEIVE_MSG, XL_TRANSMIT_MSG (32 bytes)
struct s_xl_can_msg {
    unsigned long id;
    quint16 flags;
    quint16 dlc;
    quint64 res1;
    quint8 data[MAX_MSG_LEN];
    quint64 res2;
};
static_assert(sizeof(s_xl_can_msg) == 32, "Invalid size of s_xl_can_msg structure");

// defines for SET_OUTPUT_MODE
#define XL_OUTPUT_MODE_SILENT           0 // switch CAN trx into default silent mode
#define XL_OUTPUT_MODE_NORMAL           1 // switch CAN trx into normal mode
#define XL_OUTPUT_MODE_TX_OFF           2 // switch CAN trx into silent mode with tx pin off
#define XL_OUTPUT_MODE_SJA_1000_SILENT  3 // switch CAN trx into SJA1000 silent mode

// Transceiver modes
#define XL_TRANSCEIVER_EVENT_ERROR      1
#define XL_TRANSCEIVER_EVENT_CHANGED    2

#define XL_CHIPSTAT_BUSOFF              0x01
#define XL_CHIPSTAT_ERROR_PASSIVE       0x02
#define XL_CHIPSTAT_ERROR_WARNING       0x04
#define XL_CHIPSTAT_ERROR_ACTIVE        0x08

#define XL_CAN_STATE_FLAG_SJA_MODE      0x00000001

#define XL_CANFD_RX_EVENT_HEADER_SIZE        32
#define XL_CANFD_MAX_EVENT_SIZE              128

// CAN Chip status
struct s_xl_chip_state {
    unsigned char busStatus;
    unsigned char txErrorCounter;
    unsigned char rxErrorCounter;
    unsigned char chipState;           // raw Status Register Value
    unsigned int  flags;
};
static_assert(sizeof(s_xl_chip_state) == 8, "Invalid size of s_xl_chip_state structure");

// basic bus message structure
union s_xl_tag_data {
    struct s_xl_can_msg msg;
    struct s_xl_chip_state chipState;
};

// event type definition (48 bytes)
typedef struct s_xl_event {
    quint8 tag;
    quint8 chanIndex;
    quint16 transId;
    quint16 portHandle;
    quint8 flags; // e.g. XL_EVENT_FLAG_OVERRUN
    quint8 reserved;
    quint64 timeStamp;
    union s_xl_tag_data tagData; // 32 bytes
} XLevent;
static_assert(sizeof(s_xl_event) == 48, "Invalid size of s_xl_event structure");

typedef struct {
    quint32 id;
    quint32 flags;
    quint32 crc;
    quint8 reserved1[12];
    quint16 totalBitCnt;
    quint8 dlc;
    quint8 reserved[5];
    quint8 data[XL_CAN_MAX_DATA_LEN];
} XL_CAN_EV_RX_MSG;

typedef struct {
    quint32 canId;
    quint32 msgFlags;
    quint8 dlc;
    quint8 reserved1;
    quint16 reserved;
    quint8 data[XL_CAN_MAX_DATA_LEN];
} XL_CAN_EV_TX_REQUEST;

typedef struct {
    quint8 errorCode;
    quint8 reserved[95];
} XL_CAN_EV_ERROR;

typedef struct {
    quint8 busStatus;
    quint8 txErrorCounter;
    quint8 rxErrorCounter;
    quint8 reserved;
    quint32 reserved0;
} XL_CAN_EV_CHIP_STATE;

typedef struct s_xl_sync_pulse_ev {
    quint32 triggerSource;
    quint32 reserved;
    quint64 time;
} XL_SYNC_PULSE_EV;

typedef XL_SYNC_PULSE_EV XL_CAN_EV_SYNC_PULSE;

typedef struct {
    quint32 size;
    quint16 tag;
    quint16 channelIndex;
    quint32 userHandle;
    quint16 flagsChip;
    quint16 reserved0;
    quint64 reserved1;
    quint64 timeStamp;
    union {
        quint8 raw[XL_CANFD_MAX_EVENT_SIZE - XL_CANFD_RX_EVENT_HEADER_SIZE];
        XL_CAN_EV_RX_MSG canRxOkMsg;
        XL_CAN_EV_RX_MSG canTxOkMsg;
        XL_CAN_EV_TX_REQUEST canTxRequest;
        XL_CAN_EV_ERROR canError;
        XL_CAN_EV_CHIP_STATE canChipState;
        XL_CAN_EV_SYNC_PULSE canSyncPulse;
    } tagData;
} XLcanRxEvent;

typedef struct {
     quint32 id;
     quint32 flags;
     quint8 dlc;
     quint8 reserved[7];
     quint8 data[XL_CAN_MAX_DATA_LEN];
} XL_CAN_TX_MSG;

typedef struct {
    quint16 tag;
    quint16 transId;
    quint8 channelIndex;
    quint8 reserved[3];
    union {
        XL_CAN_TX_MSG canMsg;
    } tagData;
} XLcanTxEvent;


// build a channels mask from the channels index
#define XL_CHANNEL_MASK(x)  (quint64(1) << (x))

#define XL_MAX_APPNAME  32

// driver status
typedef qint16  XLstatus;

#define XL_SUCCESS                      0
#define XL_PENDING                      1

#define XL_ERR_QUEUE_IS_EMPTY           10
#define XL_ERR_QUEUE_IS_FULL            11
#define XL_ERR_TX_NOT_POSSIBLE          12
#define XL_ERR_NO_LICENSE               14
#define XL_ERR_WRONG_PARAMETER          101
#define XL_ERR_TWICE_REGISTER           110
#define XL_ERR_INVALID_CHAN_INDEX       111
#define XL_ERR_INVALID_ACCESS           112
#define XL_ERR_PORT_IS_OFFLINE          113
#define XL_ERR_CHAN_IS_ONLINE           116
#define XL_ERR_NOT_IMPLEMENTED          117
#define XL_ERR_INVALID_PORT             118
#define XL_ERR_HW_NOT_READY             120
#define XL_ERR_CMD_TIMEOUT              121
#define XL_ERR_HW_NOT_PRESENT           129
#define XL_ERR_NOTIFY_ALREADY_ACTIVE    131
#define XL_ERR_NO_RESOURCES             152
#define XL_ERR_WRONG_CHIP_TYPE          153
#define XL_ERR_WRONG_COMMAND            154
#define XL_ERR_INVALID_HANDLE           155
#define XL_ERR_RESERVED_NOT_ZERO        157
#define XL_ERR_INIT_ACCESS_MISSING      158
#define XL_ERR_CANNOT_OPEN_DRIVER       201
#define XL_ERR_WRONG_BUS_TYPE           202
#define XL_ERR_DLL_NOT_FOUND            203
#define XL_ERR_INVALID_CHANNEL_MASK     204
#define XL_ERR_NOT_SUPPORTED            205
// special stream defines
#define XL_ERR_CONNECTION_BROKEN        210
#define XL_ERR_CONNECTION_CLOSED        211
#define XL_ERR_INVALID_STREAM_NAME      212
#define XL_ERR_CONNECTION_FAILED        213
#define XL_ERR_STREAM_NOT_FOUND         214
#define XL_ERR_STREAM_NOT_CONNECTED     215
#define XL_ERR_QUEUE_OVERRUN            216
#define XL_ERROR                        255

// defines for xlGetDriverConfig structures
#define XL_MAX_LENGTH           31
#define XL_CONFIG_MAX_CHANNELS  64

// flags for channelCapabilities
#define XL_CHANNEL_FLAG_TIME_SYNC_RUNNING   0x00000001
#define XL_CHANNEL_FLAG_CANFD_BOSCH_SUPPORT 0x20000000
#define XL_CHANNEL_FLAG_CANFD_ISO_SUPPORT   0x80000000
#define XL_CHANNEL_FLAG_CANFD_SUPPORT       (XL_CHANNEL_FLAG_CANFD_BOSCH_SUPPORT | XL_CHANNEL_FLAG_CANFD_ISO_SUPPORT)

// activate - channel flags
#define XL_ACTIVATE_NONE        0
#define XL_ACTIVATE_RESET_CLOCK 8

#define XL_BUS_COMPATIBLE_CAN       XL_BUS_TYPE_CAN

// the following bus types can be used with the current cab / piggy
#define XL_BUS_ACTIVE_CAP_CAN       (XL_BUS_COMPATIBLE_CAN << 16)

// acceptance filter
#define XL_CAN_STD  01 // flag for standard ID's
#define XL_CAN_EXT  02 // flag for extended ID's

#define XL_CAN_TXMSG_FLAG_EDL                0x0001    // extended data length
#define XL_CAN_TXMSG_FLAG_BRS                0x0002    // baud rate switch
#define XL_CAN_TXMSG_FLAG_RTR                0x0010    // remote transmission request
#define XL_CAN_TXMSG_FLAG_HIGHPRIO           0x0080    // high priority message - clears all send buffers - then transmits
#define XL_CAN_TXMSG_FLAG_WAKEUP             0x0200    // generate a wakeup message

#define XL_CAN_RXMSG_FLAG_EDL                0x0001    // extended data length
#define XL_CAN_RXMSG_FLAG_BRS                0x0002    // baud rate switch
#define XL_CAN_RXMSG_FLAG_ESI                0x0004    // error state indicator
#define XL_CAN_RXMSG_FLAG_RTR                0x0010    // remote transmission request
#define XL_CAN_RXMSG_FLAG_EF                 0x0200    // error frame (only posssible in XL_CAN_EV_TX_REQUEST/XL_CAN_EV_TX_REMOVED)
#define XL_CAN_RXMSG_FLAG_ARB_LOST           0x0400    // Arbitration Lost
                                                       // set if the receiving node tried to transmit a message but lost arbitration process
#define XL_CAN_RXMSG_FLAG_WAKEUP             0x2000    // high voltage message on single wire CAN
#define XL_CAN_RXMSG_FLAG_TE                 0x4000    // 1: transceiver error detected

typedef struct {
    quint32 busType;
    union {
        struct {
            quint32 bitRate;
            quint8 sjw;
            quint8 tseg1;
            quint8 tseg2;
            quint8 sam;
            quint8 outputMode;
        } can;
        quint8 raw[32];
    } data;
} XLbusParams;

// porthandle
#define XL_INVALID_PORTHANDLE   (-1)
typedef long XLportHandle, *pXLportHandle;

// defines for FPGA core types (fpgaCoreCapabilities)
#define XL_FPGA_CORE_TYPE_NONE  0
#define XL_FPGA_CORE_TYPE_CAN   1

// defines for special DeviceStatus
#define XL_SPECIAL_DEVICE_STAT_FPGA_UPDATE_DONE 0x01 // automatic driver FPGA flashing done

#define XL_HWTYPE_NONE                           0
#define XL_HWTYPE_VIRTUAL                        1

typedef struct s_xl_channel_config {
    char name[XL_MAX_LENGTH + 1];
    quint8 hwType; // HWTYPE_xxxx (see above)
    quint8 hwIndex; // Index of the hardware (same type) (0,1,...)
    quint8 hwChannel; // Index of the channel (same hardware) (0,1,...)
    quint16 transceiverType; // TRANSCEIVER_TYPE_xxxx (see above)
    quint16 transceiverState; // transceiver state (XL_TRANSCEIVER_STATUS...)
    quint16 configError; // XL_CHANNEL_CONFIG_ERROR_XXX (see above)
    quint8 channelIndex; // Global channel index (0,1,...)
    quint64 channelMask; // Global channel mask (=1<<channelIndex)
    quint32 channelCapabilities; // capabilities which are supported (e.g CHANNEL_FLAG_XXX)
    quint32 channelBusCapabilities; // what buses are supported and which are possible to be activated (e.g. XXX_BUS_ACTIVE_CAP_CAN)

    // channel
    quint8 isOnBus; // The channel is on bus
    quint32 connectedBusType; // currently selected bus
    XLbusParams busParams;

    quint32 driverVersion;
    quint32 interfaceVersion; // version of interface with driver
    quint32 raw_data[10];

    quint32 serialNumber;
    quint32 articleNumber;

    char transceiverName[XL_MAX_LENGTH + 1]; // name for CANcab or another transceiver

    quint32 specialCabFlags; // XL_SPECIAL_CAB_LIN_RECESSIVE_STRESS, XL_SPECIAL_CAB_LIN_DOMINANT_TIMEOUT flags
    quint32 dominantTimeout; // Dominant Timeout in us.
    quint8 dominantRecessiveDelay; // Delay in us.
    quint8 recessiveDominantDelay; // Delay in us.
    quint8 connectionInfo; // XL_CONNECTION_INFO_XXX
    quint8 currentlyAvailableTimestamps; // XL_CURRENTLY_AVAILABLE_TIMESTAMP...
    quint16 minimalSupplyVoltage; // Minimal Supply Voltage of the Cab/Piggy in 1/100 V
    quint16 maximalSupplyVoltage; // Maximal Supply Voltage of the Cab/Piggy in 1/100 V
    quint32 maximalBaudrate; // Maximal supported LIN baudrate
    quint8 fpgaCoreCapabilities; // e.g.: XL_FPGA_CORE_TYPE_XXX
    quint8 specialDeviceStatus; // e.g.: XL_SPECIAL_DEVICE_STAT_XXX
    quint16 channelBusActiveCapabilities; // like channelBusCapabilities (but without core dependencies)
    quint16 breakOffset; // compensation for edge asymmetry in ns
    quint16 delimiterOffset; // compensation for edgdfde asymmetry in ns
    quint32 reserved[3];
} XLchannelConfig, *pXLchannelConfig;

typedef struct s_xl_driver_config {
    quint32 dllVersion;
    quint32 channelCount; // total number of channels
    quint32 reserved[10];
    XLchannelConfig channel[XL_CONFIG_MAX_CHANNELS]; // [channelCount]
} XLdriverConfig, *pXLdriverConfig;

// structure for the acceptance filter
typedef struct _XLacc_filt {
    quint8 isSet;
    unsigned long code;
    unsigned long mask; // relevant = 1
} XLaccFilt;

// structure for the acceptance filter of one CAN chip
typedef struct _XLacceptance {
    XLaccFilt std;
    XLaccFilt xtd;
} XLacceptance;

typedef struct s_XLcanFdConf {
    unsigned int arbitrationBitRate;
    unsigned int sjwAbr;
    unsigned int tseg1Abr;
    unsigned int tseg2Abr;
    unsigned int dataBitRate;
    unsigned int sjwDbr;
    unsigned int tseg1Dbr;
    unsigned int tseg2Dbr;
    unsigned char reserved;
    unsigned char options;
    unsigned char reserved1[2];
    unsigned int reserved2;
} XLcanFdConf;

// defines for xlSetGlobalTimeSync
#define XL_SET_TIMESYNC_NO_CHANGE   ((unsigned long)0)
#define XL_SET_TIMESYNC_ON          ((unsigned long)1)
#define XL_SET_TIMESYNC_OFF         ((unsigned long)2)

#include <poppack.h>

#define GENERATE_SYMBOL_VARIABLE(returnType, symbolName, ...) \
    typedef returnType (WINAPI *fp_##symbolName)(__VA_ARGS__); \
    static fp_##symbolName symbolName;

#define RESOLVE_SYMBOL(symbolName) \
    symbolName = reinterpret_cast<fp_##symbolName>(vectorcanLibrary->resolve(#symbolName)); \
    if (!symbolName) \
        return false;

GENERATE_SYMBOL_VARIABLE(XLstatus, xlOpenDriver, void)
GENERATE_SYMBOL_VARIABLE(XLstatus, xlCloseDriver, void)
GENERATE_SYMBOL_VARIABLE(XLstatus, xlGetDriverConfig, XLdriverConfig *)
GENERATE_SYMBOL_VARIABLE(XLstatus, xlOpenPort, XLportHandle *, char *, XLaccess, XLaccess *, quint32, quint32, quint32)
GENERATE_SYMBOL_VARIABLE(XLstatus, xlClosePort, XLportHandle)
GENERATE_SYMBOL_VARIABLE(XLstatus, xlActivateChannel, XLportHandle, XLaccess, quint32, quint32)
GENERATE_SYMBOL_VARIABLE(XLstatus, xlDeactivateChannel, XLportHandle, XLaccess)
GENERATE_SYMBOL_VARIABLE(XLstatus, xlCanSetChannelBitrate, XLportHandle, XLaccess, quint32)
GENERATE_SYMBOL_VARIABLE(XLstatus, xlCanTransmit, XLportHandle, XLaccess, quint32 *, void *)
GENERATE_SYMBOL_VARIABLE(XLstatus, xlReceive, XLportHandle, quint32 *, XLevent *)
GENERATE_SYMBOL_VARIABLE(XLstatus, xlSetNotification, XLportHandle, XLhandle *, int)
GENERATE_SYMBOL_VARIABLE(XLstatus, xlCanRequestChipState, XLportHandle, XLaccess)
GENERATE_SYMBOL_VARIABLE(char *, xlGetErrorString, XLstatus)
GENERATE_SYMBOL_VARIABLE(XLstatus, xlCanFdSetConfiguration, XLportHandle, XLaccess, XLcanFdConf *)
GENERATE_SYMBOL_VARIABLE(XLstatus, xlCanReceive, XLportHandle, XLcanRxEvent *)
GENERATE_SYMBOL_VARIABLE(XLstatus, xlCanTransmitEx, XLportHandle, XLaccess, quint32, quint32 *, XLcanTxEvent *)
GENERATE_SYMBOL_VARIABLE(XLaccess, xlGetChannelMask, int, int, int)

inline bool resolveVectorCanSymbols(QLibrary *vectorcanLibrary)
{
    if (!vectorcanLibrary->isLoaded()) {
#ifdef Q_PROCESSOR_X86_64
        vectorcanLibrary->setFileName(QStringLiteral("vxlapi64"));
#else
        vectorcanLibrary->setFileName(QStringLiteral("vxlapi"));
#endif
        if (!vectorcanLibrary->load())
            return false;
    }

    RESOLVE_SYMBOL(xlOpenDriver)
    RESOLVE_SYMBOL(xlCloseDriver)
    RESOLVE_SYMBOL(xlGetDriverConfig)
    RESOLVE_SYMBOL(xlOpenPort)
    RESOLVE_SYMBOL(xlClosePort)
    RESOLVE_SYMBOL(xlActivateChannel)
    RESOLVE_SYMBOL(xlDeactivateChannel)
    RESOLVE_SYMBOL(xlCanSetChannelBitrate)
    RESOLVE_SYMBOL(xlCanTransmit)
    RESOLVE_SYMBOL(xlReceive)
    RESOLVE_SYMBOL(xlSetNotification)
    RESOLVE_SYMBOL(xlCanRequestChipState)
    RESOLVE_SYMBOL(xlGetErrorString)
    RESOLVE_SYMBOL(xlCanFdSetConfiguration)
    RESOLVE_SYMBOL(xlCanTransmitEx)
    RESOLVE_SYMBOL(xlCanReceive)
    RESOLVE_SYMBOL(xlGetChannelMask)
    return true;
}

#endif

#endif // VECTORCAN_SYMBOLS_P_H
