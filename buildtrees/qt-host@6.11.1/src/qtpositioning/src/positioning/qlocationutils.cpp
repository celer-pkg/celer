// Copyright (C) 2016 Jolla Ltd.
// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qlocationutils_p.h"
#include "qgeopositioninfo.h"
#include "qgeosatelliteinfo.h"

#include <QTime>
#include <QList>
#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QTimeZone>

#include <math.h>

QT_BEGIN_NAMESPACE

// converts e.g. 15306.0235 from NMEA sentence to 153.100392
static double qlocationutils_nmeaDegreesToDecimal(double nmeaDegrees)
{
    double deg;
    double min = 100.0 * modf(nmeaDegrees / 100.0, &deg);
    return deg + (min / 60.0);
}

static void qlocationutils_readGga(QByteArrayView bv, QGeoPositionInfo *info, double uere,
                                   bool *hasFix)
{
    const QList<QByteArray> parts = QByteArray::fromRawData(bv.data(), bv.size()).split(',');
    QGeoCoordinate coord;

    if (hasFix && parts.size() > 6 && !parts[6].isEmpty())
        *hasFix = parts[6].toInt() > 0;

    if (parts.size() > 1 && !parts[1].isEmpty()) {
        QTime time;
        if (QLocationUtils::getNmeaTime(parts[1], &time))
            info->setTimestamp(QDateTime(QDate(), time, QTimeZone::UTC));
    }

    if (parts.size() > 5 && parts[3].size() == 1 && parts[5].size() == 1) {
        double lat;
        double lng;
        if (QLocationUtils::getNmeaLatLong(parts[2], parts[3][0], parts[4], parts[5][0], &lat, &lng)) {
            coord.setLatitude(lat);
            coord.setLongitude(lng);
        }
    }

    if (parts.size() > 8 && !parts[8].isEmpty()) {
        bool hasHdop = false;
        double hdop = parts[8].toDouble(&hasHdop);
        if (hasHdop)
            info->setAttribute(QGeoPositionInfo::HorizontalAccuracy, 2 * hdop * uere);
    }

    if (parts.size() > 9 && !parts[9].isEmpty()) {
        bool hasAlt = false;
        double alt = parts[9].toDouble(&hasAlt);
        if (hasAlt)
            coord.setAltitude(alt);
    }

    if (coord.type() != QGeoCoordinate::InvalidCoordinate)
        info->setCoordinate(coord);
}

static void qlocationutils_readGsa(QByteArrayView bv, QGeoPositionInfo *info, double uere,
                                   bool *hasFix)
{
    const QList<QByteArray> parts = QByteArray::fromRawData(bv.data(), bv.size()).split(',');

    if (hasFix && parts.size() > 2 && !parts[2].isEmpty())
        *hasFix = parts[2].toInt() > 0;

    if (parts.size() > 16 && !parts[16].isEmpty()) {
        bool hasHdop = false;
        double hdop = parts[16].toDouble(&hasHdop);
        if (hasHdop)
            info->setAttribute(QGeoPositionInfo::HorizontalAccuracy, 2 * hdop * uere);
    }

    if (parts.size() > 17 && !parts[17].isEmpty()) {
        bool hasVdop = false;
        double vdop = parts[17].toDouble(&hasVdop);
        if (hasVdop)
            info->setAttribute(QGeoPositionInfo::VerticalAccuracy, 2 * vdop * uere);
    }
}

static void qlocationutils_readGsa(QByteArrayView bv, QList<int> &pnrsInUse)
{
    const QList<QByteArray> parts = QByteArray::fromRawData(bv.data(), bv.size()).split(',');
    pnrsInUse.clear();
    if (parts.size() <= 2)
        return;
    bool ok;
    for (qsizetype i = 3; i < qMin(15, parts.size()); ++i) {
        const QByteArray &pnrString = parts.at(i);
        if (pnrString.isEmpty())
            continue;
        int pnr = pnrString.toInt(&ok);
        if (ok)
            pnrsInUse.append(pnr);
    }
}

