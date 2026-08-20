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

#ifndef QCUSTOM3DITEM_P_H
#define QCUSTOM3DITEM_P_H

#include <QtCore/private/qobject_p.h>
#include "qcustom3ditem.h"

QT_BEGIN_NAMESPACE

struct QCustomItemDirtyBitField
{
    bool textureDirty : 1;
    bool meshDirty : 1;
    bool positionDirty : 1;
    bool scalingDirty : 1;
    bool rotationDirty : 1;
    bool visibleDirty : 1;
    bool shadowCastingDirty : 1;

    QCustomItemDirtyBitField()
        : textureDirty(false)
        , meshDirty(false)
        , positionDirty(false)
        , scalingDirty(false)
        , rotationDirty(false)
        , visibleDirty(false)
        , shadowCastingDirty(false)
    {}
};

class QCustom3DItemPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QCustom3DItem)

public:
    QCustom3DItemPrivate();
    QCustom3DItemPrivate(const QString &meshFile,
                         QVector3D position,
                         QVector3D scaling,
                         const QQuaternion &rotation);
    ~QCustom3DItemPrivate() override;

    QImage textureImage();
    void clearTextureImage();
    void resetDirtyBits();

public:
    QImage m_textureImage;
    QString m_textureFile;
    QString m_meshFile;
    QVector3D m_position;
    bool m_positionAbsolute;
    QVector3D m_scaling;
    bool m_scalingAbsolute;
    QQuaternion m_rotation;
    bool m_rotationAbsolute;
    bool m_visible;
    bool m_shadowCasting;

    bool m_isLabelItem;
    bool m_isVolumeItem;

    QCustomItemDirtyBitField m_dirtyBits;

private:
    QCustom3DItemPrivate(QCustom3DItemPrivate *d);
};

QT_END_NAMESPACE

#endif
