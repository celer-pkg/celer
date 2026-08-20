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

#ifndef DRAWER_P_H
#define DRAWER_P_H

#include <private/datavisualizationglobal_p.h>
#include <private/labelitem_p.h>
#include <private/abstractrenderitem_p.h>

#include <QtDataVisualization/q3dbars.h>
#include <QtDataVisualization/q3dtheme.h>

QT_BEGIN_NAMESPACE

class ShaderHelper;
class ObjectHelper;
class AbstractObjectHelper;
class SurfaceObject;
class TextureHelper;
class Q3DCamera;
class Abstract3DRenderer;
class QQuaternion;
class ScatterPointBufferHelper;

class Drawer : public QObject, public QOpenGLFunctions
{
    Q_OBJECT

public:
    enum LabelPosition {
        LabelBelow = 0,
        LabelLow,
        LabelMid,
        LabelHigh,
        LabelOver,
        LabelBottom,    // Absolute positions from here onward, used for axes (QDataItem is ignored)
        LabelTop,
        LabelLeft,
        LabelRight
    };

public:
    explicit Drawer(Q3DTheme *theme);
    ~Drawer();

    void initializeOpenGL();

    void setTheme(Q3DTheme *theme);
    Q3DTheme *theme() const;
    QFont font() const;
    inline GLfloat scaledFontSize() const { return m_scaledFontSize; }

    void drawObject(ShaderHelper *shader, AbstractObjectHelper *object, GLuint textureId = 0,
                    GLuint depthTextureId = 0, GLuint textureId3D = 0);
    void drawSelectionObject(ShaderHelper *shader, AbstractObjectHelper *object);
    void drawSurfaceGrid(ShaderHelper *shader, SurfaceObject *object);
    void drawPoint(ShaderHelper *shader);
    void drawPoints(ShaderHelper *shader, ScatterPointBufferHelper *object, GLuint textureId);
    void drawLine(ShaderHelper *shader);
    void drawLabel(const AbstractRenderItem &item, const LabelItem &labelItem,
                   const QMatrix4x4 &viewmatrix, const QMatrix4x4 &projectionmatrix,
                   const QVector3D &positionComp, const QQuaternion &rotation, GLfloat itemHeight,
                   QAbstract3DGraph::SelectionFlags mode, ShaderHelper *shader, ObjectHelper *object,
                   const Q3DCamera *camera, bool useDepth = false, bool rotateAlong = false,
                   LabelPosition position = LabelOver,
                   Qt::Alignment alignment = Qt::AlignCenter, bool isSlicing = false,
                   bool isSelecting = false);

    void generateSelectionLabelTexture(Abstract3DRenderer *item);
    void generateLabelItem(LabelItem &item, const QString &text, int widestLabel = 0);

Q_SIGNALS:
    void drawerChanged();

private:
    Q3DTheme *m_theme;
    TextureHelper *m_textureHelper;
    GLuint m_pointbuffer;
    GLuint m_linebuffer;
    GLfloat m_scaledFontSize;
};

QT_END_NAMESPACE

#endif
