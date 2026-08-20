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

#ifndef QVALUE3DAXISFORMATTER_P_H
#define QVALUE3DAXISFORMATTER_P_H

#include <QtCore/QLocale>
#include <QtCore/private/qobject_p.h>
#include "qvalue3daxisformatter.h"
#include "utils_p.h"

QT_BEGIN_NAMESPACE

class QValue3DAxis;

class QValue3DAxisFormatterPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QValue3DAxisFormatter)

public:
    QValue3DAxisFormatterPrivate();
    ~QValue3DAxisFormatterPrivate() override;

    void recalculate();
    void doRecalculate();
    void populateCopy(QValue3DAxisFormatter &copy);
    void doPopulateCopy(QValue3DAxisFormatterPrivate &copy);

    QString stringForValue(qreal value, const QString &format);
    float positionAt(float value) const;
    float valueAt(float position) const;

    void setAxis(QValue3DAxis *axis);
    void markDirty(bool labelsChange);

protected:
    bool m_needsRecalculate;

    float m_min;
    float m_max;
    float m_rangeNormalizer;

    QList<float> m_gridPositions;
    QList<float> m_subGridPositions;
    QList<float> m_labelPositions;
    QStringList m_labelStrings;

    QValue3DAxis *m_axis;

    QString m_previousLabelFormat;
    QByteArray m_labelFormatArray;
    Utils::ParamType m_preparsedParamType;

    bool m_allowNegatives;
    bool m_allowZero;

    QLocale m_locale;
    QString m_formatPreStr;
    QString m_formatPostStr;
    int m_formatPrecision;
    char m_formatSpec;
    bool m_cLocaleInUse;
};

QT_END_NAMESPACE

#endif
