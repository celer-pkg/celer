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

#ifndef QABSTRACT3DAXIS_P_H
#define QABSTRACT3DAXIS_P_H

#include <QtCore/private/qobject_p.h>
#include "qabstract3daxis.h"

QT_BEGIN_NAMESPACE

class QAbstract3DAxisPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QAbstract3DAxis)

public:
    QAbstract3DAxisPrivate(QAbstract3DAxis::AxisType type);
    ~QAbstract3DAxisPrivate() override;

    void setOrientation(QAbstract3DAxis::AxisOrientation orientation);

    bool isDefaultAxis() { return m_isDefaultAxis; }
    void setDefaultAxis(bool isDefault) { m_isDefaultAxis = isDefault; }

    virtual void setRange(float min, float max, bool suppressWarnings = false);
    virtual void setMin(float min);
    virtual void setMax(float max);

protected:
    virtual void updateLabels();
    virtual bool allowZero() = 0;
    virtual bool allowNegatives() = 0;
    virtual bool allowMinMaxSame() = 0;

    QString m_title;
    QStringList m_labels;
    QAbstract3DAxis::AxisOrientation m_orientation;
    QAbstract3DAxis::AxisType m_type;
    bool m_isDefaultAxis;
    float m_min;
    float m_max;
    bool m_autoAdjust;
    bool m_scaleLabelsByCount;
    qreal m_labelSize;
    float m_labelAutoAngle;
    float m_titleOffset;
    bool m_titleVisible;
    bool m_labelsVisible;
    bool m_titleFixed;

    friend class QScatterDataProxyPrivate;
    friend class QSurfaceDataProxyPrivate;
};

QT_END_NAMESPACE

#endif
