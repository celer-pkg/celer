// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
// Qt-Security score:significant reason:default

#include "canbusutil.h"

#include <QCoreApplication>
#include <QTextStream>

CanBusUtil::CanBusUtil(QTextStream &output, QCoreApplication &app, QObject *parent) :
    QObject(parent),
    m_canBus(QCanBus::instance()),
    m_output(output),
    m_app(app),
    m_readTask(new ReadTask(output, this))
{
}

void CanBusUtil::setShowTimeStamp(bool showTimeStamp)
{
    m_readTask->setShowTimeStamp(showTimeStamp);
}

void CanBusUtil::setShowFlags(bool showFlags)
{
    m_readTask->setShowFlags(showFlags);
}

void CanBusUtil::setConfigurationParameter(QCanBusDevice::ConfigurationKey key,
                                           const QVariant &value)
{
    m_configurationParameter[key] = value;
}

bool CanBusUtil::start(const QString &pluginName, const QString &deviceName, const QString &data)
{
    if (!m_canBus) {
        m_output << tr("Error: Cannot create QCanBus.") << Qt::endl;
        return false;
    }

    m_pluginName = pluginName;
    m_deviceName = deviceName;
    m_data = data;
    m_listening = data.isEmpty();

    if (!connectCanDevice())
        return false;

    if (m_listening) {
        if (m_readTask->isShowFlags())
             m_canDevice->setConfigurationParameter(QCanBusDevice::CanFdKey, true);
        connect(m_canDevice.get(), &QCanBusDevice::framesReceived,
                m_readTask, &ReadTask::handleFrames);
    } else {
        if (!sendData())
            return false;
        QTimer::singleShot(0, &m_app, QCoreApplication::quit);
    }

    return true;
}

int CanBusUtil::printPlugins()
{
    if (!m_canBus) {
        m_output << tr("Error: Cannot create QCanBus.") << Qt::endl;
        return 1;
    }

    const QStringList plugins = m_canBus->plugins();
    for (const QString &plugin : plugins)
        m_output << plugin << Qt::endl;
    return 0;
}

int CanBusUtil::printDevices(const QString &pluginName)
{
    if (!m_canBus) {
        m_output << tr("Error: Cannot create QCanBus.") << Qt::endl;
        return 1;
    }

    QString errorMessage;
    const QList<QCanBusDeviceInfo> devices = m_canBus->availableDevices(pluginName, &errorMessage);
    if (!errorMessage.isEmpty()) {
        m_output << tr("Error gathering available devices: '%1'").arg(errorMessage) << Qt::endl;
        return 1;
    }

    for (const QCanBusDeviceInfo &info : devices)
        m_output << info.name() << Qt::endl;
    return 0;
}

bool CanBusUtil::parseDataField(QCanBusFrame::FrameId &id, QString &payload)
{
    int hashMarkPos = m_data.indexOf('#');
    if (hashMarkPos < 0) {
        m_output << tr("Data field invalid: No hash mark found!") << Qt::endl;
        return false;
    }

    id = QStringView{m_data}.left(hashMarkPos).toUInt(nullptr, 16);
    payload = m_data.right(m_data.size() - hashMarkPos - 1);

    return true;
}

bool CanBusUtil::setFrameFromPayload(QString payload, QCanBusFrame *frame)
{
    if (!payload.isEmpty() && payload.at(0).toUpper() == 'R') {
        frame->setFrameType(QCanBusFrame::RemoteRequestFrame);

        if (payload.size() == 1) // payload = "R"
            return true;

        bool ok = false;
        int rtrFrameLength = QStringView{payload}.mid(1).toInt(&ok);
        if (ok && rtrFrameLength >= 0 && rtrFrameLength <= 8) { // payload = "R8"
            frame->setPayload(QByteArray(rtrFrameLength, 0));
            return true;
        }

        m_output << tr("Error: RTR frame length must be between 0 and 8 (including).") << Qt::endl;
        return false;
    }

    if (!payload.isEmpty() && payload.at(0) == '#') {
        frame->setFlexibleDataRateFormat(true);
        payload.remove(0, 1);
    }

    const QRegularExpression re(QStringLiteral("^[0-9A-Fa-f]*$"));
    if (!re.match(payload).hasMatch()) {
        m_output << tr("Data field invalid: Only hex numbers allowed.") << Qt::endl;
        return false;
    }

    if (payload.size() % 2 != 0) {
        if (frame->hasFlexibleDataRateFormat()) {
            enum { BitrateSwitchFlag = 1, ErrorStateIndicatorFlag = 2 };
            const int flags = QStringView{payload}.left(1).toInt(nullptr, 16);
            frame->setBitrateSwitch(flags & BitrateSwitchFlag);
            frame->setErrorStateIndicator(flags & ErrorStateIndicatorFlag);
            payload.remove(0, 1);
        } else {
            m_output << tr("Data field invalid: Size is not multiple of two.") << Qt::endl;
            return false;
        }
    }

    QByteArray bytes = QByteArray::fromHex(payload.toLatin1());

    const int maxSize = frame->hasFlexibleDataRateFormat() ? 64 : 8;
    if (bytes.size() > maxSize) {
        m_output << tr("Data field invalid: Size is longer than %1 bytes.").arg(maxSize) << Qt::endl;
        return false;
    }

    frame->setPayload(bytes);

    return true;
}

bool CanBusUtil::connectCanDevice()
{
    if (!m_canBus->plugins().contains(m_pluginName)) {
        m_output << tr("Cannot find CAN bus plugin '%1'.").arg(m_pluginName) << Qt::endl;
        return false;
    }

    m_canDevice.reset(m_canBus->createDevice(m_pluginName, m_deviceName));
    if (!m_canDevice) {
        m_output << tr("Cannot create CAN bus device: '%1'").arg(m_deviceName) << Qt::endl;
        return false;
    }

    const auto constEnd = m_configurationParameter.constEnd();
    for (auto i = m_configurationParameter.constBegin(); i != constEnd; ++i)
        m_canDevice->setConfigurationParameter(i.key(), i.value());

    connect(m_canDevice.get(), &QCanBusDevice::errorOccurred, m_readTask, &ReadTask::handleError);
    if (!m_canDevice->connectDevice()) {
        m_output << tr("Cannot create CAN bus device: '%1'").arg(m_deviceName) << Qt::endl;
        return false;
    }

    return true;
}

bool CanBusUtil::sendData()
{
    quint32 id;
    QString payload;
    QCanBusFrame frame;

    if (!parseDataField(id, payload))
        return false;

    if (!setFrameFromPayload(payload, &frame))
        return false;

    if (id > 0x1FFFFFFF) { // 29 bits
        m_output << tr("Cannot send invalid frame ID: '%1'").arg(id, 0, 16) << Qt::endl;
        return false;
    }

    frame.setFrameId(id);

    if (frame.hasFlexibleDataRateFormat())
        m_canDevice->setConfigurationParameter(QCanBusDevice::CanFdKey, true);

    return m_canDevice->writeFrame(frame);
}
