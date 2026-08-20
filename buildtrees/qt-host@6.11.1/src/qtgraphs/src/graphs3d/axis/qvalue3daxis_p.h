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

#ifndef QVALUE3DAXIS_P_H
#define QVALUE3DAXIS_P_H

#include "qabstract3daxis_p.h"
#include "qvalue3daxis.h"

QT_BEGIN_NAMESPACE

class QValue3DAxisPrivate : public QAbstract3DAxisPrivate
{
    Q_DECLARE_PUBLIC(QValue3DAxis)

public:
    QValue3DAxisPrivate();
    ~QValue3DAxisPrivate() override;

    void setRange(float min, float max, bool suppressWarnings = false) override;
    void setMin(float min) override;
    void setMax(float max) override;

    void emitLabelsChanged();
    void emitFormatterDirty();

protected:
    void updateLabels() override;

    bool allowZero() override;
    bool allowNegatives() override;
    bool allowMinMaxSame() override;

    qsizetype m_segmentCount;
    qsizetype m_subSegmentCount;
    QString m_labelFormat;
    bool m_labelsDirty;
    QValue3DAxisFormatter *m_formatter;
    bool m_reversed;
};

QT_END_NAMESPACE

#endif
