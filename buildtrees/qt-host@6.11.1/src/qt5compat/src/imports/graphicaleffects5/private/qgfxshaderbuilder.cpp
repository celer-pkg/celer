// Copyright (C) 2016 Jolla Ltd, author: <gunnar.sletta@jollamobile.com>
// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qgfxshaderbuilder_p.h"

#include <QtCore/QDebug>
#include <QtCore/QUrl>
#include <QtCore/QVarLengthArray>
#include <QtCore/QStandardPaths>
#include <QtCore/QCryptographicHash>
#include <QtCore/QDir>
#include <QtGui/QOffscreenSurface>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFunctions>

#include <QtQuick/qquickwindow.h>

#include <qmath.h>
#include <qnumeric.h>

QT_BEGIN_NAMESPACE

#ifndef GL_MAX_VARYING_COMPONENTS
#define GL_MAX_VARYING_COMPONENTS 0x8B4B
#endif

#ifndef GL_MAX_VARYING_FLOATS
#define GL_MAX_VARYING_FLOATS 0x8B4B
#endif

#ifndef GL_MAX_VARYING_VECTORS
#define GL_MAX_VARYING_VECTORS 0x8DFC
#endif

#ifndef GL_MAX_VERTEX_OUTPUT_COMPONENTS
#define GL_MAX_VERTEX_OUTPUT_COMPONENTS 0x9122
#endif

#if !defined(QT5COMPAT_MAX_BLUR_SAMPLES)
#define QT5COMPAT_MAX_BLUR_SAMPLES 15 // Conservative estimate for maximum varying vectors in
                                      // shaders (maximum 60 components on some Metal
                                      // implementations, hence 15 vectors of 4 components each)
#elif !defined(QT5COMPAT_MAX_BLUR_SAMPLES_GL)
#define QT5COMPAT_MAX_BLUR_SAMPLES_GL QT5COMPAT_MAX_BLUR_SAMPLES
#endif

#if !defined(QT5COMPAT_MAX_BLUR_SAMPLES_GL)
#define QT5COMPAT_MAX_BLUR_SAMPLES_GL 8 // minimum number of varyings in the ES 2.0 spec.
#endif

QGfxShaderBuilder::QGfxShaderBuilder()
{
    QList<QShaderBaker::GeneratedShader> targets =
    {
        { QShader::HlslShader, QShaderVersion(50) },
        { QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs) },
        { QShader::GlslShader, QShaderVersion(120) },
        { QShader::GlslShader, QShaderVersion(150) },
        { QShader::MslShader, QShaderVersion(12) },
        { QShader::SpirvShader, QShaderVersion(100) }
    };

    m_shaderBaker.setGeneratedShaders(targets);
    m_shaderBaker.setGeneratedShaderVariants({ QShader::StandardShader,
                                               QShader::BatchableVertexShader });

#ifndef QT_NO_OPENGL
    QSGRendererInterface::GraphicsApi graphicsApi = QQuickWindow::graphicsApi();
    if (graphicsApi == QSGRendererInterface::OpenGL) {
        // The following code makes the assumption that an OpenGL context the GUI
        // thread will get the same capabilities as the render thread's OpenGL
        // context. Not 100% accurate, but it works...
        QOpenGLContext context;
        if (!context.create()) {
            qDebug() << "failed to acquire GL context to resolve capabilities, using defaults..";
            m_maxBlurSamples = QT5COMPAT_MAX_BLUR_SAMPLES_GL;
            return;
        }

        QOffscreenSurface surface;
        // In very odd cases, we can get incompatible configs here unless we pass the
        // GL context's format on to the offscreen format.
        surface.setFormat(context.format());
        surface.create();

        QOpenGLContext *oldContext = QOpenGLContext::currentContext();
        QSurface *oldSurface = oldContext ? oldContext->surface() : 0;
        if (context.makeCurrent(&surface)) {
            QOpenGLFunctions *gl = context.functions();
            const bool coreProfile = context.format().profile() == QSurfaceFormat::CoreProfile;
            if (context.isOpenGLES()) {
                gl->glGetIntegerv(GL_MAX_VARYING_VECTORS, &m_maxBlurSamples);
            } else if (context.format().majorVersion() >= 3) {
                int components;
                gl->glGetIntegerv(coreProfile ? GL_MAX_VERTEX_OUTPUT_COMPONENTS : GL_MAX_VARYING_COMPONENTS, &components);
                m_maxBlurSamples = components / 2.0;
            } else {
                int floats;
                gl->glGetIntegerv(GL_MAX_VARYING_FLOATS, &floats);
                m_maxBlurSamples = floats / 2.0;
            }
            if (oldContext && oldSurface)
                oldContext->makeCurrent(oldSurface);
            else
                context.doneCurrent();
        } else {
            qDebug() << "QGfxShaderBuilder: Failed to acquire GL context to resolve capabilities, using defaults.";
            m_maxBlurSamples = QT5COMPAT_MAX_BLUR_SAMPLES_GL;
        }
    } else
