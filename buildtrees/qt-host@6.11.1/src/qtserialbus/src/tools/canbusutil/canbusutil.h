// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
// Qt-Security score:significant reason:default

#ifndef CANBUSUTIL_H
#define CANBUSUTIL_H

#include "readtask.h"

#include <QObject>

QT_BEGIN_NAMESPACE

class QCanBusFrame;
class QCoreApplication;
class QTextStream;

QT_END_NAMESPACE

class CanBusUtil : public QObject
{
    Q_OBJECT
public:
    explicit CanBusUtil(QTextStream &output, QCoreApplication &app, QObject *parent = nullptr);

    void setShowTimeStamp(bool showTimeStamp);
    void setShowFlags(bool showFlags);
    void setConfigurationParameter(QCanBusDevice::ConfigurationKey key, const QVariant &value);
    bool start(const QString &pluginName, const QString &deviceName, const QString &data = QString());
    int  printPlugins();
    int  printDevices(const QString &pluginName);

private:
    bool parseDataField(QCanBusFrame::FrameId &id, QString &payload);
    bool setFrameFromPayload(QString payload, QCanBusFrame *frame);
    bool connectCanDevice();
    bool sendData();

private:
    QCanBus *m_canBus = nullptr;
    QTextStream &m_output;
    QCoreApplication &m_app;
    bool m_listening = false;
    QString m_pluginName;
    QString m_deviceName;
    QString m_data;
    std::unique_ptr<QCanBusDevice> m_canDevice;
    ReadTask *m_readTask = nullptr;
    using ConfigurationParameter = QHash<QCanBusDevice::ConfigurationKey, QVariant>;
    ConfigurationParameter m_configurationParameter;
};

#endif // CANBUSUTIL_H
