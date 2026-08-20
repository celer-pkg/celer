// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef SOCKETCANBACKEND_H
#define SOCKETCANBACKEND_H

#include <QtSerialBus/qcanbusframe.h>
#include <QtSerialBus/qcanbusdevice.h>
#include <QtSerialBus/qcanbusdeviceinfo.h>

#include <QtCore/qsocketnotifier.h>
#include <QtCore/qstring.h>
#include <QtCore/qvariant.h>

// The order of the following includes is mandatory, because some
// distributions use sa_family_t in can.h without including socket.h
#include <sys/socket.h>
#include <sys/uio.h>
#include <linux/can.h>
#include <sys/time.h>

#include <memory>

#ifndef CANFD_MTU
// CAN FD support was added by Linux kernel 3.6
// For prior kernels we redefine the missing defines here
// they are taken from linux/can/raw.h & linux/can.h

enum {
    CAN_RAW_FD_FRAMES = 5
};

#define CAN_MAX_DLEN 8
#define CANFD_MAX_DLEN 64
struct canfd_frame {
    canid_t can_id;  /* 32 bit CAN_ID + EFF/RTR/ERR flags */
    __u8    len;     /* frame payload length in byte */
    __u8    flags;   /* additional flags for CAN FD */
    __u8    __res0;  /* reserved / padding */
    __u8    __res1;  /* reserved / padding */
    __u8    data[CANFD_MAX_DLEN] __attribute__((aligned(8)));
};
#define CAN_MTU     (sizeof(struct can_frame))
#define CANFD_MTU   (sizeof(struct canfd_frame))

#endif

QT_BEGIN_NAMESPACE

class LibSocketCan;

class SocketCanBackend : public QCanBusDevice
{
    Q_OBJECT
public:
    explicit SocketCanBackend(const QString &name);
    ~SocketCanBackend();

    bool open() override;
    void close() override;

    void setConfigurationParameter(ConfigurationKey key, const QVariant &value) override;

    bool writeFrame(const QCanBusFrame &newData) override;

    QString interpretErrorFrame(const QCanBusFrame &errorFrame) override;

    static QCanBusDeviceInfo socketCanDeviceInfo(const QString &deviceName);
    static QList<QCanBusDeviceInfo> interfaces();

    void resetController() override;
    bool hasBusStatus() const override;
    CanBusStatus busStatus() override;
    QCanBusDeviceInfo deviceInfo() const override;

private Q_SLOTS:
    void readSocket();

private:
    void resetConfigurations();
    bool connectSocket();
    bool applyConfigurationParameter(ConfigurationKey key, const QVariant &value);

    int protocol = CAN_RAW;
    canfd_frame m_frame;
    sockaddr_can m_address;
    msghdr m_msg;
    iovec m_iov;
    sockaddr_can m_addr;
    char m_ctrlmsg[CMSG_SPACE(sizeof(timeval)) + CMSG_SPACE(sizeof(__u32))];

    qint64 canSocket = -1;
    QSocketNotifier *notifier = nullptr;
    std::unique_ptr<LibSocketCan> libSocketCan;
    QString canSocketName;
    bool canFdOptionEnabled = false;
};

QT_END_NAMESPACE

#endif // SOCKETCANBACKEND_H
