// Copyright (C) 2023 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtProtobufQtCoreTypes/qtprotobufqtcoretypes.h>

#include <QtProtobufQtCoreTypes/private/QtCore.qpb.h>
#include <QtProtobufQtCoreTypes/private/qtprotobufqttypescommon_p.h>

#include <QtCore/qurl.h>
#include <QtCore/qchar.h>
#include <QtCore/quuid.h>
#include <QtCore/qtimezone.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qdatastream.h>
#include <QtCore/qsize.h>
#include <QtCore/qpoint.h>
#include <QtCore/qrect.h>
#include <QtCore/qbuffer.h>
#include <QtCore/qversionnumber.h>
#include <QtCore/qendian.h>

QT_BEGIN_NAMESPACE

void QtProtobufPrivate::warnTypeConversionError()
{
    qWarning("Qt Proto Type conversion error.");
}

static std::optional<QUrl> convert(const QtProtobufPrivate::QtCore::QUrl &from)
{
    QUrl url(from.url());
    return (url.isValid() || url.isEmpty()) ? std::optional<QUrl>(url) : std::nullopt;
}

static std::optional<QtProtobufPrivate::QtCore::QUrl> convert(const QUrl &from)
{
    if (from.isValid() || from.isEmpty()) {
        QtProtobufPrivate::QtCore::QUrl url;
        url.setUrl(from.url());
        return url;
    }
    return std::nullopt;
}

static QChar convert(const QtProtobufPrivate::QtCore::QChar &from)
{
    return QChar(from.utf16CodePoint());
}

static QtProtobufPrivate::QtCore::QChar convert(const QChar &from)
{
    QtProtobufPrivate::QtCore::QChar symbol;
    symbol.setUtf16CodePoint(from.unicode());
    return symbol;
}

static std::optional<QUuid> convert(const QtProtobufPrivate::QtCore::QUuid &from)
{
    if (from.rfc4122Uuid().size() != 16)
        return std::nullopt;

    return QUuid::fromRfc4122(from.rfc4122Uuid());
}

static std::optional<QtProtobufPrivate::QtCore::QUuid> convert(const QUuid &from)
{
    if (from.toRfc4122().size() != 16)
        return std::nullopt;

    QtProtobufPrivate::QtCore::QUuid uuid;
    uuid.setRfc4122Uuid(from.toRfc4122());
    return uuid;
}

static std::optional<QTime> convert(const QtProtobufPrivate::QtCore::QTime &from)
{
    QTime time = QTime::fromMSecsSinceStartOfDay(from.millisecondsSinceMidnight());
    return time.isValid() ? std::optional<QTime>(time) : std::nullopt;
}

static std::optional<QtProtobufPrivate::QtCore::QTime> convert(const QTime &from)
{
    if (!from.isValid() || from.isNull())
        return std::nullopt;

    QtProtobufPrivate::QtCore::QTime time;
    time.setMillisecondsSinceMidnight(from.msecsSinceStartOfDay());
    return time;
}

static std::optional<QDate> convert(const QtProtobufPrivate::QtCore::QDate &from)
{
    QDate date = QDate::fromJulianDay(from.julianDay());
    return date.isValid() ? std::optional<QDate>(date) : std::nullopt;
}

static std::optional<QtProtobufPrivate::QtCore::QDate> convert(const QDate &from)
{
    if (!from.isValid() || from.isNull())
        return std::nullopt;

    QtProtobufPrivate::QtCore::QDate date;
    date.setJulianDay(from.toJulianDay());
    return date;
}

static QTimeZone::Initialization getTimeZoneInitialization(
        QtProtobufPrivate::QtCore::QTimeZone::TimeSpec protoSpec)
{
    return protoSpec == QtProtobufPrivate::QtCore::QTimeZone::TimeSpec::LocalTime
            ? QTimeZone::LocalTime : QTimeZone::UTC;
}

