// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcanvascustombrush.h"
#include "engine/qcpainterengineutils_p.h"
#include "qcanvascustombrush_p.h"
#include <QFile>
#include <QDebug>

QT_BEGIN_NAMESPACE

#define C_D() auto *d = static_cast<QCanvasCustomBrushPrivate*>(baseData.get())
#define C_CD() const auto *d = static_cast<QCanvasCustomBrushPrivate*>(baseData.get())

static QShader getCustomShader(const QString &name)
{
    if (name.isEmpty())
        return QShader();

    QFile f(name);
    if (f.open(QIODevice::ReadOnly)) {
        return QShader::fromSerialized(f.readAll());
    } else {
        qWarning() << "Unable to create custom brush with shader:" << name;
        return QShader();
    }
}

/*!
    \class QCanvasCustomBrush
    \since 6.11
    \brief QCanvasCustomBrush is a brush with custom shaders.
    \inmodule QtCanvasPainter

    QCanvasCustomBrush is a stroke/fill brush with custom vertex and/or fragment shaders.

    These shaders are expected to be written in Vulkan-style GLSL, similarly to
    Qt Quick \l ShaderEffect shaders. They must always contain a \c QC_INCLUDE
    statement, either with \c{"customfrag.glsl"} or \c{"customvert.glsl"}. This
    makes available a uniform block, the image and font textures, and a few
    helper functions.

    Below is a simple example of a custom fragment shader:

    \code
        #version 440

        QC_INCLUDE "customfrag.glsl"

        void main()
        {
            float a = 0.6 + 0.2 * sin(0.1 * fragCoord.x + 4.0 * iTime);
            vec4 color = vec4(a, a, a, 1.0);
            fragColor = sdfFontAlpha() * globalAlpha * color;
            applyColorEffects(fragColor);
        }
    \endcode

    \c iTime is an example of a commonly used member in the built-in uniform
    block. Calling setTimeRunning() with \c true will make this value update
    automatically every frame, and can be used to drive animated content.

    Shaders that are used with QCanvasCustomBrush must always be added to the
    application project via the \c qc_add_shaders() CMake function, provided by
    the Qt Canvas Painter package. This function performs additional
    preprocessing at build time before internally invoking the standard \c
    qt_add_shaders().

    For example:
    \code
        qc_add_shaders(app "app_custombrush_shaders"
            PREFIX
                "/shaders"
            FILES
                brush1.frag
        )
    \endcode

    At run time, the generated \c{.qsb} file can be used for example like this:
    \code
        QCanvasCustomBrush customBrush(":/shaders/brush1.frag.qsb"));
        customBrush.setTimeRunning(true); // iTime updates automatically
        // expose custom data to the shader in data1
        customBrush.setData1(QVector4D(1.0, 2.0, 3.0, 4.0));
    \endcode

    The QCanvasCustomBrush can then be used in a fill, for example:
    \code
        painter->setFillStyle(customBrush);
    \endcode

    \sa {Qt Canvas Painter - Gallery Example}
*/

/*!
    Constructs a default custom brush.
*/

QCanvasCustomBrush::QCanvasCustomBrush()
    : QCanvasBrush(new QCanvasCustomBrushPrivate)
{
}

/*!
    Constructs a custom brush.

    The fragment shader is \a fragmentShader and the vertex shader is \a
    vertexShader. This constructor takes two filenames, where both files are
    expected to be \c{.qsb} files that are read and deserialized into QShader
    objects. The files can be a local file or embedded in the application via
    the \l{The Qt Resource System}.

    When not specified, \a vertexShader defaults to an empty string, which
    implies that the default, standard shader is used for the vertex stage. It
    is also possible to pass an empty string as \a fragmentShader, and only
    provide a custom shader for \a vertexShader.

    \sa setFragmentShader, setVertexShader
*/

QCanvasCustomBrush::QCanvasCustomBrush(const QString &fragmentShader,
                             const QString &vertexShader)
    : QCanvasBrush(new QCanvasCustomBrushPrivate)
{
    setFragmentShader(fragmentShader);
    setVertexShader(vertexShader);
}


/*!
    Destroys the custom brush.
*/

QCanvasCustomBrush::~QCanvasCustomBrush() = default;


/*!
   Returns the custom brush as a QVariant.
*/

QCanvasCustomBrush::operator QVariant() const
{
    return QVariant::fromValue(*this);
}

/*!
    \fn bool QCanvasCustomBrush::operator!=(const QCanvasCustomBrush &lhs, const QCanvasCustomBrush &rhs)

    \return \c true if the custom brush \a lhs is different from \a rhs; \c false otherwise.

    \sa operator==()
*/

