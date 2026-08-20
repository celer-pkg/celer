// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSVGUTILS_P_H
#define QSVGUTILS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QString>
#include <QtGui/qpainterpath.h>
#include <optional>

QT_BEGIN_NAMESPACE

namespace QSvgUtils {

enum LengthType {
    LT_PERCENT,
    LT_PX,
    LT_PC,
    LT_PT,
    LT_MM,
    LT_CM,
    LT_IN,
    LT_OTHER
};

bool isDigit(ushort ch);
qreal toDouble(QStringView *str);
qreal toDouble(QStringView str, bool *ok = NULL);
qreal parseLength(QStringView str, LengthType *type, bool *ok = NULL);
qreal convertToPixels(qreal len, bool , LengthType type);
std::optional<qreal> parseAngle(QStringView str);
void parseNumbersArray(QStringView *str, QVarLengthArray<qreal, 8> &points,
                       const char *pattern = nullptr);
std::optional<QPainterPath> parsePathDataFast(QStringView dataStr, bool limitLength = true);

}

QT_END_NAMESPACE

#endif // QSVGUTILS_P_H
