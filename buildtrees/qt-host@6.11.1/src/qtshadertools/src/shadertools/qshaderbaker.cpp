// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser


#include "qshaderbaker.h"
#include "qspirvcompiler_p.h"
#include "qspirvshader_p.h"
#include <QFileInfo>
#include <QFile>
#include <QDebug>

QT_BEGIN_NAMESPACE

/*!
    \class QShaderBaker
    \inmodule QtShaderTools
    \since 6.6

    \brief Compiles a GLSL/Vulkan shader into SPIR-V, translates into other
    shading languages, and gathers reflection metadata.

    \warning QShaderBaker, just like the QRhi family of classes in the Qt Gui
    module, including QShader and QShaderDescription, offers limited
    compatibility guarantees. There are no source or binary compatibility
    guarantees for these classes, meaning the API is only guaranteed to work
    with the Qt version the application was developed against. Source
    incompatible changes are however aimed to be kept at a minimum and will only
    be made in minor releases (6.7, 6.8, and so on). To use this class in an
    application, link to \c{Qt::ShaderToolsPrivate} (if using CMake), and
    include the headers with the \c rhi prefix, for example
    \c{#include <rhi/qshaderbaker.h>}.

    QShaderBaker takes a graphics (vertex, fragment, etc.) or compute shader,
    and produces multiple - either source or bytecode - variants of it,
    together with reflection information. The results are represented by a
    QShader instance, which also provides simple and fast serialization
    and deserialization.

    \note Applications and libraries are recommended to avoid using this class
    directly. Rather, all Qt users are encouraged to rely on offline compilation
    by invoking the \c qsb command-line tool at build time via CMake. The \c qsb
    tool uses QShaderBaker and writes the serialized version of the generated
    QShader into a file. The usage of this class should be restricted to cases
    where run time compilation cannot be avoided, such as when working with
    user-provided or dynamically generated shader source strings.

    The input format is always assumed to be Vulkan-flavored GLSL at the
    moment. See the
    \l{https://github.com/KhronosGroup/GLSL/blob/master/extensions/khr/GL_KHR_vulkan_glsl.txt}{GL_KHR_vulkan_glsl
    specification} for an overview, keeping in mind that the Qt Shader Tools
    module is meant to be used in combination with the QRhi classes from Qt
    Rendering Hardware Interface module, and therefore a number of concepts and
    constructs (push constants, storage buffers, subpasses, etc.) are not
    applicable at the moment. Additional options may be introduced in the
    future, for example, by enabling
    \l{https://docs.microsoft.com/en-us/windows/desktop/direct3dhlsl/dx-graphics-hlsl}{HLSL}
    as a source format, once HLSL to SPIR-V compilation is deemed suitable.

    The reflection metadata is retrievable from the resulting QShader by
    calling QShader::description(). This is essential when having to
    discover what set of vertex inputs and shader resources a shader expects,
    and what the layouts of those are, as many modern graphics APIs offer no
    built-in shader reflection capabilities.

    \section2 Typical Workflow

    Let's assume an application has a vertex and fragment shader like the following:

    Vertex shader:
    \snippet color.vert 0

    Fragment shader:
    \snippet color.frag 0

    To get QShader instances that can be passed as-is to a
    QRhiGraphicsPipeline, there are two options: doing the shader pack
    generation off line, or at run time.

    The former involves running the \c qsb tool:

    \badcode
    qsb --glsl "100 es,120" --hlsl 50 --msl 12 color.vert -o color.vert.qsb
    qsb --glsl "100 es,120" --hlsl 50 --msl 12 color.frag -o color.frag.qsb
    \endcode

    The example uses the translation targets as appropriate for QRhi. This
    means GLSL/ES 100, GLSL 120, HLSL Shader Model 5.0, and Metal Shading
    Language 1.2.

    Note how the command line options correspond to what can be specified via
    setGeneratedShaders(). Once the resulting files are available, they can be
    shipped with the application (typically embedded into the executable the
    the Qt Resource System), and can be loaded and passed to
    QShader::fromSerialized() at run time.

    While not shown here, \c qsb can do more: it is also able to invoke \c fxc
    on Windows or the appropriate XCode tools on macOS to compile the generated
    HLSL or Metal shader code into bytecode and include the compiled versions
    in the QShader. After a baked shader pack is written into a file, its
    contents can be examined by running \c{qsb -d} on it. Run \c qsb with
    \c{--help} for more information.

    The alternative approach is to perform the same at run time. This involves
    creating a QShaderBaker instance, calling setSourceFileName(), and then
    setting up the translation targets via setGeneratedShaders():

    \badcode
        baker.setGeneratedShaderVariants({ QShader::StandardShader });
        QList<QShaderBaker::GeneratedShader> targets;
        targets.append({ QShader::SpirvShader, QShaderVersion(100) });
        targets.append({ QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs) });
        targets.append({ QShader::SpirvShader, QShaderVersion(120) });
        targets.append({ QShader::HlslShader, QShaderVersion(50) });
        targets.append({ QShader::MslShader, QShaderVersion(12) });
        baker.setGeneratedShaders(targets);
        QShader shaders = baker.bake();
        if (!shaders.isValid())
            qWarning() << baker.errorMessage();
    \endcode

    \sa QShader
 */

