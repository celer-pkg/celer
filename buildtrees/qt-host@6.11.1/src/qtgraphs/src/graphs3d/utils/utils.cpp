// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtCore/qregularexpression.h>
#include <QtGui/qbrush.h>

#include "private/qquickrectangle_p.h"
#include "utils_p.h"

QT_BEGIN_NAMESPACE

Utils::ParamType Utils::preParseFormat(
    const QString &format, QString &preStr, QString &postStr, int &precision, char &formatSpec)
{
    static QRegularExpression formatMatcher(
        QStringLiteral("^([^%]*)%([\\-\\+#\\s\\d\\.lhjztL]*)([dicuoxfegXFEG])(.*)$"));
    static QRegularExpression precisionMatcher(QStringLiteral("\\.(\\d+)"));

    Utils::ParamType retVal;

    QRegularExpressionMatch formatMatch = formatMatcher.match(format, 0);

    if (formatMatch.hasMatch()) {
        preStr = formatMatch.captured(1);
        // Six and 'g' are defaults in Qt API
        precision = 6;
        if (!formatMatch.captured(2).isEmpty()) {
            QRegularExpressionMatch precisionMatch = precisionMatcher.match(formatMatch.captured(2),
                                                                            0);
            if (precisionMatch.hasMatch())
                precision = precisionMatch.captured(1).toInt();
        }
        if (formatMatch.captured(3).isEmpty())
            formatSpec = 'g';
        else
            formatSpec = formatMatch.captured(3).at(0).toLatin1();
        postStr = formatMatch.captured(4);
        retVal = mapFormatCharToParamType(formatSpec);
    } else {
        retVal = ParamType::Unknown;
        // The out parameters are irrelevant in unknown case
    }

    return retVal;
}

Utils::ParamType Utils::mapFormatCharToParamType(char formatSpec)
{
    ParamType retVal = ParamType::Unknown;
    if (formatSpec == 'd' || formatSpec == 'i' || formatSpec == 'c') {
        retVal = ParamType::Int;
    } else if (formatSpec == 'u' || formatSpec == 'o' || formatSpec == 'x' || formatSpec == 'X') {
        retVal = ParamType::UInt;
    } else if (formatSpec == 'f' || formatSpec == 'F' || formatSpec == 'e' || formatSpec == 'E'
               || formatSpec == 'g' || formatSpec == 'G') {
        retVal = ParamType::Real;
    }

    return retVal;
}

QString Utils::formatLabelSprintf(const QByteArray &format, Utils::ParamType paramType, qreal value)
{
    switch (paramType) {
    case ParamType::Int:
        return QString::asprintf(format.constData(), qint64(value));
    case ParamType::UInt:
        return QString::asprintf(format.constData(), quint64(value));
    case ParamType::Real:
        return QString::asprintf(format.constData(), value);
    default:
        // Return format string to detect errors. Bars selection label logic also
        // depends on this.
        return QString::fromUtf8(format);
    }
}

QString Utils::formatLabelLocalized(Utils::ParamType paramType,
                                    qreal value,
                                    const QLocale &locale,
                                    const QString &preStr,
                                    const QString &postStr,
                                    int precision,
                                    char formatSpec,
                                    const QByteArray &format)
{
    switch (paramType) {
    case ParamType::Int:
    case ParamType::UInt:
        return preStr + locale.toString(qint64(value)) + postStr;
    case ParamType::Real:
        return preStr + locale.toString(value, formatSpec, precision) + postStr;
    default:
        // Return format string to detect errors. Bars selection label logic also
        // depends on this.
        return QString::fromUtf8(format);
    }
}

QString Utils::defaultLabelFormat()
{
    static const QString defaultFormat(QStringLiteral("%.2f"));
    return defaultFormat;
}

float Utils::wrapValue(float value, float min, float max)
{
    if (value > max) {
        value = min + (value - max);

        // In case single wrap fails, jump to opposite end.
        if (value > max)
            value = min;
    }

    if (value < min) {
        value = max + (value - min);

        // In case single wrap fails, jump to opposite end.
        if (value < min)
            value = max;
    }

    return value;
}

