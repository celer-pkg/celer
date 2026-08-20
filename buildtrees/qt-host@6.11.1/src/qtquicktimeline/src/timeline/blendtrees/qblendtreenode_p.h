// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QBLENDTREENODE_P_H
#define QBLENDTREENODE_P_H

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

#include "qtquicktimelineblendtreesglobal.h"

#include <QObject>
#include <QtQml>

QT_BEGIN_NAMESPACE

class Q_QUICKTIMELINEBLENDTREES_EXPORT QBlendTreeNode : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool outputEnabled READ outputEnabled WRITE setOutputEnabled NOTIFY outputEnabledChanged FINAL)
    QML_NAMED_ELEMENT(BlendTreeNode)
    QML_UNCREATABLE("Interface Class")
public:
    explicit QBlendTreeNode(QObject *parent = nullptr);

    const QHash<QQmlProperty, QVariant> &frameData();

    bool outputEnabled() const;
    void setOutputEnabled(bool isOutputEnabled);

Q_SIGNALS:
    void frameDataChanged();
    void outputEnabledChanged();

protected:
    QHash<QQmlProperty, QVariant> m_frameData;

private Q_SLOTS:
    void handleFrameDataChanged();

private:
    bool m_outputEnabled = false;
};

QT_END_NAMESPACE

#endif // QBLENDTREENODE_P_H