struct QShaderBakerPrivate
{
    bool readFile(const QString &fn);
    std::pair<QByteArray, QByteArray> compile();
    QByteArray perTargetDefines(const QShaderBaker::GeneratedShader &key);

    QString sourceFileName;
    QByteArray source;
    QShader::Stage stage;
    QList<QShaderBaker::GeneratedShader> reqVersions;
    QList<QShader::Variant> variants;
    QByteArray preamble;
    int batchLoc = 7;
    bool perTargetEnabled = false;
    bool breakOnShaderTranslationError = true;
    QSpirvShader::TessellationInfo tessInfo;
    QSpirvShader::MultiViewInfo multiViewInfo;
    QShaderBaker::SpirvOptions spirvOptions;
    QShaderBaker::GlslOptions glslOptions;
    QSpirvCompiler compiler;
    QString errorMessage;
};

bool QShaderBakerPrivate::readFile(const QString &fn)
{
    QFile f(fn);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("QShaderBaker: Failed to open %s", qPrintable(fn));
        return false;
    }
    source = f.readAll();

    // Remove UTF-8 BOM, if present. glslang fails otherwise, since the BOM is
    // removed in glslangValidator (the command-line frontend), not in the
    // compiler itself. Replicate that here.
    if (source.size() >= 3) {
        const unsigned char *p = reinterpret_cast<const unsigned char *>(source.constData());
        if (p[0] == 0xEF && p[1] == 0xBB && p[2] == 0xBF)
            source.remove(0, 3);
    }

    sourceFileName = fn;
    return true;
}

/*!
    Constructs a new QShaderBaker.
 */
QShaderBaker::QShaderBaker()
    : d(new QShaderBakerPrivate)
{
}

/*!
    Destructor.
 */
QShaderBaker::~QShaderBaker()
{
    delete d;
}

/*!
    Sets the name of the shader source file to \a fileName. This is the file
    that will be read when calling bake(). The shader stage is deduced
    automatically from the file extension. When this is not desired or not
    possible, use the overload with the stage argument instead.

    The supported file extensions are:
    \list
    \li \c{.vert} - vertex shader
    \li \c{.frag} - fragment (pixel) shader
    \li \c{.tesc} - tessellation control (hull) shader
    \li \c{.tese} - tessellation evaluation (domain) shader
    \li \c{.geom} - geometry shader
    \li \c{.comp} - compute shader
    \endlist

    \warning \a fileName is expected to contain trusted content. Application
    developers are advised to carefully consider the potential implications
    before passing in user-provided source files that are not part of the
    application.
 */
void QShaderBaker::setSourceFileName(const QString &fileName)
{
    if (!d->readFile(fileName))
        return;

    const QString suffix = QFileInfo(fileName).suffix();
    if (suffix == QStringLiteral("vert")) {
        d->stage = QShader::VertexStage;
    } else if (suffix == QStringLiteral("frag")) {
        d->stage = QShader::FragmentStage;
    } else if (suffix == QStringLiteral("tesc")) {
        d->stage = QShader::TessellationControlStage;
    } else if (suffix == QStringLiteral("tese")) {
        d->stage = QShader::TessellationEvaluationStage;
    } else if (suffix == QStringLiteral("geom")) {
        d->stage = QShader::GeometryStage;
    } else if (suffix == QStringLiteral("comp")) {
        d->stage = QShader::ComputeStage;
    } else {
        qWarning("QShaderBaker: Unknown shader stage, defaulting to vertex");
        d->stage = QShader::VertexStage;
    }
}

