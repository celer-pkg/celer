// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANFRAMEPROCESSOR_H
#define QCANFRAMEPROCESSOR_H

#include <QtCore/QVariantMap>

#include <QtSerialBus/qcancommondefinitions.h>
#include <QtSerialBus/qtserialbusglobal.h>

#include <memory>

QT_BEGIN_NAMESPACE

class QCanBusFrame;
class QCanMessageDescription;
class QCanUniqueIdDescription;
class QCanFrameProcessorPrivate;

class QCanFrameProcessor
{
public:
    enum class Error : quint8 {
        None = 0,
        InvalidFrame,
        UnsupportedFrameFormat,
        Decoding,
        Encoding,
    };

    struct ParseResult {
        QtCanBus::UniqueId uniqueId = QtCanBus::UniqueId{0};
        QVariantMap signalValues;
    };

    Q_SERIALBUS_EXPORT QCanFrameProcessor();
    Q_SERIALBUS_EXPORT ~QCanFrameProcessor();

    Q_SERIALBUS_EXPORT QCanBusFrame prepareFrame(QtCanBus::UniqueId uniqueId,
                                                 const QVariantMap &signalValues);
    Q_SERIALBUS_EXPORT ParseResult parseFrame(const QCanBusFrame &frame);

    Q_SERIALBUS_EXPORT Error error() const;
    Q_SERIALBUS_EXPORT QString errorString() const;
    Q_SERIALBUS_EXPORT QStringList warnings() const;

    Q_SERIALBUS_EXPORT QList<QCanMessageDescription> messageDescriptions() const;
    Q_SERIALBUS_EXPORT
    void addMessageDescriptions(const QList<QCanMessageDescription> &descriptions);
    Q_SERIALBUS_EXPORT
    void setMessageDescriptions(const QList<QCanMessageDescription> &descriptions);
    Q_SERIALBUS_EXPORT void clearMessageDescriptions();

    Q_SERIALBUS_EXPORT QCanUniqueIdDescription uniqueIdDescription() const;
    Q_SERIALBUS_EXPORT void setUniqueIdDescription(const QCanUniqueIdDescription &description);

private:
    std::unique_ptr<QCanFrameProcessorPrivate> d;
    friend class QCanFrameProcessorPrivate;

    Q_DISABLE_COPY_MOVE(QCanFrameProcessor)
};

QT_END_NAMESPACE

#endif // QCANFRAMEPROCESSOR_H
