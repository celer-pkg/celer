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

#ifndef QCATEGORY3DAXIS_P_H
#define QCATEGORY3DAXIS_P_H

#include "qabstract3daxis_p.h"
#include "qcategory3daxis.h"

QT_BEGIN_NAMESPACE

class QCategory3DAxisPrivate : public QAbstract3DAxisPrivate
{
    Q_DECLARE_PUBLIC(QCategory3DAxis)

public:
    QCategory3DAxisPrivate();
    ~QCategory3DAxisPrivate() override;

    void setDataLabels(const QStringList &labels);

protected:
    bool allowZero() override;
    bool allowNegatives() override;
    bool allowMinMaxSame() override;

private:
    bool m_labelsExplicitlySet;
};

QT_END_NAMESPACE

#endif