/*!
    Sets the name of the shader source file to \a fileName. This is the file
    that will be read when calling bake(). The shader stage is specified by \a
    stage.

    \warning \a fileName is expected to contain trusted content. Application
    developers are advised to carefully consider the potential implications
    before passing in user-provided source files that are not part of the
    application.
 */
void QShaderBaker::setSourceFileName(const QString &fileName, QShader::Stage stage)
{
    if (d->readFile(fileName))
        d->stage = stage;
}

/*!
    Sets the source \a device. This allows using any QIODevice instead of just
    files. \a stage specifies the shader stage, while the optional \a fileName
    contains a filename that is used in the error messages.

    \warning \a device is expected to contain trusted content. Application
    developers are advised to carefully consider the potential implications
    before passing in user-provided data from sources that are not under the
    application's control.
 */
void QShaderBaker::setSourceDevice(QIODevice *device, QShader::Stage stage, const QString &fileName)
{
    setSourceString(device->readAll(), stage, fileName);
}

/*!
    Sets the input shader \a sourceString. \a stage specified the shader stage,
    while the optional \a fileName contains a filename that is used in the
    error messages.

    \warning \a sourceString is expected to contain trusted content. Application
    developers are advised to carefully consider the potential implications
    before passing in user-provided data from sources that are not under the
    application's control.
 */
void QShaderBaker::setSourceString(const QByteArray &sourceString, QShader::Stage stage, const QString &fileName)
{
    d->sourceFileName = fileName; // for error messages, include handling, etc.
    d->source = sourceString;
    d->stage = stage;
}

/*!
    \typedef QShaderBaker::GeneratedShader

    Synonym for std::pair<QShader::Source, QShaderVersion>.
*/

/*!
    Specifies what kind of shaders to compile or translate to. Nothing is
    generated by default so calling this function before bake() is mandatory

    \note when this function is not called or \a v is empty or contains only invalid
    entries, the resulting QShader will be empty and thus invalid.

    For example, the minimal possible baking target is SPIR-V, without any
    additional translations to other languages. To request this, do:

    \badcode
        baker.setGeneratedShaders({ QShader::SpirvShader, QShaderVersion(100) });
    \endcode

    \note QShaderBaker only handles the SPIR-V and human-readable source
    targets. Further compilation into API-specific intermediate formats, such
    as QShader::DxbcShader or QShader::MetalLibShader is implemented by the
    \c qsb command-line tool, and is not part of the QShaderBaker runtime API.
 */
void QShaderBaker::setGeneratedShaders(const QList<GeneratedShader> &v)
{
    d->reqVersions = v;
}

/*!
    Specifies which shader variants are generated. Each shader version can have
    multiple variants in the resulting QShader.

    In most cases \a v contains a single entry, QShader::StandardShader.

    \note when no variants are set, the resulting QShader will be empty and
    thus invalid.
 */
void QShaderBaker::setGeneratedShaderVariants(const QList<QShader::Variant> &v)
{
    d->variants = v;
}

/*!
    Specifies a custom \a preamble that is processed before the normal shader
    code.

    This is more than just prepending to the source string: the validity of the
    GLSL version directive, which is required to be placed before everything
    else, is not affected. Line numbers in the reported error messages also
    remain unchanged, ignoring the contents given in the \a preamble.

    One use case for preambles is to transparently insert dynamically generated
    \c{#define} statements.
 */
void QShaderBaker::setPreamble(const QByteArray &preamble)
{
    d->preamble = preamble;
}

/*!
    When generating a QShader::BatchableVertexShader variant, \a location
    specifies the input location for the inserted vertex input. The value is by
    default 7 and needs to be overridden only if the vertex shader already uses
    input location 7.
 */
void QShaderBaker::setBatchableVertexShaderExtraInputLocation(int location)
{
    d->batchLoc = location;
}