#endif
    m_maxBlurSamples = QT5COMPAT_MAX_BLUR_SAMPLES;
}

QGfxShaderBuilder::~QGfxShaderBuilder()
    = default;

/*

    The algorithm works like this..

    For every two pixels we want to sample we take one sample between those
    two pixels and rely on linear interpoliation to get both values at the
    cost of one texture sample. The sample point is calculated based on the
    gaussian weights at the two texels.

    I've included the table here for future reference:

    Requested     Effective       Actual    Actual
    Samples       Radius/Kernel   Samples   Radius(*)
    -------------------------------------------------
    0             0 / 1x1         1         0
    1             0 / 1x1         1         0
    2             1 / 3x3         2         0
    3             1 / 3x3         2         0
    4             2 / 5x5         3         1
    5             2 / 5x5         3         1
    6             3 / 7x7         4         1
    7             3 / 7x7         4         1
    8             4 / 9x9         5         2
    9             4 / 9x9         5         2
    10            5 / 11x11       6         2
    11            5 / 11x11       6         2
    12            6 / 13x13       7         3
    13            6 / 13x13       7         3
    ...           ...             ...       ...

    When ActualSamples is an 'odd' nunber, sample center pixel separately:
    EffectiveRadius: 4
    EffectiveKernel: 9x9
    ActualSamples: 5
     -4  -3  -2  -1   0  +1  +2  +3  +4
    |   |   |   |   |   |   |   |   |   |
      \   /   \   /   |   \   /   \   /
       tL2     tL1    tC   tR1     tR2

    When ActualSamples is an 'even' number, sample 3 center pixels with two
    samples:
    EffectiveRadius: 3
    EffectiveKernel: 7x7
    ActualSamples: 4
     -3  -2  -1   0  +1  +2  +3
    |   |   |   |   |   |   |   |
      \   /   \   /   |    \   /
       tL1     tL0   tR0    tR2

    From this table we have the following formulas:
    EffectiveRadius = RequestedSamples / 2;
    EffectiveKernel = EffectiveRadius * 2 + 1
    ActualSamples   = 1 + RequstedSamples / 2;
    ActualRadius    = RequestedSamples / 4;

    (*) ActualRadius excludes the pixel pair sampled in the center
        for even 'actual sample' counts
*/

static qreal qgfx_gaussian(qreal x, qreal d)
{
    return qExp(- x * x / (2 * d * d));
}

struct QGfxGaussSample
{
    QByteArray name;
    qreal pos;
    qreal weight;
    inline void set(const QByteArray &n, qreal p, qreal w) {
        name = n;
        pos = p;
        weight = w;
    }
};

static void qgfx_declareBlur(QByteArray &shader, const QByteArray& direction, QGfxGaussSample *s, int samples)
{
    for (int i=0; i<samples; ++i) {
        shader += "layout(location = " + QByteArray::number(i) + ") " + direction + " vec2 ";
        shader += s[i].name;
        shader += ";\n";
    }
}

static void qgfx_buildGaussSamplePoints(QGfxGaussSample *p, int samples, int radius, qreal deviation)
{

    if ((samples % 2) == 1) {
        p[radius].set("tC", 0, 1);
        for (int i=0; i<radius; ++i) {
            qreal p0 = (i + 1) * 2 - 1;
            qreal p1 = (i + 1) * 2;
            qreal w0 = qgfx_gaussian(p0, deviation);
            qreal w1 = qgfx_gaussian(p1, deviation);
            qreal w = w0 + w1;
            qreal samplePos = (p0 * w0 + p1 * w1) / w;
            if (qIsNaN(samplePos)) {
                samplePos = 0;
                w = 0;
            }
            p[radius - i - 1].set("tL" + QByteArray::number(i), samplePos, w);
            p[radius + i + 1].set("tR" + QByteArray::number(i), -samplePos, w);
        }
    } else {
        { // tL0
            qreal wl = qgfx_gaussian(-1.0, deviation);
            qreal wc = qgfx_gaussian(0.0, deviation);
            qreal w = wl + wc;
            p[radius].set("tL0", -1.0 * wl / w, w);
            p[radius+1].set("tR0", 1.0, wl);  // reuse wl as gauss(-1)==gauss(1);
        }
        for (int i=0; i<radius; ++i) {
            qreal p0 = (i + 1) * 2;
            qreal p1 = (i + 1) * 2 + 1;
            qreal w0 = qgfx_gaussian(p0, deviation);
            qreal w1 = qgfx_gaussian(p1, deviation);
            qreal w = w0 + w1;
            qreal samplePos = (p0 * w0 + p1 * w1) / w;
            if (qIsNaN(samplePos)) {
                samplePos = 0;
                w = 0;
            }
            p[radius - i - 1].set("tL" + QByteArray::number(i+1), samplePos, w);
            p[radius + i + 2].set("tR" + QByteArray::number(i+1), -samplePos, w);

        }
    }
}

