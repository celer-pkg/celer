// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANDBCFILEPARSER_H
#define QCANDBCFILEPARSER_H

#include <QtCore/QList>

#include <QtSerialBus/qcancommondefinitions.h>
#include <QtSerialBus/qtserialbusglobal.h>

#include <memory>

QT_BEGIN_NAMESPACE

class QCanDbcFileParserPrivate;
class QCanMessageDescription;
class QCanUniqueIdDescription;

class QCanDbcFileParser
{
public:
    enum class Error : quint8 {
        None = 0,
        FileReading,
        Parsing
    };

    // The DBC protocol uses unsigned_integer to describe the supported values.
    // Do we need to use QVariant instead of quint32? Or qint64 for better BC
    // guarantees?
    using ValueDescriptions = QHash<quint32, QString>;
    using SignalValueDescriptions = QHash<QString, ValueDescriptions>;
    using MessageValueDescriptions = QHash<QtCanBus::UniqueId, SignalValueDescriptions>;

    Q_SERIALBUS_EXPORT QCanDbcFileParser();
    Q_SERIALBUS_EXPORT ~QCanDbcFileParser();

    Q_SERIALBUS_EXPORT bool parse(const QString &fileName);
    Q_SERIALBUS_EXPORT bool parse(const QStringList &fileNames);
    Q_SERIALBUS_EXPORT bool parseData(QStringView data);

    Q_SERIALBUS_EXPORT QList<QCanMessageDescription> messageDescriptions() const;
    Q_SERIALBUS_EXPORT MessageValueDescriptions messageValueDescriptions() const;

    Q_SERIALBUS_EXPORT Error error() const;
    Q_SERIALBUS_EXPORT QString errorString() const;
    Q_SERIALBUS_EXPORT QStringList warnings() const;

    Q_SERIALBUS_EXPORT static QCanUniqueIdDescription uniqueIdDescription();

private:
    std::unique_ptr<QCanDbcFileParserPrivate> d;
    friend class QCanDbcFileParserPrivate;

    Q_DISABLE_COPY_MOVE(QCanDbcFileParser)
};

QT_END_NAMESPACE

#endif // QCANDBCFILEPARSER_H
