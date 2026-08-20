// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

/****************************************************************************
**
** This is a simple QGLWidget displaying an openGL wireframe box
**
** The OpenGL code is mostly borrowed from Brian Pauls "spin" example
** in the Mesa distribution
**
****************************************************************************/

#include "glbox.h"
#include <QAxAggregated>
#include <QUuid>
//! [0]
#include <objsafe.h>
//! [0]

/*!
  Create a GLBox widget
*/

GLBox::GLBox(QWidget *parent, const char *name)
    : QOpenGLWidget(parent)
{
    setObjectName(name);

    QSurfaceFormat format;
    format.setVersion(1, 1);
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
    setFormat(format);
}


/*!
  Release allocated resources
*/

GLBox::~GLBox()
{
    makeCurrent();

    if (m_object)
        glDeleteLists(m_object, 1);
}


/*!
  Paint the box. The actual openGL commands for drawing the box are
  performed here.
*/

void GLBox::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glLoadIdentity();
    glTranslated(0, 0, -10);
    glScaled(m_scale, m_scale, m_scale);

    glRotated(m_xRot, 1, 0, 0);
    glRotated(m_yRot, 0, 1, 0);
    glRotated(m_zRot, 0, 0, 1);

    glCallList(m_object);
}


/*!
  Set up the OpenGL rendering state, and define display list
*/

void GLBox::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, 1);           // Let OpenGL clear to black
    m_object = makeObject();            // Generate an OpenGL display list
    glShadeModel(GL_FLAT);
}



/*!
  Set up the OpenGL view port, matrix mode, etc.
*/

void GLBox::resizeGL(int w, int h)
{
    glViewport(0, 0, (GLint)w, (GLint)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1, 1, -1, 1, 5, 15);
    glMatrixMode(GL_MODELVIEW);
}


/*!
  Generate an OpenGL display list for the object to be shown, i.e. the box
*/

GLuint GLBox::makeObject()
{
    GLuint list;

    list = glGenLists(1);

    glNewList(list, GL_COMPILE);

    glColor3d(1, 1, 1);                      // Shorthand for glColor3f or glIndex

    glLineWidth(2);

    glBegin(GL_LINE_LOOP);
    glVertex3d( 1,  0.5, -0.4);
    glVertex3d( 1, -0.5, -0.4);
    glVertex3d(-1, -0.5, -0.4);
    glVertex3d(-1,  0.5, -0.4);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex3d( 1,  0.5, 0.4);
    glVertex3d( 1, -0.5, 0.4);
    glVertex3d(-1, -0.5, 0.4);
    glVertex3d(-1,  0.5, 0.4);
    glEnd();

    glBegin(GL_LINES);
    glVertex3d( 1,  0.5, -0.4);   glVertex3d( 1,  0.5, 0.4);
    glVertex3d( 1, -0.5, -0.4);   glVertex3d( 1, -0.5, 0.4);
    glVertex3d(-1, -0.5, -0.4);   glVertex3d(-1, -0.5, 0.4);
    glVertex3d(-1,  0.5, -0.4);   glVertex3d(-1,  0.5, 0.4);
    glEnd();

    glEndList();

    return list;
}


/*!
  Set the rotation angle of the object to \e degrees around the X axis.
*/

void GLBox::setXRotation(int degrees)
{
    m_xRot = GLdouble(degrees % 360);
    update();
}


/*!
  Set the rotation angle of the object to \e degrees around the Y axis.
*/

void GLBox::setYRotation(int degrees)
{
    m_yRot = GLdouble(degrees % 360);
    update();
}


/*!
  Set the rotation angle of the object to \e degrees around the Z axis.
*/

void GLBox::setZRotation(int degrees)
{
    m_zRot =  GLdouble(degrees % 360);
    update();
}

//! [1]
class ObjectSafetyImpl : public QAxAggregated,
                         public IObjectSafety
{
public:
//! [1] //! [2]
    explicit ObjectSafetyImpl() = default;

    long queryInterface(const QUuid &iid, void **iface) override
    {
        *iface = nullptr;
        if (iid != IID_IObjectSafety)
            return E_NOINTERFACE;

        *iface = static_cast<IObjectSafety*>(this);
        AddRef();
        return S_OK;
    }

//! [2] //! [3]
    QAXAGG_IUNKNOWN;

//! [3] //! [4]
    HRESULT WINAPI GetInterfaceSafetyOptions(REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions) override
    {
        Q_UNUSED(riid);
        *pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_DATA | INTERFACESAFE_FOR_UNTRUSTED_CALLER;
        *pdwEnabledOptions = INTERFACESAFE_FOR_UNTRUSTED_DATA | INTERFACESAFE_FOR_UNTRUSTED_CALLER;
        return S_OK;
    }

    HRESULT WINAPI SetInterfaceSafetyOptions(REFIID riid, DWORD pdwSupportedOptions, DWORD pdwEnabledOptions) override
    {
        Q_UNUSED(riid);
        Q_UNUSED(pdwSupportedOptions);
        Q_UNUSED(pdwEnabledOptions);
        return S_OK;
    }
};
//! [4] //! [5]

QAxAggregated *GLBox::createAggregate()
{
    return new ObjectSafetyImpl();
}
//! [5]
