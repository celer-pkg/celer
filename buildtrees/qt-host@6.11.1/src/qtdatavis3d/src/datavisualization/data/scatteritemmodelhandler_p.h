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

#ifndef SCATTERITEMMODELHANDLER_P_H
#define SCATTERITEMMODELHANDLER_P_H

#include "abstractitemmodelhandler_p.h"
#include "qitemmodelscatterdataproxy_p.h"

QT_BEGIN_NAMESPACE

class ScatterItemModelHandler : public AbstractItemModelHandler
{
    Q_OBJECT
public:
    ScatterItemModelHandler(QItemModelScatterDataProxy *proxy, QObject *parent = 0);
    virtual ~ScatterItemModelHandler();

public Q_SLOTS:
    void handleDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                           const QList<int> &roles = QList<int>()) override;
    void handleRowsInserted(const QModelIndex &parent, int start, int end) override;
    void handleRowsRemoved(const QModelIndex &parent, int start, int end) override;

protected:
    void resolveModel() override;

private:
    void modelPosToScatterItem(int modelRow, int modelColumn, QScatterDataItem &item);

    QItemModelScatterDataProxy *m_proxy; // Not owned
    QScatterDataArray *m_proxyArray; // Not owned
    int m_xPosRole;
    int m_yPosRole;
    int m_zPosRole;
    int m_rotationRole;
    QRegularExpression m_xPosPattern;
    QRegularExpression m_yPosPattern;
    QRegularExpression m_zPosPattern;
    QRegularExpression m_rotationPattern;
    QString m_xPosReplace;
    QString m_yPosReplace;
    QString m_zPosReplace;
    QString m_rotationReplace;
    bool m_haveXPosPattern;
    bool m_haveYPosPattern;
    bool m_haveZPosPattern;
    bool m_haveRotationPattern;
};

QT_END_NAMESPACE

#endif
