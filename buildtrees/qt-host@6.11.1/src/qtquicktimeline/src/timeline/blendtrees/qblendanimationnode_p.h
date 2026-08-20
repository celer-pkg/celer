// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QBLENDANIMATIONNODE_P_H
#define QBLENDANIMATIONNODE_P_H

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

#include <QObject>
#include <QtQml>
#include <QtQuick/private/qquickanimation_p.h>
#include <QtQuickTimelineBlendTrees/private/qblendtreenode_p.h>

QT_BEGIN_NAMESPACE

class Q_QUICKTIMELINEBLENDTREES_EXPORT QBlendAnimationNode : public QBlendTreeNode
{
    Q_OBJECT
    Q_PROPERTY(QBlendTreeNode *source1 READ source1 WRITE setSource1 NOTIFY source1Changed FINAL)
    Q_PROPERTY(QBlendTreeNode *source2 READ source2 WRITE setSource2 NOTIFY source2Changed FINAL)
    Q_PROPERTY(qreal weight READ weight WRITE setWeight NOTIFY weightChanged FINAL)
    QML_NAMED_ELEMENT(BlendAnimationNode)
public:
    explicit QBlendAnimationNode(QObject *parent = nullptr);

    QBlendTreeNode *source1() const;
    void setSource1(QBlendTreeNode *newSource1);

    QBlendTreeNode *source2() const;
    void setSource2(QBlendTreeNode *newSource2);

    qreal weight() const;
    void setWeight(qreal newWeight);

private Q_SLOTS:
    void handleInputFrameDataChanged();

Q_SIGNALS:
    void source1Changed();
    void source2Changed();
    void weightChanged();

private:
    QBlendTreeNode *m_source1 = nullptr;
    QBlendTreeNode *m_source2 = nullptr;
    qreal m_weight = 0.5;

    QMetaObject::Connection m_source1OutputConnection;
    QMetaObject::Connection m_source2OutputConnection;
    QMetaObject::Connection m_source1DestroyedConnection;
    QMetaObject::Connection m_source2DestroyedConnection;
};

QT_END_NAMESPACE

#endif // QBLENDANIMATIONNODE_P_H
