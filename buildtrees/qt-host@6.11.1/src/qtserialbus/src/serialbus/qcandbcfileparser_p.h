// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANDBCFILEPARSER_P_H
#define QCANDBCFILEPARSER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qcandbcfileparser.h"
#include "qcanmessagedescription.h"

#include <QtCore/QHash>

QT_BEGIN_NAMESPACE

class QCanSignalDescription;

class QCanDbcFileParserPrivate
{
public:
    void reset();
    bool parseFile(const QString &fileName);
    bool parseData(QStringView data);
    bool processLine(const QStringView line);
    bool parseMessage(const QStringView data);
    QCanMessageDescription extractMessage(const QRegularExpressionMatch &match);
    bool parseSignal(const QStringView data);
    QCanSignalDescription extractSignal(const QRegularExpressionMatch &match);
    void parseSignalType(const QStringView data);
    void parseComment(const QStringView data);
    void parseExtendedMux(const QStringView data);
    void parseValueDescriptions(const QStringView data);
    void postProcessSignalMultiplexing();

    void addWarning(QString &&warning);
    void addCurrentMessage();

    QList<QCanMessageDescription> getMessages() const;

    QString m_fileName;
    QCanDbcFileParser::Error m_error = QCanDbcFileParser::Error::None;
    QString m_errorString;
    QStringList m_warnings;
    qsizetype m_lineOffset = 0;
    bool m_isProcessingMessage = false;
    bool m_seenExtraData = false;
    QCanMessageDescription m_currentMessage;
    QHash<QtCanBus::UniqueId, QCanMessageDescription> m_messageDescriptions;
    QCanDbcFileParser::MessageValueDescriptions m_valueDescriptions;
};

QT_END_NAMESPACE

#endif // QCANDBCFILEPARSER_P_H