static void qlocationutils_readGll(QByteArrayView bv, QGeoPositionInfo *info, bool *hasFix)
{
    const QList<QByteArray> parts = QByteArray::fromRawData(bv.data(), bv.size()).split(',');
    QGeoCoordinate coord;

    if (hasFix && parts.size() > 6 && !parts[6].isEmpty())
        *hasFix = (parts[6][0] == 'A');

    if (parts.size() > 5 && !parts[5].isEmpty()) {
        QTime time;
        if (QLocationUtils::getNmeaTime(parts[5], &time))
            info->setTimestamp(QDateTime(QDate(), time, QTimeZone::UTC));
    }

    if (parts.size() > 4 && parts[2].size() == 1 && parts[4].size() == 1) {
        double lat;
        double lng;
        if (QLocationUtils::getNmeaLatLong(parts[1], parts[2][0], parts[3], parts[4][0], &lat, &lng)) {
            coord.setLatitude(lat);
            coord.setLongitude(lng);
        }
    }

    if (coord.type() != QGeoCoordinate::InvalidCoordinate)
        info->setCoordinate(coord);
}

static void qlocationutils_readRmc(QByteArrayView bv, QGeoPositionInfo *info, bool *hasFix)
{
    const QList<QByteArray> parts = QByteArray::fromRawData(bv.data(), bv.size()).split(',');
    QGeoCoordinate coord;
    QDate date;
    QTime time;

    if (hasFix && parts.size() > 2 && !parts[2].isEmpty())
        *hasFix = (parts[2][0] == 'A');

    if (parts.size() > 9 && parts[9].size() == 6) {
        date = QDate::fromString(QString::fromLatin1(parts[9]), QStringLiteral("ddMMyy"));
        if (date.isValid())
            date = date.addYears(100);     // otherwise starts from 1900
    }

    if (parts.size() > 1 && !parts[1].isEmpty())
        QLocationUtils::getNmeaTime(parts[1], &time);

    if (parts.size() > 6 && parts[4].size() == 1 && parts[6].size() == 1) {
        double lat;
        double lng;
        if (QLocationUtils::getNmeaLatLong(parts[3], parts[4][0], parts[5], parts[6][0], &lat, &lng)) {
            coord.setLatitude(lat);
            coord.setLongitude(lng);
        }
    }

    bool parsed = false;
    double value = 0.0;
    if (parts.size() > 7 && !parts[7].isEmpty()) {
        value = parts[7].toDouble(&parsed);
        if (parsed)
            info->setAttribute(QGeoPositionInfo::GroundSpeed, qreal(value * 1.852 / 3.6));    // knots -> m/s
    }
    if (parts.size() > 8 && !parts[8].isEmpty()) {
        value = parts[8].toDouble(&parsed);
        if (parsed)
            info->setAttribute(QGeoPositionInfo::Direction, qreal(value));
    }
    if (parts.size() > 11 && parts[11].size() == 1
            && (parts[11][0] == 'E' || parts[11][0] == 'W')) {
        value = parts[10].toDouble(&parsed);
        if (parsed) {
            if (parts[11][0] == 'W')
                value *= -1;
            info->setAttribute(QGeoPositionInfo::MagneticVariation, qreal(value));
        }
    }

    if (coord.type() != QGeoCoordinate::InvalidCoordinate)
        info->setCoordinate(coord);

    info->setTimestamp(QDateTime(date, time, QTimeZone::UTC));
}

static void qlocationutils_readVtg(QByteArrayView bv, QGeoPositionInfo *info, bool *hasFix)
{
    if (hasFix)
        *hasFix = false;

    const QList<QByteArray> parts = QByteArray::fromRawData(bv.data(), bv.size()).split(',');

    bool parsed = false;
    double value = 0.0;
    if (parts.size() > 1 && !parts[1].isEmpty()) {
        value = parts[1].toDouble(&parsed);
        if (parsed)
            info->setAttribute(QGeoPositionInfo::Direction, qreal(value));
    }
    if (parts.size() > 7 && !parts[7].isEmpty()) {
        value = parts[7].toDouble(&parsed);
        if (parsed)
            info->setAttribute(QGeoPositionInfo::GroundSpeed, qreal(value / 3.6));    // km/h -> m/s
    }
}

