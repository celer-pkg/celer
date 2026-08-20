// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QQUICKLAYERITEM_P_H
#define QQUICKLAYERITEM_P_H

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

#include <QtGui/qmatrix4x4.h>
#include <QtQuick/qquickitem.h>
#include <QtLottieVectorImageHelpers/qtlottievectorimagehelpersexports.h>

QT_BEGIN_NAMESPACE

class Q_LOTTIEVECTORIMAGEHELPERS_EXPORT QQuickLayerItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QMatrix4x4 transformMatrix READ transformMatrix NOTIFY transformMatrixChanged)
    QML_NAMED_ELEMENT(LayerItem)
    Q_DECLARE_PRIVATE(QQuickItem)
    QML_ADDED_IN_VERSION(6, 10)

public:
    QQuickLayerItem(QQuickItem *parent = nullptr);
    QMatrix4x4 transformMatrix();

protected:
    void itemChange(ItemChange change, const ItemChangeData &) override;

private:
    bool m_transformDirty = true;
    QMatrix4x4 m_transform;

signals:
    void transformMatrixChanged();
};

QT_END_NAMESPACE

#endif // QQUICKLAYERITEM_P_H