/*!
    Sets per-target compilation to \a enable. By default this is disabled,
    meaning that the Vulkan/GLSL source is compiled to SPIR-V once per variant.
    (so once by default, twice if it is a vertex shader and the Batchable
    variant as requested as well). The resulting SPIR-V is then translated to
    the various target languages (GLSL, HLSL, MSL).

    In per-target compilation mode, there is a separate GLSL to SPIR-V
    compilation step for each target, meaning for each GLSL/HLSL/MSL version
    requested via setGeneratedShaders(). The input source is the same, but with
    target-specific preprocessor defines inserted. This is significantly more
    time consuming, but allows applications to provide a single shader and use
    \c{#ifdef} blocks to differentiate. When this mode is disabled, the only
    way to achieve the same is to provide multiple versions of the shader file,
    process each separately, ship {.qsb} files for each, and choose the right
    file based on run time logic.

    The following macros will be automatically defined in this mode. Note that
    the macros are always tied to shading languages, not graphics APIs.

    \list

    \li \c{QSHADER_SPIRV} - defined when targeting SPIR-V (to be consumed,
    typically, by Vulkan).

    \li \c{QSHADER_SPIRV_VERSION} - the targeted SPIR-V version number, such as
    \c 100.

    \li \c{QSHADER_GLSL} - defined when targeting GLSL or GLSL ES (to be
    consumed, typically, by OpenGL or OpenGL ES)

    \li \c{QSHADER_GLSL_VERSION} - the targeted GLSL or GLSL ES version number,
    such as \c 100, \c 300, or \c 330.

    \li \c{QSHADER_GLSL_ES} - defined only when targeting GLSL ES

    \li \c{QSHADER_HLSL} - defined when targeting HLSL (to be consumed,
    typically, by Direct 3D)

    \li \c{QSHADER_HLSL_VERSION} - the targeted HLSL shader model version, such
    as \c 50

    \li \c{QSHADER_MSL} - defined when targeting the Metal Shading Language (to
    be consumed, typically, by Metal)

    \li \c{QSHADER_MSL_VERSION} - the targeted MSL version, such as \c 12 or
    \c 20.

    \endlist

    This allows writing shader code like the following.

    \badcode
      #if QSHADER_HLSL || QSHADER_MSL
      vec2 uv = vec2(uv_coord.x, 1.0 - uv_coord.y);
      #else
      vec2 uv = uv_coord;
      #endif
    \endcode

    \note Version numbers follow the GLSL-inspired QShaderVersion syntax and
    thus are a single integer always.

    \note There is only one QShaderDescription per QShader, no matter how many
    individual targets there are. Therefore members of uniform blocks, vertex
    inputs, etc. must not be made conditional using the macros described above.

    \warning Be aware of the differences between the concepts of graphics APIs
    and shading languages. QShaderBaker and the related tools work strictly
    with the concept of shading languages, ignoring how the results are
    consumed afterwards. Therefore, if the higher layers in the Qt graphics
    stack one day start using SPIR-V also for an API other than Vulkan, the
    assumption that QSHADER_SPIRV implies Vulkan will no longer hold.
 */
void QShaderBaker::setPerTargetCompilation(bool enable)
{
    d->perTargetEnabled = enable;
}

/*!
    Controls the behavior when shader translation (from SPIR-V to
    GLSL/HLSL/MSL) fails. By default this setting is true, which will cause
    bake() to return with an error if a requested shader cannot be generated.
    If that is not desired, and the intention is to generate what we can but
    silently skip the rest, then set \a enable to false.

    Targeting multiple GLSL versions can lead to errors when a feature is not
    translatable to a given version. For example, attempting to translate a
    shader using textureSize() to GLSL ES 100 would fail the entire bake() call
    with the error message "textureSize is not supported in ESSL 100". If it is
    acceptable to not have a GLSL ES 100 shader in the result, even though it
    was requested, then setting this flag to false makes bake() to succeed.
 */
void QShaderBaker::setBreakOnShaderTranslationError(bool enable)
{
    d->breakOnShaderTranslationError = enable;
}

/*!
    When generating MSL shader code for a tessellation control shader, the
    tessellation \a mode (triangles or quads) must be known upfront. In GLSL
    this is declared in the tessellation evaluation shader typically, but for
    Metal it must be known also when generating the compute shader from the
    tessellation control shader.

    When not set, the default is triangles.
 */