static std::optional<QTimeZone> convert(
        const QtProtobufPrivate::QtCore::QTimeZone &from)
{
    QTimeZone result;
    switch (from.timeZoneField())
    {
    case QtProtobufPrivate::QtCore::QTimeZone::TimeZoneFields::IanaId:
#if QT_CONFIG(timezone)
        result = QTimeZone(from.ianaId());
        break;
#else
        qWarning() << "No timezone support. IanaId cannot be used for conversion.";
        return std::nullopt;
#endif // timezone
    case QtProtobufPrivate::QtCore::QTimeZone::TimeZoneFields::TimeSpec:
        result = QTimeZone(getTimeZoneInitialization(from.timeSpec()));
        break;
    case QtProtobufPrivate::QtCore::QTimeZone::TimeZoneFields::OffsetSeconds:
        result = QTimeZone::fromSecondsAheadOfUtc(from.offsetSeconds());
        break;
    case QtProtobufPrivate::QtCore::QTimeZone::TimeZoneFields::UninitializedField:
        return std::nullopt;
    }
    return result.isValid() ?  std::optional<QTimeZone>(result) : std::nullopt;
}

static std::optional<QtProtobufPrivate::QtCore::QTimeZone> convert(const QTimeZone &from)
{
    if (!from.isValid())
        return std::nullopt;

    QtProtobufPrivate::QtCore::QTimeZone result;
    switch (from.timeSpec()) {
    case Qt::TimeZone:
#if QT_CONFIG(timezone)
        result.setIanaId(from.id());
#else
        qInfo() << "Result will be treated like UTC.";
        result.setTimeSpec(QtProtobufPrivate::QtCore::QTimeZone::TimeSpec::UTC);
#endif // QT_CONFIG(timezone)
        break;
    case Qt::LocalTime:
        result.setTimeSpec(QtProtobufPrivate::QtCore::QTimeZone::TimeSpec::LocalTime);
        break;
    case Qt::UTC:
        result.setTimeSpec(QtProtobufPrivate::QtCore::QTimeZone::TimeSpec::UTC);
        break;
    case Qt::OffsetFromUTC:
        result.setOffsetSeconds(from.fixedSecondsAheadOfUtc());
        break;
    }

    return result;
}

static std::optional<QDateTime> convert(const QtProtobufPrivate::QtCore::QDateTime &from)
{
    QDateTime dateTime;
    std::optional<QTimeZone> zone = convert(from.timeZone());
    if (zone)
        dateTime = QDateTime::fromMSecsSinceEpoch(from.utcMsecsSinceUnixEpoch(), zone.value());
    else
        dateTime = QDateTime::fromMSecsSinceEpoch(from.utcMsecsSinceUnixEpoch());

    return dateTime.isValid() ? std::optional<QDateTime>(dateTime) : std::nullopt;
}

static std::optional<QtProtobufPrivate::QtCore::QDateTime> convert(const QDateTime &from)
{
    if (!from.isValid() || from.isNull())
        return std::nullopt;

    QtProtobufPrivate::QtCore::QDateTime datetime;
    datetime.setUtcMsecsSinceUnixEpoch(from.toMSecsSinceEpoch());

    std::optional<QtProtobufPrivate::QtCore::QTimeZone> tZone
            = convert(from.timeRepresentation());

    if (tZone) {
        datetime.setTimeZone(tZone.value());
        return datetime;
    }

    return std::nullopt;
}

static QSize convert(const QtProtobufPrivate::QtCore::QSize &from)
{
    return QSize(from.width(), from.height());
}

static std::optional<QtProtobufPrivate::QtCore::QSize> convert(const QSize &from)
{
    if (from.isNull() || from.isEmpty() || !from.isValid())
        return std::nullopt;

    QtProtobufPrivate::QtCore::QSize size;
    size.setWidth(from.width());
    size.setHeight(from.height());
    return size;
}

static QSizeF convert(const QtProtobufPrivate::QtCore::QSizeF &from)
{
    return QSizeF(from.width(), from.height());
}

static std::optional<QtProtobufPrivate::QtCore::QSizeF> convert(const QSizeF &from)
{
    if (from.isNull() || from.isEmpty() || !from.isValid())
        return std::nullopt;

    QtProtobufPrivate::QtCore::QSizeF sizeF;
    sizeF.setWidth(from.width());
    sizeF.setHeight(from.height());
    return sizeF;
}

static QPoint convert(const QtProtobufPrivate::QtCore::QPoint &from)
{
    return QPoint(from.x(), from.y());
}

static std::optional<QtProtobufPrivate::QtCore::QPoint> convert(const QPoint &from)
{
    if (from.isNull())
        return std::nullopt;

    QtProtobufPrivate::QtCore::QPoint pointT;
    pointT.setX(from.x());
    pointT.setY(from.y());
    return pointT;
}