void qgfx_declareUniforms(QByteArray &shader, bool alphaOnly)
{
    shader += "layout(std140, binding = 0) uniform buf {\n"
              "    mat4 qt_Matrix;\n"
              "    float qt_Opacity;\n"
              "    float spread;\n"
              "    vec2 dirstep;\n";

    if (alphaOnly) {
        shader += "    vec4 color;\n"
                  "    float thickness;\n";
    }
    shader += "};\n\n";
}

QByteArray qgfx_gaussianVertexShader(QGfxGaussSample *p, int samples, bool alphaOnly)
{
    QByteArray shader;
    shader.reserve(1024);
    shader += "#version 440\n\n"
              "layout(location = 0) in vec4 qt_Vertex;\n"
              "layout(location = 1) in vec2 qt_MultiTexCoord0;\n\n";

    qgfx_declareUniforms(shader, alphaOnly);

    shader += "out gl_PerVertex { vec4 gl_Position; };\n\n";

    qgfx_declareBlur(shader, "out", p, samples);

    shader += "\nvoid main() {\n"
              "    gl_Position = qt_Matrix * qt_Vertex;\n\n";

    for (int i=0; i<samples; ++i) {
        shader += "    ";
        shader += p[i].name;
        shader += " = qt_MultiTexCoord0";
        if (p[i].pos != 0.0) {
            shader += " + spread * dirstep * float(";
            shader += QByteArray::number(p[i].pos);
            shader += ')';
        }
        shader += ";\n";
    }

    shader += "}\n";

    return shader;
}

QByteArray qgfx_gaussianFragmentShader(QGfxGaussSample *p, int samples, bool alphaOnly)
{
    QByteArray shader;
    shader.reserve(1024);
    shader += "#version 440\n\n";

    qgfx_declareUniforms(shader, alphaOnly);

    shader += "layout(binding = 1) uniform sampler2D source;";
    shader += "layout(location = 0) out vec4 fragColor;\n";

    qgfx_declareBlur(shader, "in", p, samples);

    shader += "\nvoid main() {\n"
              "    fragColor = ";
    if (alphaOnly)
        shader += "mix(vec4(0), color, clamp((";
    else
        shader += "(";

    qreal sum = 0;
    for (int i=0; i<samples; ++i)
        sum += p[i].weight;

    for (int i=0; i<samples; ++i) {
        shader += "\n                    + float(";
        shader += QByteArray::number(p[i].weight / sum);
        shader += ") * texture(source, ";
        shader += p[i].name;
        shader += ")";
        if (alphaOnly)
            shader += ".a";
    }

    shader += "\n                   )";
    if (alphaOnly)
        shader += "/thickness, 0.0, 1.0))";
    shader += "* qt_Opacity;\n}";

    return shader;
}

static QByteArray qgfx_fallbackVertexShader(bool alphaOnly)
{
    QByteArray vertexShader =
           "#version 440\n"
           "layout(location = 0) in vec4 qt_Vertex;\n"
           "layout(location = 1) in vec2 qt_MultiTexCoord0;\n\n";

    qgfx_declareUniforms(vertexShader, alphaOnly);

    vertexShader +=
           "layout(location = 0) out vec2 qt_TexCoord0;\n"
           "out gl_PerVertex { vec4 gl_Position; };\n"
           "void main() {\n"
           "    gl_Position = qt_Matrix * qt_Vertex;\n"
           "    qt_TexCoord0 = qt_MultiTexCoord0;\n"
           "}\n";

    return vertexShader;
}

