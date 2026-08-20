// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlocationtestutils_p.h"

bool QLocationTestUtils::hasDefaultSource()
{
    return false;
}

bool QLocationTestUtils::hasDefaultMonitor()
{
    return false;
}

QString QLocationTestUtils::addNmeaChecksumAndBreaks(const QString &sentence)
{
    Q_ASSERT(sentence[0] == '$' && sentence[sentence.size()-1] == '*');

    // XOR byte value of all characters between '$' and '*'
    int result = 0;
    for (int i=1; i<sentence.size()-1; i++)
        result ^= sentence[i].toLatin1();
    const QString sum = QString::asprintf("%02x", result);
    return sentence + sum + "\r\n";
}

QString QLocationTestUtils::createRmcSentence(const QDateTime &dt)
{
    QString time = dt.toString("hhmmss.zzz");
    QString date = dt.toString("ddMMyy");
    QString nmea = QString("$GPRMC,%1,A,2730.83609,S,15301.87844,E,0.7,9.0,%2,11.2,W,A*")
        .arg(time).arg(date);
    return addNmeaChecksumAndBreaks(nmea);
}

QString QLocationTestUtils::createGgaSentence(const QTime &time)
{
    QString nmea = QString("$GPGGA,%1,2734.76859,S,15305.99361,E,1,04,3.5,49.4,M,39.2,M,,*")
            .arg(time.toString("hhmmss.zzz"));
    return addNmeaChecksumAndBreaks(nmea);
}

QString QLocationTestUtils::createGgaSentence(int lat, int lng, const QTime &time) {
    QString nmea = QString("$GPGGA,%1,%200.00000,S,%300.,E,1,04,3.5,49.4,M,39.2,M,,*")
            .arg(time.toString("hhmmss.zzz")).arg(lat).arg(lng);
    return addNmeaChecksumAndBreaks(nmea);
}

QString QLocationTestUtils::createZdaSentence(const QDateTime &dt)
{
    QString time = dt.toString("hhmmss.zzz");
    QString nmea = QString("$GPZDA,%1,%2,%3,%4,,*")
        .arg(time).arg(dt.toString("dd")).arg(dt.toString("MM")).arg(dt.toString("yyyy"));
    return addNmeaChecksumAndBreaks(nmea);
}

QString QLocationTestUtils::createGsaSentence()
{
    return addNmeaChecksumAndBreaks(QStringLiteral("$GPGSA,A,3,,,,,,,,,,,,,3.0,3.5,4.0*"));
}

QString QLocationTestUtils::createGsvSentence()
{
    return addNmeaChecksumAndBreaks(QStringLiteral("$GPGSV,1,1,0,,,,,,,,,,,,,,,,*"));
}

QString QLocationTestUtils::createGsaLongSentence()
{
    return addNmeaChecksumAndBreaks(QStringLiteral("$GPGSA,A,3,16,25,,,,,,,,,,,2.3,1.3,1.9*"));
}

QString QLocationTestUtils::createGsvLongSentence()
{
    return addNmeaChecksumAndBreaks(
            QStringLiteral("$GPGSV,1,1,4,16,49,115,42,25,39,269,36,23,58,176,29,20,72,335,35*"));
}

QString QLocationTestUtils::createGsaVariableSentence(quint8 satId)
{
    const QString nmea = QString("$GPGSA,A,3,%1,,,,,,,,,,,,2.3,1.3,1.9*").arg(static_cast<int>(satId));
    return addNmeaChecksumAndBreaks(nmea);
}

QString QLocationTestUtils::createGsvVariableSentence(quint8 satId)
{
    const QString nmea = QString("$GPGSV,1,1,1,%1,49,115,42,,,,,,,,,,,,*").arg(static_cast<int>(satId));
    return addNmeaChecksumAndBreaks(nmea);
}