static QPointF convert(const QtProtobufPrivate::QtCore::QPointF &from)
{
    return QPointF(from.x(), from.y());
}

static std::optional<QtProtobufPrivate::QtCore::QPointF> convert(const QPointF &from)
{
    if (from.isNull())
        return std::nullopt;

    QtProtobufPrivate::QtCore::QPointF pointF;
    pointF.setX(from.x());
    pointF.setY(from.y());
    return pointF;
}

static QRect convert(const QtProtobufPrivate::QtCore::QRect &from)
{
    return QRect(QPoint(from.x(), from.y()),
                 QSize(from.width(), from.height()));
}

static std::optional<QtProtobufPrivate::QtCore::QRect> convert(const QRect &from)
{
    if (from.isNull() || from.isEmpty() || !from.isValid())
        return std::nullopt;

    QtProtobufPrivate::QtCore::QRect rect;
    rect.setX(from.x());
    rect.setY(from.y());
    rect.setWidth(from.width());
    rect.setHeight(from.height());
    return rect;
}

static QRectF convert(const QtProtobufPrivate::QtCore::QRectF &from)
{
    return QRectF(QPointF(from.x(), from.y()),
                  QSizeF(from.width(), from.height()));
}

static std::optional<QtProtobufPrivate::QtCore::QRectF> convert(const QRectF &from)
{
    if (from.isNull() || from.isEmpty() || !from.isValid())
        return std::nullopt;

    QtProtobufPrivate::QtCore::QRectF rectF;
    rectF.setX(from.x());
    rectF.setY(from.y());
    rectF.setWidth(from.width());
    rectF.setHeight(from.height());
    return rectF;
}

static std::optional<QVersionNumber> convert(const QtProtobufPrivate::QtCore::QVersionNumber &from)
{
    if (from.segments().size() == 0)
        return std::nullopt;

    QList<int> versionList;
    for (const auto &segment : from.segments())
        versionList.append(segment);
    return QVersionNumber(versionList);
}

static std::optional<QtProtobufPrivate::QtCore::QVersionNumber> convert(const QVersionNumber &from)
{
    if (from.segments().size() == 0)
        return std::nullopt;

    QtProtobufPrivate::QtCore::QVersionNumber version;
    QtProtobuf::int32List newSegments;
    const auto segments = from.segments();
    for (const auto &segment : segments)
        newSegments.append(segment);
    version.setSegments(std::move(newSegments));
    return version;
}

namespace QtProtobuf {

/*!
    Registers serializers for the Qt::ProtobufQtCoreTypes library.
*/
void registerProtobufQtCoreTypes() {
    QtProtobufPrivate::registerQtTypeHandler<QUrl, QtProtobufPrivate::QtCore::QUrl>();
    QtProtobufPrivate::registerQtTypeHandler<QChar, QtProtobufPrivate::QtCore::QChar>();
    QtProtobufPrivate::registerQtTypeHandler<QUuid, QtProtobufPrivate::QtCore::QUuid>();
    QtProtobufPrivate::registerQtTypeHandler<QTime, QtProtobufPrivate::QtCore::QTime>();
    QtProtobufPrivate::registerQtTypeHandler<QDate, QtProtobufPrivate::QtCore::QDate>();
    QtProtobufPrivate::registerQtTypeHandler<QTimeZone, QtProtobufPrivate::QtCore::QTimeZone>();
    QtProtobufPrivate::registerQtTypeHandler<QDateTime, QtProtobufPrivate::QtCore::QDateTime>();
    QtProtobufPrivate::registerQtTypeHandler<QSize, QtProtobufPrivate::QtCore::QSize>();
    QtProtobufPrivate::registerQtTypeHandler<QSizeF, QtProtobufPrivate::QtCore::QSizeF>();
    QtProtobufPrivate::registerQtTypeHandler<QPoint, QtProtobufPrivate::QtCore::QPoint>();
    QtProtobufPrivate::registerQtTypeHandler<QPointF, QtProtobufPrivate::QtCore::QPointF>();
    QtProtobufPrivate::registerQtTypeHandler<QRect, QtProtobufPrivate::QtCore::QRect>();
    QtProtobufPrivate::registerQtTypeHandler<QRectF, QtProtobufPrivate::QtCore::QRectF>();
    QtProtobufPrivate::registerQtTypeHandler<QVersionNumber,
            QtProtobufPrivate::QtCore::QVersionNumber>();
}
}

QT_END_NAMESPACE