/*!
    \fn bool QCanvasCustomBrush::operator==(const QCanvasCustomBrush &lhs, const QCanvasCustomBrush &rhs)

    \return \c true if the custom brush \a lhs is equal to \a rhs; \c false otherwise.

    \sa operator!=()
*/
bool comparesEqual(const QCanvasCustomBrush &lhs, const QCanvasCustomBrush &rhs) noexcept
{
    auto *d = QCanvasCustomBrushPrivate::get(&lhs);
    auto *bd = QCanvasCustomBrushPrivate::get(&rhs);
    if (bd == d)
        return true;

    if (d->fragmentShader != bd->fragmentShader
        || d->vertexShader != bd->vertexShader
        || d->timeRunning != bd->timeRunning
        || d->time != bd->time
        || d->data[0] != bd->data[0]
        || d->data[1] != bd->data[1]
        || d->data[2] != bd->data[2]
        || d->data[3] != bd->data[3])
        return false;

    return true;
}

#ifndef QT_NO_DEBUG_STREAM
/*!
  \internal
*/
QDebug operator<<(QDebug dbg, const QCanvasCustomBrush &)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "QCanvasCustomBrush()";
    return dbg;
}
#endif // QT_NO_DEBUG_STREAM


/*!
    Sets the custom brush to use \a fragmentShader.
    This must be path to a valid qsb file. The file can be a local file or
    embedded in the application via the \l{The Qt Resource System}.
*/

void QCanvasCustomBrush::setFragmentShader(const QString &fragmentShader)
{
    auto *d = QCanvasCustomBrushPrivate::get(this);
    detach();
    d->fragmentShader = getCustomShader(fragmentShader);
}

/*!
    Sets the custom brush to use \a fragmentShader.
*/

void QCanvasCustomBrush::setFragmentShader(const QShader &fragmentShader)
{
    auto *d = QCanvasCustomBrushPrivate::get(this);
    detach();
    d->fragmentShader = fragmentShader;
}

/*!
    Sets the custom brush to use \a vertexShader.
    This must be path to a valid qsb file. The file can be a local file or
    embedded in the application via the \l{The Qt Resource System}.
*/

void QCanvasCustomBrush::setVertexShader(const QString &vertexShader)
{
    auto *d = QCanvasCustomBrushPrivate::get(this);
    detach();
    d->vertexShader = getCustomShader(vertexShader);
}

/*!
    Sets the custom brush to use \a vertexShader.
*/

void QCanvasCustomBrush::setVertexShader(const QShader &vertexShader)
{
    auto *d = QCanvasCustomBrushPrivate::get(this);
    detach();
    d->vertexShader = vertexShader;
}

/*!
    Returns true if the time is running.
*/

bool QCanvasCustomBrush::timeRunning() const
{
    auto *d = QCanvasCustomBrushPrivate::get(this);
    return d->timeRunning;
}

/*!
    Sets the time running state to \a running. When this is true, the shader
    uniform \c iTime is updated automatically, and can be used to get the
    current animation running time in the shader.

    The default value is \c false.
*/

void QCanvasCustomBrush::setTimeRunning(bool running)
{
    auto *d = QCanvasCustomBrushPrivate::get(this);
    detach();
    d->timeRunning = running;
}

/*!
    Sets the uniform data1 value to \a data.
    This allows setting custom data into shaders.
*/

void QCanvasCustomBrush::setData1(const QVector4D &data)
{
    auto *d = QCanvasCustomBrushPrivate::get(this);
    detach();
    d->data[0] = data;
}

/*!
    Sets the uniform data2 value to \a data.
    This allows setting custom data into shaders.
*/

void QCanvasCustomBrush::setData2(const QVector4D &data)
{
    auto *d = QCanvasCustomBrushPrivate::get(this);
    detach();
    d->data[1] = data;
}

/*!
    Sets the uniform data3 value to \a data.
    This allows setting custom data into shaders.
*/

void QCanvasCustomBrush::setData3(const QVector4D &data)
{
    auto *d = QCanvasCustomBrushPrivate::get(this);
    detach();
    d->data[2] = data;
}

/*!
    Sets the uniform data4 value to \a data.
    This allows setting custom data into shaders.
*/

void QCanvasCustomBrush::setData4(const QVector4D &data)
{
    auto *d = QCanvasCustomBrushPrivate::get(this);
    detach();
    d->data[3] = data;
}

QCanvasBrushPrivate *QCanvasCustomBrushPrivate::clone()
{
    return new QCanvasCustomBrushPrivate(*this);
}

QCPaint QCanvasCustomBrushPrivate::createPaint(QCanvasPainter *) const
{
    QCPaint empty;
    return empty;
}

QT_END_NAMESPACE
