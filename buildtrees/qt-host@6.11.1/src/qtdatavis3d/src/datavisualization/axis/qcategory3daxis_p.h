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

#ifndef QCATEGORY3DAXIS_P_H
#define QCATEGORY3DAXIS_P_H

#include "qcategory3daxis.h"
#include "qabstract3daxis_p.h"

QT_BEGIN_NAMESPACE

class QCategory3DAxisPrivate : public QAbstract3DAxisPrivate
{
    Q_OBJECT

public:
    QCategory3DAxisPrivate(QCategory3DAxis *q);
    virtual ~QCategory3DAxisPrivate();

    void setDataLabels(const QStringList &labels);

protected:
    bool allowZero() override;
    bool allowNegatives() override;
    bool allowMinMaxSame() override;

private:
    QCategory3DAxis *qptr();

    bool m_labelsExplicitlySet;
    friend class QCategory3DAxis;
};

QT_END_NAMESPACE

#endif
