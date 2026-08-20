// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#include "qquickeventreplayclient_p.h"

#include <private/qpacket_p.h>
#include <private/qqmldebugconnection_p.h>
#include <private/qqmlprofilerdefinitions_p.h>

#include <QtCore/qxmlstream.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

QQuickEventReplayClient::QQuickEventReplayClient(QQmlDebugConnection *connection)
    : QQmlDebugClient(QLatin1String("EventReplay"), connection)
{
}

struct SendEventType {
    int eventType = -1;
    int detailType = -1;
};

bool QQuickEventReplayClient::sendEvents(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file" << fileName << "for reading";
        return false;
    }

    int currentEventIndex = -1;

    QHash<int, SendEventType> types;

    QXmlStreamReader reader(&file);
    while (!reader.atEnd()) {
        if (reader.readNext() != QXmlStreamReader::StartElement)
            continue;

        const QStringView name = reader.name();
        if (name == "event"_L1) {
            currentEventIndex = reader.attributes().value("index"_L1).toInt();
        } else if (name == "mouseEvent"_L1) {
            types[currentEventIndex] = SendEventType {
                QQmlProfilerDefinitions::EventType::Mouse,
                reader.readElementText().toInt(),
            };
        } else if (name == "keyEvent"_L1) {
            types[currentEventIndex] = SendEventType {
                QQmlProfilerDefinitions::EventType::Key,
                reader.readElementText().toInt(),
            };
        } else if (name == "range"_L1) {
            const QXmlStreamAttributes attributes = reader.attributes();
            const int eventIndex = attributes.value("eventIndex"_L1).toInt();
            const SendEventType &type = types[eventIndex];
            if (type.eventType != -1) {
                const qint64 startTime = attributes.value("startTime"_L1).toLongLong();
                const int inputType = attributes.value("type"_L1).toInt();
                const int data1 = attributes.value("data1"_L1).toInt();
                const int data2 = attributes.value("data2"_L1).toInt();

                QPacket stream(connection()->currentDataStreamVersion());
                stream << startTime << QQmlProfilerDefinitions::Message::Event
                       << type.detailType << inputType << data1 << data2;
                sendMessage(stream.data());
            }
        }
    }

    return true;
}

void QQuickEventReplayClient::sendEvent(
        const QQmlProfilerEventType &type, const QQmlProfilerEvent &event)
{
    QPacket stream(connection()->currentDataStreamVersion());
    stream << event.timestamp() << QQmlProfilerDefinitions::Message::Event
           << type.detailType() << event.number<quint32>(0) << event.number<quint32>(1)
           << event.number<quint32>(2);
    sendMessage(stream.data());
}

QT_END_NAMESPACE
