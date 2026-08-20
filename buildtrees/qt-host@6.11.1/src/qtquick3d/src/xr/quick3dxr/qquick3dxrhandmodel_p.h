// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DXRHANDTRACKERINPUT_H
#define QQUICK3DXRHANDTRACKERINPUT_H

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
#include <QVector2D>
#include <QVector3D>
#include <QQuaternion>
#include <QtQml/qqml.h>
#include <private/qquick3dmodel_p.h>

#include <QQuick3DGeometry>
#include <QtQuick3DXr/private/qtquick3dxrglobal_p.h>

QT_BEGIN_NAMESPACE

class QQuick3DXrHandInput;

class QQuick3DXrHandModel : public QQuick3DModel
{
    Q_OBJECT

    Q_PROPERTY(Hand hand READ hand WRITE setHand NOTIFY handChanged FINAL)
    QML_NAMED_ELEMENT(XrHandModel)
    QML_ADDED_IN_VERSION(6, 8)

public:
    enum Hand : quint8 {
        LeftHand = 0,
        RightHand,
        Unknown,
    };
    Q_ENUM(Hand)

    QQuick3DXrHandModel(QQuick3DNode *parent = nullptr);

    void componentComplete() override;

    Hand hand() const;
    void setHand(Hand newHand);

Q_SIGNALS:
    void handChanged();
    void handTrackerChanged();

private Q_SLOTS:
    void updatePose();

private:
    void setupModel();
    QQuick3DXrHandInput *m_handTracker = nullptr;
    bool m_initialized = false;
    Hand m_hand;
};

QT_END_NAMESPACE

#endif // QQUICK3DXRHANDTRACKERINPUT_H