void QShaderBaker::setTessellationMode(QShaderDescription::TessellationMode mode)
{
    d->tessInfo.infoForTesc.mode = mode;
}

/*!
    When generating MSL shader code for a tessellation evaluation shader, the
    output vertex \a count of the tessellation control shader must be known
    upfront. in GLSL this would be declared in the tessellation control shader
    typically, but for Metal it must be known also when generating the vertex
    shader from the teselation evaluation shader.

    When not set, the default value is 3.
 */
void QShaderBaker::setTessellationOutputVertexCount(int count)
{
    d->tessInfo.infoForTese.vertexCount = count;
}

/*!
    When transpiling shaders using multiview (e.g. a vertex shader using
    gl_ViewIndex for a renderer relying on GL_OVR_multiview2, VK_KHR_multiview,
    etc.), for some of the targets it is necessary to declare the number of
    views in the shader. This is not done in the Vulkan-style GLSL code, and is
    not relevant for targets such as SPIR-V or HLSL, but is required for OpenGL
    and GLSL, and so the value has to be provided as additional metadata.

    By default the value is 0, which disables injecting the \c{num_views}
    statement. Setting 1 is not useful since that is the default \c{num_views}
    regardless. Therefore \a count should be >= 2 to make an effect. When set
    to, for example, 2, the generated GLSL shader will contain a
    \c{layout(num_views = 2) in;} statement.

    Setting a \a count of 2 or greater also injects some preprocessor
    statements:
    \c{QSHADER_VIEW_COUNT} is set to \a count, whereas the
    \c GL_EXT_multiview extension is enabled automatically. Therefore, setting
    the appropriate
    \a count can be relevant with other types of shaders as well, e.g. when
    sharing a uniform buffer between the vertex and fragment shader and both
    shaders have to be able to write something like
    \c{#if QSHADER_VIEW_COUNT >= 2}.

    \since 6.7
 */
void QShaderBaker::setMultiViewCount(int count)
{
    d->multiViewInfo.viewCount = count >= 2 ? count : 0;
}

/*!
    \enum QShaderBaker::SpirvOption
    \value GenerateFullDebugInfo Generate and store additional debug information in the SPIR-V binary.
    \value StripDebugAndVarInfo Strip all debug and variable name information from the SPIR-V binary.
 */

/*!
    Sets additional \a options for the generated SPIR-V binary.
    By default no flags are set.
 */
void QShaderBaker::setSpirvOptions(SpirvOptions options)
{
    d->spirvOptions = options;
}

/*!
    \enum QShaderBaker::GlslOption
    \value GlslEsFragDefaultFloatPrecisionMedium Emit \c{precision mediump float;} in fragment shaders for GLSL ES.
 */

/*!
    Sets additional \a options for the generated GLSL and GLSL ES sources.
    By default no flags are set.
    \since 6.9
 */
void QShaderBaker::setGlslOptions(GlslOptions options)
{
    d->glslOptions = options;
}

#ifndef Q_OS_INTEGRITY // QTBUG-145990
inline size_t qHash(const QShaderBaker::GeneratedShader &k, size_t seed = 0)
{
    return qHash(k.first, seed) ^ k.second.version();
}
#endif

std::pair<QByteArray, QByteArray> QShaderBakerPrivate::compile()
{
    QSpirvCompiler::Flags flags;
    if (spirvOptions.testFlag(QShaderBaker::SpirvOption::GenerateFullDebugInfo))
        flags |= QSpirvCompiler::FullDebugInfo;

    compiler.setFlags(flags);
    const QByteArray spirvBin = compiler.compileToSpirv();
    if (spirvBin.isEmpty()) {
        errorMessage = compiler.errorMessage();
        return {};
    }
    QByteArray batchableSpirvBin;
    if (stage == QShader::VertexStage && variants.contains(QShader::BatchableVertexShader)) {
        compiler.setFlags(flags | QSpirvCompiler::RewriteToMakeBatchableForSG);
        compiler.setSGBatchingVertexInputLocation(batchLoc);
        batchableSpirvBin = compiler.compileToSpirv();
        if (batchableSpirvBin.isEmpty()) {
            errorMessage = compiler.errorMessage();
            return {};
        }
    }
    return { spirvBin, batchableSpirvBin };
}

