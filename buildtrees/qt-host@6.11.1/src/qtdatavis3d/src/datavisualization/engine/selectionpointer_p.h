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

#ifndef SELECTIONPOINTER_P_H
#define SELECTIONPOINTER_P_H

#include "datavisualizationglobal_p.h"
#include "surface3dcontroller_p.h"

#include <QtGui/qquaternion.h>

QT_BEGIN_NAMESPACE

class ShaderHelper;
class ObjectHelper;
class TextureHelper;
class Drawer;

class SelectionPointer : public QObject, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit SelectionPointer(Drawer *drawer);
    ~SelectionPointer();

    void renderSelectionPointer(GLuint defaultFboHandle = 0, bool useOrtho = false);
    void renderSelectionLabel(GLuint defaultFboHandle = 0, bool useOrtho = false);
    void setPosition(const QVector3D &position);
    void setLabel(const QString &label, bool themeChange = false);
    void setPointerObject(ObjectHelper *object);
    void setLabelObject(ObjectHelper *object);
    void handleDrawerChange();
    void updateBoundingRect(const QRect &rect);
    void updateScene(Q3DScene *scene);
    void updateSliceData(bool sliceActivated, GLfloat autoScaleAdjustment);
    void setHighlightColor(const QVector4D &colorVector);
    void setRotation(const QQuaternion &rotation);

private:
    void initializeOpenGL();
    void initShaders();

private:
    ShaderHelper *m_labelShader;
    ShaderHelper *m_pointShader;
    ObjectHelper *m_labelObj; // Not owned
    ObjectHelper *m_pointObj; // Not owned
    TextureHelper *m_textureHelper;
    Q3DTheme *m_cachedTheme;
    bool m_labelBackground;
    LabelItem m_labelItem;
    Drawer *m_drawer;
    QRect m_mainViewPort;
    QVector3D m_position;
    Q3DScene *m_cachedScene;
    QString m_label;
    bool m_cachedIsSlicingActivated;
    GLfloat m_autoScaleAdjustment;
    QVector4D m_highlightColor;
    QQuaternion m_rotation;
};

QT_END_NAMESPACE

#endif
