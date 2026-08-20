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

#ifndef QLOGVALUE3DAXISFORMATTER_P_H
#define QLOGVALUE3DAXISFORMATTER_P_H

#include "qlogvalue3daxisformatter.h"
#include "qvalue3daxisformatter_p.h"

QT_BEGIN_NAMESPACE

class QLogValue3DAxisFormatterPrivate : public QValue3DAxisFormatterPrivate
{
    Q_DECLARE_PUBLIC(QLogValue3DAxisFormatter)

public:
    QLogValue3DAxisFormatterPrivate();
    ~QLogValue3DAxisFormatterPrivate() override;

    void recalculate();
    void populateCopy(QValue3DAxisFormatter &copy) const;

    float positionAt(float value) const;
    float valueAt(float position) const;

protected:
    qreal m_base;
    qreal m_logMin;
    qreal m_logMax;
    qreal m_logRangeNormalizer;
    bool m_autoSubGrid;
    bool m_edgeLabelsVisible;

private:
    bool m_evenMinSegment;
    bool m_evenMaxSegment;
};

QT_END_NAMESPACE

#endif
