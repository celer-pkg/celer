// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

/****************************************************************************
**
** This is a simple QGLWidget displaying an openGL wireframe box
**
****************************************************************************/

#ifndef GLBOX_H
#define GLBOX_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_1_1>
//! [0]
#include <QAxBindable>

class GLBox : public QOpenGLWidget,
              public QOpenGLFunctions_1_1,
              public QAxBindable
{
    Q_OBJECT
    Q_CLASSINFO("ClassID",     "{5fd9c22e-ed45-43fa-ba13-1530bb6b03e0}")
    Q_CLASSINFO("InterfaceID", "{33b051af-bb25-47cf-a390-5cfd2987d26a}")
    Q_CLASSINFO("EventsID",    "{8c996c29-eafa-46ac-a6f9-901951e765b5}")
    //! [0] //! [1]

public:
    explicit GLBox(QWidget *parent, const char *name = nullptr);
    virtual ~GLBox();
    QAxAggregated *createAggregate() override;

public slots:
    void                setXRotation(int degrees);
//! [1]
    void                setYRotation(int degrees);
    void                setZRotation(int degrees);

protected:
    void                initializeGL() override;
    void                paintGL() override;
    void                resizeGL(int w, int h) override;
    virtual GLuint      makeObject();

private:
    GLuint  m_object = 0;
    GLdouble m_xRot = 0;
    GLdouble m_yRot = 0;
    GLdouble m_zRot = 0;
    GLdouble m_scale = 1.25;
};

#endif // GLBOX_H
