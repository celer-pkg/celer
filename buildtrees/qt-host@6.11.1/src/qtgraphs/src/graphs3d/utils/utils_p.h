// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtGraphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef UTILS_P_H
#define UTILS_P_H

#include <QtCore/qlocale.h>
#include <QtGui/qimage.h>
#include <QtGui/qquaternion.h>
#include "common/theme/qquickgraphscolor_p.h"
#include "qabstract3dseries.h"
#include <private/qgraphsglobal_p.h>

QT_FORWARD_DECLARE_CLASS(QLinearGradient)

QT_BEGIN_NAMESPACE

class QQuickGradient;

class Utils
{
public:
    enum class ParamType { Unknown, Int, UInt, Real };

    static ParamType preParseFormat(
        const QString &format, QString &preStr, QString &postStr, int &precision, char &formatSpec);
    static QString formatLabelSprintf(const QByteArray &format, ParamType paramType, qreal value);
    static QString formatLabelLocalized(ParamType paramType,
                                        qreal value,
                                        const QLocale &locale,
                                        const QString &preStr,
                                        const QString &postStr,
                                        int precision,
                                        char formatSpec,
                                        const QByteArray &format);
    static QString defaultLabelFormat();

    static float wrapValue(float value, float min, float max);
    static QQuaternion calculateRotation(QVector3D xyzRotations);
    static void verifyGradientCompleteness(QLinearGradient &gradient);
    static void setSeriesGradient(QAbstract3DSeries *series, QJSValue gradient, GradientType type);
    static void setSeriesGradient(QAbstract3DSeries *series,
                                  QQuickGradient *gradient,
                                  GradientType type);
    static void connectSeriesGradient(QAbstract3DSeries *series,
                                      QJSValue newGradient,
                                      GradientType type,
                                      QJSValue &memberGradient);
    static bool imageHasTransparency(const QImage &image);

private:
    static ParamType mapFormatCharToParamType(char formatSpec);
};

QT_END_NAMESPACE

#endif
