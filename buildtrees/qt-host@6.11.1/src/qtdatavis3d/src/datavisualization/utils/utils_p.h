// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtDataVisualization API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef UTILS_P_H
#define UTILS_P_H

#include "datavisualizationglobal_p.h"

QT_FORWARD_DECLARE_CLASS(QLinearGradient)

QT_BEGIN_NAMESPACE

class QQuaternion;

class Utils
{
public:
    enum ParamType {
        ParamTypeUnknown = 0,
        ParamTypeInt,
        ParamTypeUInt,
        ParamTypeReal
    };

    static GLuint getNearestPowerOfTwo(GLuint value);
    static QVector4D vectorFromColor(const QColor &color);
    static QColor colorFromVector(const QVector3D &colorVector);
    static QColor colorFromVector(const QVector4D &colorVector);
    static QImage printTextToImage(const QFont &font,
                                   const QString &text,
                                   const QColor &bgrColor,
                                   const QColor &txtColor,
                                   bool labelBackground,
                                   bool borders = false,
                                   int maxLabelWidth = 0);
    static QVector4D getSelection(QPoint mousepos, int height);
    static QImage getGradientImage(QLinearGradient &gradient);

    static ParamType preParseFormat(const QString &format, QString &preStr, QString &postStr,
                                    int &precision, char &formatSpec);
    static QString formatLabelSprintf(const QByteArray &format, ParamType paramType, qreal value);
    static QString formatLabelLocalized(ParamType paramType, qreal value,
                               const QLocale &locale, const QString &preStr, const QString &postStr,
                               int precision, char formatSpec, const QByteArray &format);
    static QString defaultLabelFormat();

    static float wrapValue(float value, float min, float max);
    static QQuaternion calculateRotation(const QVector3D &xyzRotations);
    static bool isOpenGLES();
    static void resolveStatics();

private:
    static ParamType mapFormatCharToParamType(char formatSpec);
};

QT_END_NAMESPACE

#endif