QQuaternion Utils::calculateRotation(QVector3D xyzRotations)
{
    QQuaternion rotQuatX = QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, xyzRotations.x());
    QQuaternion rotQuatY = QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, xyzRotations.y());
    QQuaternion rotQuatZ = QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, xyzRotations.z());
    QQuaternion totalRotation = rotQuatY * rotQuatZ * rotQuatX;
    return totalRotation;
}

void Utils::verifyGradientCompleteness(QLinearGradient &gradient)
{
    // Fix the start and end stops of the gradient, to make sure it's complete
    // (0...1)
    auto stops = gradient.stops();
    if (stops.first().first != 0.) {
        const QColor firstColor = stops.first().second;
        gradient.setColorAt(0., firstColor);
    }
    if (stops.last().first != 1.) {
        const QColor lastColor = stops.last().second;
        gradient.setColorAt(1., lastColor);
    }
}

void Utils::setSeriesGradient(QAbstract3DSeries *series, QJSValue gradient, GradientType type)
{
    auto newGradient = qobject_cast<QQuickGradient *>(gradient.toQObject());
    QLinearGradient linearGradient;
    linearGradient.setStops(newGradient->gradientStops());

    switch (type) {
    case GradientType::Base:
        series->setBaseGradient(linearGradient);
        break;
    case GradientType::Single:
        series->setSingleHighlightGradient(linearGradient);
        break;
    case GradientType::Multi:
        series->setMultiHighlightGradient(linearGradient);
        break;
    default: // Never goes here
        break;
    }
}

void Utils::setSeriesGradient(QAbstract3DSeries *series, QQuickGradient *gradient, GradientType type)
{
    QLinearGradient lg;
    lg.setStops(gradient->gradientStops());
    switch (type) {
    case GradientType::Base:
        series->setBaseGradient(lg);
        break;
    case GradientType::Single:
        series->setSingleHighlightGradient(lg);
        break;
    case GradientType::Multi:
        series->setMultiHighlightGradient(lg);
        break;
    default: // Never goes here
        break;
    }
}

void Utils::connectSeriesGradient(QAbstract3DSeries *series,
                                  QJSValue newGradient,
                                  GradientType type,
                                  QJSValue &memberGradient)
{
    // connect new / disconnect old
    if (newGradient.isQObject() && !newGradient.equals(memberGradient)) {
        auto quickGradient = qobject_cast<QQuickGradient *>(memberGradient.toQObject());
        if (quickGradient)
            QObject::disconnect(quickGradient, 0, series, 0);

        memberGradient = newGradient;
        quickGradient = qobject_cast<QQuickGradient *>(memberGradient.toQObject());

        const int updatedIndex = QMetaMethod::fromSignal(&QQuickGradient::updated).methodIndex();

        int handleIndex = -1;
        switch (type) {
        case GradientType::Base:
            handleIndex = series->metaObject()->indexOfSlot("handleBaseGradientUpdate()");
            break;
        case GradientType::Single:
            handleIndex = series->metaObject()->indexOfSlot(
                "handleSingleHighlightGradientUpdate()");
            break;
        case GradientType::Multi:
            handleIndex = series->metaObject()->indexOfSlot("handleMultiHighlightGradientUpdate()");
            break;
        default: // Never goes here
            break;
        }

        if (quickGradient)
            QMetaObject::connect(quickGradient, updatedIndex, series, handleIndex);
    }

    if (!memberGradient.isNull())
        setSeriesGradient(series, memberGradient, type);
}

bool Utils::imageHasTransparency(const QImage &image)
{
    if (image.hasAlphaChannel()) {
        const uchar *pixels = image.bits();
        int bytes = image.sizeInBytes();

        for (const QRgb *pixel = reinterpret_cast<const QRgb *>(pixels); bytes > 0;
             pixel++, bytes -= sizeof(QRgb)) {
            if (qAlpha(*pixel) != UCHAR_MAX)
                return true;
        }
    }
    return false;
}

QT_END_NAMESPACE
