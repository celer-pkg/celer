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

#ifndef BARITEMMODELHANDLER_P_H
#define BARITEMMODELHANDLER_P_H

#include "abstractitemmodelhandler_p.h"
#include "qitemmodelbardataproxy_p.h"

QT_BEGIN_NAMESPACE

class BarItemModelHandler : public AbstractItemModelHandler
{
    Q_OBJECT
public:
    BarItemModelHandler(QItemModelBarDataProxy *proxy, QObject *parent = 0);
    virtual ~BarItemModelHandler();

public Q_SLOTS:
    void handleDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                           const QList<int> &roles = QList<int>()) override;

protected:
    void resolveModel() override;

    QItemModelBarDataProxy *m_proxy; // Not owned
    QBarDataArray *m_proxyArray; // Not owned
    int m_columnCount;
    int m_valueRole;
    int m_rotationRole;
    QRegularExpression m_valuePattern;
    QRegularExpression m_rotationPattern;
    QString m_valueReplace;
    QString m_rotationReplace;
    bool m_haveValuePattern;
    bool m_haveRotationPattern;
};

QT_END_NAMESPACE

#endif
