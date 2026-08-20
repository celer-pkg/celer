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

#ifndef LABELITEM_P_H
#define LABELITEM_P_H

#include <private/datavisualizationglobal_p.h>
#include <QtCore/QSize>

QT_BEGIN_NAMESPACE

class LabelItem
{
public:
    explicit LabelItem();
    ~LabelItem();

    void setSize(const QSize &size);
    QSize size() const;
    void setTextureId(GLuint textureId);
    GLuint textureId() const;
    void clear();

private:
    Q_DISABLE_COPY(LabelItem)

    QSize m_size;
    GLuint m_textureId;
};

QT_END_NAMESPACE

#endif
