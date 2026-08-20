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

#ifndef ABSTRACTRENDERITEM_P_H
#define ABSTRACTRENDERITEM_P_H

#include <private/datavisualizationglobal_p.h>
#include <private/labelitem_p.h>

#include <QtGui/QVector3D>
#include <QtGui/QQuaternion>

QT_BEGIN_NAMESPACE

class AbstractRenderItem
{
public:
    AbstractRenderItem();
    AbstractRenderItem(const AbstractRenderItem &other);
    AbstractRenderItem &operator=(const AbstractRenderItem &other) = default;
    virtual ~AbstractRenderItem();

    // Position in 3D scene
    inline void setTranslation(const QVector3D &translation) { m_translation = translation; }
    inline const QVector3D &translation() const {return m_translation; }

    inline const QQuaternion &rotation() const { return m_rotation; }
    inline void setRotation(const QQuaternion &rotation)
    {
        if (rotation.isNull())
            m_rotation = identityQuaternion;
        else
            m_rotation = rotation;
    }

protected:
    QVector3D m_translation;
    QQuaternion m_rotation;

    friend class QAbstractDataItem;
};

QT_END_NAMESPACE

#endif