QByteArray QShaderBakerPrivate::perTargetDefines(const QShaderBaker::GeneratedShader &key)
{
    QByteArray preamble;
    switch (key.first) {
    case QShader::SpirvShader:
        preamble += QByteArrayLiteral("\n#define QSHADER_SPIRV 1\n#define QSHADER_SPIRV_VERSION ");
        preamble += QByteArray::number(key.second.version());
        preamble += QByteArrayLiteral("\n");
        break;
    case QShader::GlslShader:
        preamble += QByteArrayLiteral("\n#define QSHADER_GLSL 1\n#define QSHADER_GLSL_VERSION ");
        preamble += QByteArray::number(key.second.version());
        if (key.second.flags().testFlag(QShaderVersion::GlslEs))
            preamble += QByteArrayLiteral("\n#define QSHADER_GLSL_ES 1");
        preamble += QByteArrayLiteral("\n");
        break;
    case QShader::HlslShader:
        preamble += QByteArrayLiteral("\n#define QSHADER_HLSL 1\n#define QSHADER_HLSL_VERSION ");
        preamble += QByteArray::number(key.second.version());
        preamble += QByteArrayLiteral("\n");
        break;
    case QShader::MslShader:
        preamble += QByteArrayLiteral("\n#define QSHADER_MSL 1\n#define QSHADER_MSL_VERSION ");
        preamble += QByteArray::number(key.second.version());
        preamble += QByteArrayLiteral("\n");
        break;
    default:
        Q_UNREACHABLE();
    }
    return preamble;
}

/*!
    Runs the compilation and translation process.

    \return a QShader instance. To check if the process was successful,
    call QShader::isValid(). When that indicates \c false, call
    errorMessage() to retrieve the log.

    This is an expensive operation. When calling this from applications, it can
    be advisable to do it on a separate thread.

    \note QShaderBaker instances are reusable: after calling bake(), the same
    instance can be used with different inputs again. However, a QShaderBaker
    instance should only be used on one single thread during its lifetime.
 */