static QByteArray qgfx_fallbackFragmentShader(int requestedRadius, qreal deviation, bool masked, bool alphaOnly)
{
    QByteArray fragShader = "#version 440\n\n";

    qgfx_declareUniforms(fragShader, alphaOnly);

    fragShader += "layout(binding = 1) uniform sampler2D source;\n";
    if (masked)
        fragShader += "layout(binding = 2) uniform sampler2D mask;\n";

    fragShader +=
        "layout(location = 0) out vec4 fragColor;\n"
        "layout(location = 0) in vec2 qt_TexCoord0;\n"
        "\n"
        "void main() {\n";
    if (alphaOnly)
        fragShader += "    float result = 0.0;\n";
    else
        fragShader += "    vec4 result = vec4(0);\n";
    fragShader += "    vec2 pixelStep = dirstep * spread;\n";
    if (masked)
        fragShader += "    pixelStep *= texture(mask, qt_TexCoord0).a;\n";

    float wSum = 0;
    for (int r=-requestedRadius; r<=requestedRadius; ++r) {
        float w = qgfx_gaussian(r, deviation);
        wSum += w;
        fragShader += "    result += float(";
        fragShader += QByteArray::number(w);
        fragShader += ") * texture(source, qt_TexCoord0 + pixelStep * float(";
        fragShader += QByteArray::number(r);
        fragShader += "))";
        if (alphaOnly)
            fragShader += ".a";
        fragShader += ";\n";
    }
    fragShader += "    const float wSum = float(";
    fragShader += QByteArray::number(wSum);
    fragShader += ");\n"
        "    fragColor = ";
    if (alphaOnly)
        fragShader += "mix(vec4(0), color, clamp((result / wSum) / thickness, 0.0, 1.0)) * qt_Opacity;\n";
    else
        fragShader += "(qt_Opacity / wSum) * result;\n";
    fragShader += "}\n";

    return fragShader;
}

QVariantMap QGfxShaderBuilder::gaussianBlur(const QJSValue &parameters)
{
    int requestedRadius = qMax(0.0, parameters.property(QStringLiteral("radius")).toNumber());
    qreal deviation = parameters.property(QStringLiteral("deviation")).toNumber();
    bool masked = parameters.property(QStringLiteral("masked")).toBool();
    bool alphaOnly = parameters.property(QStringLiteral("alphaOnly")).toBool();

    int requestedSamples = requestedRadius * 2 + 1;
    int samples = 1 + requestedSamples / 2;
    int radius = requestedSamples / 4;
    bool fallback = parameters.property(QStringLiteral("fallback")).toBool();

    QVariantMap result;

    QByteArray vertexShader;
    QByteArray fragmentShader;
    if (samples > m_maxBlurSamples || masked || fallback) {
        fragmentShader = qgfx_fallbackFragmentShader(requestedRadius, deviation, masked, alphaOnly);
        vertexShader = qgfx_fallbackVertexShader(alphaOnly);
    } else {
        QVarLengthArray<QGfxGaussSample, 64> p(samples);
        qgfx_buildGaussSamplePoints(p.data(), samples, radius, deviation);

        fragmentShader = qgfx_gaussianFragmentShader(p.data(), samples, alphaOnly);
        vertexShader = qgfx_gaussianVertexShader(p.data(), samples, alphaOnly);
    }

    result["fragmentShader"] = buildFragmentShader(fragmentShader);
    result["vertexShader"] = buildVertexShader(vertexShader);
    return result;
}

QUrl QGfxShaderBuilder::buildFragmentShader(const QByteArray &code)
{
    return buildShader(code, QShader::FragmentStage);
}

QUrl QGfxShaderBuilder::buildVertexShader(const QByteArray &code)
{
    return buildShader(code, QShader::VertexStage);
}

QUrl QGfxShaderBuilder::buildShader(const QByteArray &code,
                                    QShader::Stage stage)
{
    static bool recreateShaders = qEnvironmentVariableIntValue("QT_GFXSHADERBUILDER_REFRESH_CACHE");

    QCryptographicHash fileNameHash(QCryptographicHash::Sha1);
    fileNameHash.addData(code);

    QString path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                   + QStringLiteral("/_qt_QGfxShaderBuilder_")
                   + QStringLiteral(QT_VERSION_STR)
                   + QStringLiteral("/");
    QString filePath = path
                   + fileNameHash.result().toHex()
                   + QStringLiteral(".qsb");

    if (!QFile::exists(filePath) || recreateShaders) {
        if (!QDir().mkpath(path)) {
            qWarning() << "QGfxShaderBuilder: Failed to create path:" << path;
            return QUrl{};

        }

        QFile output(filePath);
        if (!output.open(QIODevice::WriteOnly)) {
            qWarning() << "QGfxShaderBuilder: Failed to store shader cache in file:" << filePath;
            return QUrl{};
        }

        m_shaderBaker.setSourceString(code, stage, filePath);
        {
            QShader compiledShader = m_shaderBaker.bake();
            if (!compiledShader.isValid()) {
                qWarning() << "QGfxShaderBuilder: Failed to compile shader for stage "
                           << stage << ": "
                           << m_shaderBaker.errorMessage()
                           << QString(code).replace('\n', QChar(QChar::LineFeed));
                return QUrl{};
            }
            output.write(compiledShader.serialized());
        }
    }

    return QUrl::fromLocalFile(filePath);
}

QT_END_NAMESPACE

#include "moc_qgfxshaderbuilder_p.cpp"