static void qlocationutils_readZda(QByteArrayView bv, QGeoPositionInfo *info, bool *hasFix)
{
    if (hasFix)
        *hasFix = false;

    const QList<QByteArray> parts = QByteArray::fromRawData(bv.data(), bv.size()).split(',');
    QDate date;
    QTime time;

    if (parts.size() > 1 && !parts[1].isEmpty())
        QLocationUtils::getNmeaTime(parts[1], &time);

    if (parts.size() > 4 && !parts[2].isEmpty() && !parts[3].isEmpty()
            && parts[4].size() == 4) {     // must be full 4-digit year
        int day = parts[2].toInt();
        int month = parts[3].toInt();
        int year = parts[4].toInt();
        if (day > 0 && month > 0 && year > 0)
            date.setDate(year, month, day);
    }

    info->setTimestamp(QDateTime(date, time, QTimeZone::UTC));
}

QLocationUtils::NmeaSentence QLocationUtils::getNmeaSentenceType(QByteArrayView bv)
{
    if (bv.size() < 6 || bv[0] != '$' || !hasValidNmeaChecksum(bv))
        return NmeaSentenceInvalid;

    QByteArrayView key = bv.sliced(3);

    if (key.startsWith("GGA"))
        return NmeaSentenceGGA;

    if (key.startsWith("GSA"))
        return NmeaSentenceGSA;

    if (key.startsWith("GSV"))
        return NmeaSentenceGSV;

    if (key.startsWith("GLL"))
        return NmeaSentenceGLL;

    if (key.startsWith("RMC"))
        return NmeaSentenceRMC;

    if (key.startsWith("VTG"))
        return NmeaSentenceVTG;

    if (key.startsWith("ZDA"))
        return NmeaSentenceZDA;

    return NmeaSentenceInvalid;
}

QGeoSatelliteInfo::SatelliteSystem QLocationUtils::getSatelliteSystem(QByteArrayView bv)
{
    if (bv.size() < 6 || bv[0] != '$' || !hasValidNmeaChecksum(bv))
        return QGeoSatelliteInfo::Undefined;

    QByteArrayView key = bv.sliced(1);

    // GPS: GP
    if (key.startsWith("GP"))
        return QGeoSatelliteInfo::GPS;

    // GLONASS: GL
    if (key.startsWith("GL"))
        return QGeoSatelliteInfo::GLONASS;

    // GALILEO: GA
    if (key.startsWith("GA"))
        return QGeoSatelliteInfo::GALILEO;

    // BeiDou: BD or GB
    if (key.startsWith("BD") || key.startsWith("GB"))
        return QGeoSatelliteInfo::BEIDOU;

    // QZSS: GQ, PQ, QZ
    if (key.startsWith("GQ") || key.startsWith("PQ") || key.startsWith("QZ"))
        return QGeoSatelliteInfo::QZSS;

    // Multiple: GN
    if (key.startsWith("GN"))
        return QGeoSatelliteInfo::Multiple;

    return QGeoSatelliteInfo::Undefined;
}

QGeoSatelliteInfo::SatelliteSystem QLocationUtils::getSatelliteSystemBySatelliteId(int satId)
{
    if (satId >= 1 && satId <= 32)
        return QGeoSatelliteInfo::GPS;

    if (satId >= 65 && satId <= 96) // including future extensions
        return QGeoSatelliteInfo::GLONASS;

    if (satId >= 193 && satId <= 200) // including future extensions
        return QGeoSatelliteInfo::QZSS;

    if ((satId >= 201 && satId <= 235) || (satId >= 401 && satId <= 437))
        return QGeoSatelliteInfo::BEIDOU;

    if (satId >= 301 && satId <= 336)
        return QGeoSatelliteInfo::GALILEO;

    return QGeoSatelliteInfo::Undefined;
}

bool QLocationUtils::getPosInfoFromNmea(QByteArrayView bv, QGeoPositionInfo *info,
                                        double uere, bool *hasFix)
{
    if (!info)
        return false;

    if (hasFix)
        *hasFix = false;

    NmeaSentence nmeaType = getNmeaSentenceType(bv);
    if (nmeaType == NmeaSentenceInvalid)
        return false;

    // Adjust size so that * and following characters are not parsed by the following functions.
    qsizetype idx = bv.indexOf('*');
    QByteArrayView key = idx < 0 ? bv : bv.first(idx);

    switch (nmeaType) {
    case NmeaSentenceGGA:
        qlocationutils_readGga(key, info, uere, hasFix);
        return true;
    case NmeaSentenceGSA:
        qlocationutils_readGsa(key, info, uere, hasFix);
        return true;
    case NmeaSentenceGLL:
        qlocationutils_readGll(key, info, hasFix);
        return true;
    case NmeaSentenceRMC:
        qlocationutils_readRmc(key, info, hasFix);
        return true;
    case NmeaSentenceVTG:
        qlocationutils_readVtg(key, info, hasFix);
        return true;
    case NmeaSentenceZDA:
        qlocationutils_readZda(key, info, hasFix);
        return true;
    default:
        return false;
    }
}