QShader QShaderBaker::bake()
{
    d->errorMessage.clear();

    if (d->source.isEmpty()) {
        d->errorMessage = QLatin1String("QShaderBaker: No source specified");
        return QShader();
    }

    d->compiler.setSourceString(d->source, d->stage, d->sourceFileName);

    // Normally one entry, for QShader::SpirvShader only. However, in
    // compile-per-target mode there is a separate SPIR-V binary generated for
    // each target (so for each GLSL/HLSL/MSL version requested).
    QHash<GeneratedShader, QByteArray> spirv;
    QHash<GeneratedShader, QByteArray> batchableSpirv;
    const auto compileSpirvAndBatchable = [this, &spirv, &batchableSpirv](const GeneratedShader &key) {
        const std::pair<QByteArray, QByteArray> bin = d->compile();
        if (bin.first.isEmpty())
            return false;
        spirv.insert(key, bin.first);
        if (!bin.second.isEmpty())
            batchableSpirv.insert(key, bin.second);
        return true;
    };

    QByteArray preamble = d->preamble;
    if (d->multiViewInfo.viewCount >= 2) {
        if (d->stage == QShader::VertexStage)
            preamble += QByteArrayLiteral("\n#extension GL_EXT_multiview : require\n");
        preamble += QByteArrayLiteral("\n#define QSHADER_VIEW_COUNT ");
        preamble += QByteArray::number(d->multiViewInfo.viewCount);
        preamble += QByteArrayLiteral("\n");
    }

    if (!d->perTargetEnabled) {
        d->compiler.setPreamble(preamble);
        if (!compileSpirvAndBatchable({ QShader::SpirvShader, {} }))
            return QShader();
    } else {
        // per-target compilation. the value here comes from the varying
        // preamble (and so preprocessor defines)
        for (GeneratedShader req: d->reqVersions) {
            d->compiler.setPreamble(preamble + d->perTargetDefines(req));
            if (!compileSpirvAndBatchable(req))
                return QShader();
        }
    }

    // Now spirv, and, if in use, batchableSpirv, contain at least one,
    // optionally more SPIR-V binaries.
    Q_ASSERT(!spirv.isEmpty() && (d->perTargetEnabled || spirv.size() == 1));

    QShader bs;
    bs.setStage(d->stage);

    QSpirvShader spirvShader;
    QSpirvShader batchableSpirvShader;
    // The QShaderDescription can be different for variants (we just have a
    // hardcoded rule to pick one), but cannot differ for targets (in
    // per-target mode, hence we can just pick the first SPIR-V binary and
    // generate the reflection data based on that)
    spirvShader.setSpirvBinary(spirv.constKeyValueBegin()->second, d->stage);
    if (batchableSpirv.isEmpty()) {
        bs.setDescription(spirvShader.shaderDescription());
    } else {
        batchableSpirvShader.setSpirvBinary(batchableSpirv.constKeyValueBegin()->second, d->stage);
        // prefer the batchable's reflection info with _qt_order and such present
        bs.setDescription(batchableSpirvShader.shaderDescription());
    }

    for (const GeneratedShader &req: d->reqVersions) {
        for (const QShader::Variant &v : d->variants) {
            if (d->stage != QShader::VertexStage) {
                if (v == QShader::BatchableVertexShader
                        || v == QShader::UInt32IndexedVertexAsComputeShader
                        || v == QShader::UInt16IndexedVertexAsComputeShader
                        || v == QShader::NonIndexedVertexAsComputeShader)
                {
                    continue;
                }
            }
            if (req.first != QShader::MslShader && req.first != QShader::MetalLibShader) {
                if (v == QShader::UInt32IndexedVertexAsComputeShader
                        || v == QShader::UInt16IndexedVertexAsComputeShader
                        || v == QShader::NonIndexedVertexAsComputeShader)
                {
                    continue;
                }
            }

            QSpirvShader *currentSpirvShader = nullptr;
            if (d->perTargetEnabled) {
                // This is expensive too, in addition to the multiple
                // compilation rounds, but opting in to per-target mode is a
                // careful, conscious choice (hopefully), so it's fine.
                if (v == QShader::BatchableVertexShader)
                    batchableSpirvShader.setSpirvBinary(batchableSpirv[req], d->stage);
                else
                    spirvShader.setSpirvBinary(spirv[req], d->stage);
            }
            if (v == QShader::BatchableVertexShader)
                currentSpirvShader = &batchableSpirvShader;
            else
                currentSpirvShader = &spirvShader;
            Q_ASSERT(currentSpirvShader);
            Q_ASSERT(!currentSpirvShader->spirvBinary().isEmpty());
            const QShaderKey key(req.first, req.second, v);
            QShaderCode shader;
            shader.setEntryPoint(QByteArrayLiteral("main"));
            switch (req.first) {
            case QShader::SpirvShader:
                if (d->spirvOptions.testFlag(QShaderBaker::SpirvOption::StripDebugAndVarInfo)) {
                    QString errorMsg;
                    const QByteArray strippedSpirv = currentSpirvShader->remappedSpirvBinary(QSpirvShader::RemapFlag::StripOnly, &errorMsg);
                    if (strippedSpirv.isEmpty()) {
                        d->errorMessage = errorMsg;
                        return QShader();
                    }
                    shader.setShader(strippedSpirv);
                } else {
                    shader.setShader(currentSpirvShader->spirvBinary());
                }
                break;
            case QShader::GlslShader:
            {
                QSpirvShader::GlslFlags flags;
                if (req.second.flags().testFlag(QShaderVersion::GlslEs)) {
                    flags |= QSpirvShader::GlslFlag::GlslEs;
                    if (d->glslOptions.testFlag(GlslOption::GlslEsFragDefaultFloatPrecisionMedium))
                        flags |= QSpirvShader::GlslFlag::EsFragDefaultFloatPrecisionMedium;
                }
                QVector<QSpirvShader::SeparateToCombinedImageSamplerMapping> separateToCombinedImageSamplerMappings;
                shader.setShader(currentSpirvShader->translateToGLSL(req.second.version(),
                                                                     flags,
                                                                     d->stage,
                                                                     d->multiViewInfo,
                                                                     &separateToCombinedImageSamplerMappings));
                if (shader.shader().isEmpty()) {
                    if (d->breakOnShaderTranslationError) {
                        d->errorMessage = currentSpirvShader->translationErrorMessage();
                        return QShader();
                    } else {
                        d->errorMessage += QLatin1String(" ") + currentSpirvShader->translationErrorMessage();
                        continue;
                    }
                }
                if (!separateToCombinedImageSamplerMappings.isEmpty()) {
                    const QShaderDescription desc = bs.description();
                    QVector<QShaderDescription::InOutVariable> separateImages = desc.separateImages();
                    QVector<QShaderDescription::InOutVariable> separateSamplers = desc.separateSamplers();
                    QShader::SeparateToCombinedImageSamplerMappingList result;
                    for (const QSpirvShader::SeparateToCombinedImageSamplerMapping &mapping : separateToCombinedImageSamplerMappings) {
                        int textureBinding = -1;
                        int samplerBinding = -1;
                        for (int i = 0, count = separateImages.size(); i < count; ++i) {
                            if (separateImages[i].name == mapping.textureName) {
                                textureBinding = separateImages[i].binding;
                                break;
                            }
                        }
                        for (int i = 0, count = separateSamplers.size(); i < count; ++i) {
                            if (separateSamplers[i].name == mapping.samplerName) {
                                samplerBinding = separateSamplers[i].binding;
                                break;
                            }
                        }
                        result.append({ mapping.combinedSamplerName, textureBinding, samplerBinding });
                    }
                    bs.setSeparateToCombinedImageSamplerMappingList(key, result);
                }
            }
                break;
            case QShader::HlslShader:
            {
                QShader::NativeResourceBindingMap nativeBindings;
                shader.setShader(currentSpirvShader->translateToHLSL(req.second.version(), &nativeBindings));
                if (shader.shader().isEmpty()) {
                    if (d->breakOnShaderTranslationError) {
                        d->errorMessage = currentSpirvShader->translationErrorMessage();
                        return QShader();
                    } else {
                        d->errorMessage += QLatin1String(" ") + currentSpirvShader->translationErrorMessage();
                        continue;
                    }
                }
                bs.setResourceBindingMap(key, nativeBindings);
            }
                break;
            case QShader::MslShader:
            {
                QShader::NativeResourceBindingMap nativeBindings;
                QShader::NativeShaderInfo shaderInfo;
                QSpirvShader::MslFlags flags;
                if (d->stage == QShader::VertexStage) {
                    switch (v) {
                    case QShader::UInt16IndexedVertexAsComputeShader:
                        flags |= QSpirvShader::MslFlag::VertexAsCompute;
                        flags |= QSpirvShader::MslFlag::WithUInt16Index;
                        break;
                    case QShader::UInt32IndexedVertexAsComputeShader:
                        flags |= QSpirvShader::MslFlag::VertexAsCompute;
                        flags |= QSpirvShader::MslFlag::WithUInt32Index;
                        break;
                    case QShader::NonIndexedVertexAsComputeShader:
                        flags |= QSpirvShader::MslFlag::VertexAsCompute;
                        break;
                    default:
                        break;
                    }
                }
                shader.setShader(currentSpirvShader->translateToMSL(req.second.version(),
                                                                    flags,
                                                                    d->stage,
                                                                    &nativeBindings,
                                                                    &shaderInfo,
                                                                    d->multiViewInfo,
                                                                    d->tessInfo));
                if (shader.shader().isEmpty()) {
                    if (d->breakOnShaderTranslationError) {
                        d->errorMessage = currentSpirvShader->translationErrorMessage();
                        return QShader();
                    } else {
                        d->errorMessage += QLatin1String(" ") + currentSpirvShader->translationErrorMessage();
                        continue;
                    }
                }
                shader.setEntryPoint(QByteArrayLiteral("main0"));
                bs.setResourceBindingMap(key, nativeBindings);
                bs.setNativeShaderInfo(key, shaderInfo);
            }
                break;
            default:
                Q_UNREACHABLE();
            }
            bs.setShader(key, shader);
        }
    }

    return bs;
}

/*!
    \return the error message from the last bake() run, or an empty string if
    there was no error.

    \note Errors include file read errors, compilation, and translation
    failures. Not requesting any targets or variants does not count as an error
    even though the resulting QShader is invalid.
 */
QString QShaderBaker::errorMessage() const
{
    return d->errorMessage;
}

QT_END_NAMESPACE