QNmeaSatelliteInfoSource::SatelliteInfoParseStatus
QLocationUtils::getSatInfoFromNmea(QByteArrayView bv, QList<QGeoSatelliteInfo> &infos, QGeoSatelliteInfo::SatelliteSystem &system)
{
    if (bv.isEmpty())
        return QNmeaSatelliteInfoSource::NotParsed;

    NmeaSentence nmeaType = getNmeaSentenceType(bv);
    if (nmeaType != NmeaSentenceGSV)
        return QNmeaSatelliteInfoSource::NotParsed;

    // Standard forbids using $GN talker id for GSV messages, so the system
    // type here will be uniquely identified.
    system = getSatelliteSystem(bv);

    // Adjust size so that * and following characters are not parsed by the
    // following code.
    qsizetype idx = bv.indexOf('*');

    const QList<QByteArray> parts = QByteArray::fromRawData(bv.data(),
                                                            idx < 0 ? bv.size() : idx).split(',');

    if (parts.size() <= 3) {
        infos.clear();
        return QNmeaSatelliteInfoSource::FullyParsed; // Malformed sentence.
    }
    bool ok;
    const int totalSentences = parts.at(1).toInt(&ok);
    if (!ok) {
        infos.clear();
        return QNmeaSatelliteInfoSource::FullyParsed; // Malformed sentence.
    }

    const int sentence = parts.at(2).toInt(&ok);
    if (!ok) {
        infos.clear();
        return QNmeaSatelliteInfoSource::FullyParsed; // Malformed sentence.
    }

    const int totalSats = parts.at(3).toInt(&ok);
    if (!ok) {
        infos.clear();
        return QNmeaSatelliteInfoSource::FullyParsed; // Malformed sentence.
    }

    if (sentence == 1)
        infos.clear();

    const int numSatInSentence = qMin(sentence * 4, totalSats) - (sentence - 1) * 4;
    if (parts.size() < (4 + numSatInSentence * 4)) {
        infos.clear();
        return QNmeaSatelliteInfoSource::FullyParsed; // Malformed sentence.
    }

    int field = 4;
    for (int i = 0; i < numSatInSentence; ++i) {
        QGeoSatelliteInfo info;
        info.setSatelliteSystem(system);
        int prn = parts.at(field++).toInt(&ok);
        // Quote from: https://gpsd.gitlab.io/gpsd/NMEA.html#_satellite_ids
        // GLONASS satellite numbers come in two flavors. If a sentence has a GL
        // talker ID, expect the skyviews to be GLONASS-only and in the range
        // 1-32; you must add 64 to get a globally-unique NMEA ID. If the
        // sentence has a GN talker ID, the device emits a multi-constellation
        // skyview with GLONASS IDs already in the 65-96 range.
        //
        // However I don't observe such behavior with my device. So implementing
        // a safe scenario.
        if (ok && (system == QGeoSatelliteInfo::GLONASS)) {
            if (prn <= 64)
                prn += 64;
        }
        info.setSatelliteIdentifier((ok) ? prn : 0);
        const int elevation = parts.at(field++).toInt(&ok);
        info.setAttribute(QGeoSatelliteInfo::Elevation, (ok) ? elevation : 0);
        const int azimuth = parts.at(field++).toInt(&ok);
        info.setAttribute(QGeoSatelliteInfo::Azimuth, (ok) ? azimuth : 0);
        const int snr = parts.at(field++).toInt(&ok);
        info.setSignalStrength((ok) ? snr : -1);
        infos.append(info);
    }

    if (sentence == totalSentences)
        return QNmeaSatelliteInfoSource::FullyParsed;

    return QNmeaSatelliteInfoSource::PartiallyParsed;
}

QGeoSatelliteInfo::SatelliteSystem QLocationUtils::getSatInUseFromNmea(QByteArrayView bv,
                                                                       QList<int> &pnrsInUse)
{
    if (bv.isEmpty())
        return QGeoSatelliteInfo::Undefined;

    NmeaSentence nmeaType = getNmeaSentenceType(bv);
    if (nmeaType != NmeaSentenceGSA)
        return QGeoSatelliteInfo::Undefined;

    auto systemType = getSatelliteSystem(bv);
    if (systemType == QGeoSatelliteInfo::Undefined)
        return systemType;

    // The documentation states that we do not modify pnrsInUse if we could not
    // parse the data
    pnrsInUse.clear();

    // Adjust size so that * and following characters are not parsed by the following functions.
    qsizetype idx = bv.indexOf('*');
    QByteArrayView key = idx < 0 ? bv : bv.first(idx);

    qlocationutils_readGsa(key, pnrsInUse);

    // Quote from: https://gpsd.gitlab.io/gpsd/NMEA.html#_satellite_ids
    // GLONASS satellite numbers come in two flavors. If a sentence has a GL
    // talker ID, expect the skyviews to be GLONASS-only and in the range 1-32;
    // you must add 64 to get a globally-unique NMEA ID. If the sentence has a
    // GN talker ID, the device emits a multi-constellation skyview with
    // GLONASS IDs already in the 65-96 range.
    //
    // However I don't observe such behavior with my device. So implementing a
    // safe scenario.
    if (systemType == QGeoSatelliteInfo::GLONASS) {
        std::for_each(pnrsInUse.begin(), pnrsInUse.end(), [](int &id) {
            if (id <= 64)
                id += 64;
        });
    }

    if ((systemType == QGeoSatelliteInfo::Multiple) && !pnrsInUse.isEmpty()) {
        // Standard claims that in case of multiple system types we will receive
        // several GSA messages, each containing data from only one satellite
        // system, so we can pick the first id to determine the system type.
        auto tempSystemType = getSatelliteSystemBySatelliteId(pnrsInUse.front());
        if (tempSystemType != QGeoSatelliteInfo::Undefined)
            systemType = tempSystemType;
    }

    return systemType;
}

bool QLocationUtils::hasValidNmeaChecksum(QByteArrayView bv)
{
    qsizetype asteriskIndex = bv.indexOf('*');

    constexpr qsizetype CSUM_LEN = 2;
    if (asteriskIndex < 0 || asteriskIndex >= bv.size() - CSUM_LEN)
        return false;

    // XOR byte value of all characters between '$' and '*'
    int result = 0;
    for (qsizetype i = 1; i < asteriskIndex; ++i)
        result ^= bv[i];
    /*
        char calc[CSUM_LEN + 1];
        ::snprintf(calc, CSUM_LEN + 1, "%02x", result);
        return ::strncmp(calc, &data[asteriskIndex+1], 2) == 0;
        */

    QByteArrayView checkSumBytes = bv.sliced(asteriskIndex + 1, 2);
    bool ok = false;
    int checksum = checkSumBytes.toInt(&ok,16);
    return ok && checksum == result;
}

bool QLocationUtils::getNmeaTime(const QByteArray &bytes, QTime *time)
{
    QTime tempTime = QTime::fromString(QString::fromLatin1(bytes),
                        QStringView(bytes.size() > 6 && bytes[6] == '.'
                                                       ? u"hhmmss.z"
                                                       : u"hhmmss"));

    if (tempTime.isValid()) {
        *time = tempTime;
        return true;
    }
    return false;
}

bool QLocationUtils::getNmeaLatLong(const QByteArray &latString, char latDirection, const QByteArray &lngString, char lngDirection, double *lat, double *lng)
{
    if ((latDirection != 'N' && latDirection != 'S')
            || (lngDirection != 'E' && lngDirection != 'W')) {
        return false;
    }

    bool hasLat = false;
    bool hasLong = false;
    double tempLat = latString.toDouble(&hasLat);
    double tempLng = lngString.toDouble(&hasLong);
    if (hasLat && hasLong) {
        tempLat = qlocationutils_nmeaDegreesToDecimal(tempLat);
        if (latDirection == 'S')
            tempLat *= -1;
        tempLng = qlocationutils_nmeaDegreesToDecimal(tempLng);
        if (lngDirection == 'W')
            tempLng *= -1;

        if (isValidLat(tempLat) && isValidLong(tempLng)) {
            *lat = tempLat;
            *lng = tempLng;
            return true;
        }
    }
    return false;
}

QT_END_NAMESPACE

