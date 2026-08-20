// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtCore/qcoreapplication.h>
#include <QtCore/qcommandlineparser.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qfile.h>
#include <QtCore/qdir.h>
#include <QtCore/qset.h>
#include <QtCore/qtemporarydir.h>
#include <QtCore/qdebug.h>
#include <QtCore/qlibraryinfo.h>
#include <QtGui/private/qshader_p.h>
#include <rhi/qshaderbaker.h>

#if QT_CONFIG(process)
#include <QtCore/qprocess.h>
#endif

#include <cstdarg>
#include <cstdio>

// All qDebug must be guarded by !silent. For qWarnings, only the most
// fatal ones should be unconditional; warnings from external tool
// invocations must be guarded with !silent.
static bool silent = false;

using namespace Qt::StringLiterals;

enum class FileType
{
    Binary,
    Text
};

static void printError(const char *msg, ...)
{
    va_list arglist;
    va_start(arglist, msg);
    vfprintf(stderr, msg, arglist);
    fputs("\n", stderr);
    va_end(arglist);
}

static bool writeToFile(const QByteArray &buf, const QString &filename, FileType fileType)
{
    QDir().mkpath(QFileInfo(filename).path());
    QFile f(filename);
    QIODevice::OpenMode flags = QIODevice::WriteOnly | QIODevice::Truncate;
    if (fileType == FileType::Text)
        flags |= QIODevice::Text;
    if (!f.open(flags)) {
        printError("Failed to open %s for writing", qPrintable(filename));
        return false;
    }
    f.write(buf);
    return true;
}

static QByteArray readFile(const QString &filename, FileType fileType)
{
    QFile f(filename);
    QIODevice::OpenMode flags = QIODevice::ReadOnly;
    if (fileType == FileType::Text)
        flags |= QIODevice::Text;
    if (!f.open(flags)) {
        printError("Failed to open %s", qPrintable(filename));
        return QByteArray();
    }
    return f.readAll();
}

static QString writeTemp(const QTemporaryDir &tempDir, const QString &filename, const QShaderCode &s, FileType fileType)
{
    const QString fullPath = tempDir.path() + QLatin1String("/") + filename;
    if (writeToFile(s.shader(), fullPath, fileType))
        return fullPath;
    else
        return QString();
}

static bool runProcess(const QString &binary, const QStringList &arguments,
                       QByteArray *output, QByteArray *errorOutput)
{
#if QT_CONFIG(process)
    QProcess p;
    p.start(binary, arguments);
    const QString cmd = binary + QLatin1Char(' ') + arguments.join(QLatin1Char(' '));
    if (!silent)
        qDebug("%s", qPrintable(cmd));
    if (!p.waitForStarted()) {
        if (!silent)
            printError("Failed to run %s: %s", qPrintable(cmd), qPrintable(p.errorString()));
        return false;
    }
    if (!p.waitForFinished()) {
        if (!silent)
            printError("%s timed out", qPrintable(cmd));
        return false;
    }

    if (p.exitStatus() == QProcess::CrashExit) {
        if (!silent)
            printError("%s crashed", qPrintable(cmd));
        return false;
    }

    *output = p.readAllStandardOutput();
    *errorOutput = p.readAllStandardError();

    if (p.exitCode() != 0) {
        if (!silent)
            printError("%s returned non-zero error code %d", qPrintable(cmd), p.exitCode());
        return false;
    }

    return true;
#else
    Q_UNUSED(binary);
    Q_UNUSED(arguments);
    Q_UNUSED(output);
    *errorOutput = QByteArrayLiteral("QProcess not supported on this platform");
    return false;
#endif
}

static QString stageStr(QShader::Stage stage)
{
    switch (stage) {
    case QShader::VertexStage:
        return QStringLiteral("Vertex");
    case QShader::TessellationControlStage:
        return QStringLiteral("TessellationControl");
    case QShader::TessellationEvaluationStage:
        return QStringLiteral("TessellationEvaluation");
    case QShader::GeometryStage:
        return QStringLiteral("Geometry");
    case QShader::FragmentStage:
        return QStringLiteral("Fragment");
    case QShader::ComputeStage:
        return QStringLiteral("Compute");
    default:
        Q_UNREACHABLE();
    }
}

static QString sourceStr(QShader::Source source)
{
    switch (source) {
    case QShader::SpirvShader:
        return QStringLiteral("SPIR-V");
    case QShader::GlslShader:
        return QStringLiteral("GLSL");
    case QShader::HlslShader:
        return QStringLiteral("HLSL");
    case QShader::DxbcShader:
        return QStringLiteral("DXBC");
    case QShader::MslShader:
        return QStringLiteral("MSL");
    case QShader::DxilShader:
        return QStringLiteral("DXIL");
    case QShader::MetalLibShader:
        return QStringLiteral("metallib");
    case QShader::WgslShader:
        return QStringLiteral("WGSL");
    default:
        Q_UNREACHABLE();
    }
}

static QString sourceVersionStr(const QShaderVersion &v)
{
    QString s = v.version() ? QString::number(v.version()) : QString();
    if (v.flags().testFlag(QShaderVersion::GlslEs))
        s += QLatin1String(" es");

    return s;
}

static QString sourceVariantStr(const QShader::Variant &v)
{
    switch (v) {
    case QShader::StandardShader:
        return QLatin1String("Standard");
    case QShader::BatchableVertexShader:
        return QLatin1String("Batchable");
    case QShader::UInt32IndexedVertexAsComputeShader:
        return QLatin1String("UInt32IndexedVertexAsCompute");
    case QShader::UInt16IndexedVertexAsComputeShader:
        return QLatin1String("UInt16IndexedVertexAsCompute");
    case QShader::NonIndexedVertexAsComputeShader:
        return QLatin1String("NonIndexedVertexAsCompute");
    default:
        Q_UNREACHABLE();
    }
}

static void dump(const QShader &bs)
{
    QTextStream ts(stdout);
    ts << "Stage: " << stageStr(bs.stage()) << "\n";
    ts << "QSB_VERSION: " << QShaderPrivate::get(&bs)->qsbVersion << "\n";
    const QList<QShaderKey> keys = bs.availableShaders();
    ts << "Has " << keys.size() << " shaders:\n";
    for (int i = 0; i < keys.size(); ++i) {
        ts << "  Shader " << i << ": " << sourceStr(keys[i].source())
            << " " << sourceVersionStr(keys[i].sourceVersion())
            << " [" << sourceVariantStr(keys[i].sourceVariant()) << "]\n";
    }
    ts << "\n";
    ts << "Reflection info: " << bs.description().toJson() << "\n\n";
    for (int i = 0; i < keys.size(); ++i) {
        ts << "Shader " << i << ": " << sourceStr(keys[i].source())
            << " " << sourceVersionStr(keys[i].sourceVersion())
            << " [" << sourceVariantStr(keys[i].sourceVariant()) << "]\n";
        QShaderCode shader = bs.shader(keys[i]);
        if (!shader.entryPoint().isEmpty())
            ts << "Entry point: " << shader.entryPoint() << "\n";
        QShader::NativeResourceBindingMap nativeResMap = bs.nativeResourceBindingMap(keys[i]);
        if (!nativeResMap.isEmpty()) {
            ts << "Native resource binding map:\n";
            for (auto mapIt = nativeResMap.cbegin(), mapItEnd = nativeResMap.cend(); mapIt != mapItEnd; ++mapIt)
                ts << mapIt.key() << " -> [" << mapIt.value().first << ", " << mapIt.value().second << "]\n";
        }
        QShader::SeparateToCombinedImageSamplerMappingList samplerMapList = bs.separateToCombinedImageSamplerMappingList(keys[i]);
        if (!samplerMapList.isEmpty()) {
            ts << "Mapping table for auto-generated combined image samplers:\n";
            for (auto listIt = samplerMapList.cbegin(), listItEnd = samplerMapList.cend(); listIt != listItEnd; ++listIt)
                ts << "\"" << listIt->combinedSamplerName << "\" -> [" << listIt->textureBinding << ", " << listIt->samplerBinding << "]\n";
        }
        QShader::NativeShaderInfo shaderInfo = bs.nativeShaderInfo(keys[i]);
        if (shaderInfo.flags)
            ts << "Native shader info flags: " << shaderInfo.flags << "\n";
        if (!shaderInfo.extraBufferBindings.isEmpty()) {
            ts << "Native shader extra buffer bindings:\n";
            for (auto mapIt = shaderInfo.extraBufferBindings.cbegin(), mapItEnd = shaderInfo.extraBufferBindings.cend();
                 mapIt != mapItEnd; ++mapIt)
            {
                static struct {
                    QShaderPrivate::MslNativeShaderInfoExtraBufferBindings key;
                    const char *str;
                } ebbNames[] = {
                    { QShaderPrivate::MslTessVertIndicesBufferBinding, "tessellation(vert)-index-buffer-binding" },
                    { QShaderPrivate::MslTessVertTescOutputBufferBinding, "tessellation(vert/tesc)-output-buffer-binding" },
                    { QShaderPrivate::MslTessTescTessLevelBufferBinding, "tessellation(tesc)-level-buffer-binding" },
                    { QShaderPrivate::MslTessTescPatchOutputBufferBinding, "tessellation(tesc)-patch-output-buffer-binding" },
                    { QShaderPrivate::MslTessTescParamsBufferBinding, "tessellation(tesc)-params-buffer-binding" },
                    { QShaderPrivate::MslTessTescInputBufferBinding, "tessellation(tesc)-input-buffer-binding" },
                    { QShaderPrivate::MslBufferSizeBufferBinding, "buffer-size-buffer-binding" },
                    { QShaderPrivate::MslMultiViewMaskBufferBinding, "view-mask-buffer-binding" }
                };
                bool known = false;
                for (size_t i = 0; i < sizeof(ebbNames) / sizeof(ebbNames[0]); ++i) {
                    if (ebbNames[i].key == mapIt.key()) {
                        ts << "[" << ebbNames[i].str << "] = " << mapIt.value() << "\n";
                        known = true;
                        break;
                    }
                }
                if (!known)
                    ts << "[" << mapIt.key() << "] = " << mapIt.value() << "\n";
            }
        }

        ts << "Contents:\n";
        switch (keys[i].source()) {
        case QShader::SpirvShader:
        case QShader::DxbcShader:
        case QShader::DxilShader:
        case QShader::MetalLibShader:
            ts << "Binary of " << shader.shader().size() << " bytes\n\n";
            break;
        default:
            ts << shader.shader() << "\n";
            break;
        }
        ts << "\n************************************\n\n";
    }
}

static QShaderKey shaderKeyFromWhatSpec(const QString &what, QShader::Variant variant)
{
    const QStringList typeAndVersion = what.split(QLatin1Char(','), Qt::SkipEmptyParts);
    if (typeAndVersion.size() < 2)
        return {};

    QShader::Source src;
    if (typeAndVersion[0] == QLatin1String("spirv"))
        src = QShader::SpirvShader;
    else if (typeAndVersion[0] == QLatin1String("glsl"))
        src = QShader::GlslShader;
    else if (typeAndVersion[0] == QLatin1String("hlsl"))
        src = QShader::HlslShader;
    else if (typeAndVersion[0] == QLatin1String("msl"))
        src = QShader::MslShader;
    else if (typeAndVersion[0] == QLatin1String("dxbc"))
        src = QShader::DxbcShader;
    else if (typeAndVersion[0] == QLatin1String("dxil"))
        src = QShader::DxilShader;
    else if (typeAndVersion[0] == QLatin1String("metallib"))
        src = QShader::MetalLibShader;
    else if (typeAndVersion[0] == QLatin1String("wgsl"))
        src = QShader::WgslShader;
    else
        return {};

    QShaderVersion::Flags flags;
    QString version = typeAndVersion[1];
    if (version.endsWith(QLatin1String(" es"))) {
        version = version.left(version.size() - 3);
        flags |= QShaderVersion::GlslEs;
    } else if (version.endsWith(QLatin1String("es"))) {
        version = version.left(version.size() - 2);
        flags |= QShaderVersion::GlslEs;
    }
    const int ver = version.toInt();

    return { src, { ver, flags }, variant };
}

static bool extract(const QShader &bs, const QString &what, QShader::Variant variant, const QString &outfn)
{
    if (what == QLatin1String("reflect")) {
        const QByteArray reflect = bs.description().toJson();
        if (!writeToFile(reflect, outfn, FileType::Text))
            return false;
        if (!silent)
            qDebug("Reflection data written to %s", qPrintable(outfn));
        return true;
    }

    const QShaderKey key = shaderKeyFromWhatSpec(what, variant);
    const QShaderCode code = bs.shader(key);
    if (code.shader().isEmpty())
        return false;
    if (!writeToFile(code.shader(), outfn, FileType::Binary))
        return false;
    if (!silent) {
        qDebug("%s %d%s code (variant %s) written to %s. Entry point is '%s'.",
               qPrintable(sourceStr(key.source())),
               key.sourceVersion().version(),
               key.sourceVersion().flags().testFlag(QShaderVersion::GlslEs) ? " es" : "",
               qPrintable(sourceVariantStr(key.sourceVariant())),
               qPrintable(outfn), code.entryPoint().constData());
    }
    return true;
}

static bool addOrReplace(const QShader &shaderPack,
                         const QStringList &whatList,
                         QShader::Variant variant,
                         const QString &outfn,
                         QShader::SerializedFormatVersion qsbVersion)
{
    QShader workShaderPack = shaderPack;
    for (const QString &what : whatList) {
        const QStringList spec = what.split(QLatin1Char(','), Qt::SkipEmptyParts);
        if (spec.size() < 3) {
            printError("Invalid replace spec '%s'", qPrintable(what));
            return false;
        }

        const QShaderKey key = shaderKeyFromWhatSpec(what, variant);
        const QString fn = spec[2];

        const QByteArray buf = readFile(fn, FileType::Binary);
        if (buf.isEmpty())
            return false;

        // Does not matter if 'key' was present before or not, we support both
        // replacing and adding using the same qsb -r ... syntax.

        const QShaderCode code(buf, QByteArrayLiteral("main"));
        workShaderPack.setShader(key, code);

        if (!silent) {
            qDebug("Replaced %s %d%s (variant %s) with %s. Entry point is 'main'.",
                   qPrintable(sourceStr(key.source())),
                   key.sourceVersion().version(),
                   key.sourceVersion().flags().testFlag(QShaderVersion::GlslEs) ? " es" : "",
                   qPrintable(sourceVariantStr(key.sourceVariant())),
                   qPrintable(fn));
        }
    }
    return writeToFile(workShaderPack.serialized(qsbVersion), outfn, FileType::Binary);
}

static bool remove(const QShader &shaderPack,
                   const QStringList &whatList,
                   QShader::Variant variant,
                   const QString &outfn,
                   QShader::SerializedFormatVersion qsbVersion)
{
    QShader workShaderPack = shaderPack;
    for (const QString &what : whatList) {
        const QShaderKey key = shaderKeyFromWhatSpec(what, variant);
        if (!workShaderPack.availableShaders().contains(key))
            continue;
        workShaderPack.removeShader(key);
        if (!silent) {
            qDebug("Removed %s %d%s (variant %s).",
                   qPrintable(sourceStr(key.source())),
                   key.sourceVersion().version(),
                   key.sourceVersion().flags().testFlag(QShaderVersion::GlslEs) ? " es" : "",
                   qPrintable(sourceVariantStr(key.sourceVariant())));
        }
    }
    return writeToFile(workShaderPack.serialized(qsbVersion), outfn, FileType::Binary);
}

static QByteArray fxcProfile(const QShader &bs, const QShaderKey &k)
{
    QByteArray t;

    switch (bs.stage()) {
    case QShader::VertexStage:
        t += QByteArrayLiteral("vs_");
        break;
    case QShader::TessellationControlStage:
        t += QByteArrayLiteral("hs_");
        break;
    case QShader::TessellationEvaluationStage:
        t += QByteArrayLiteral("ds_");
        break;
    case QShader::GeometryStage:
        t += QByteArrayLiteral("gs_");
        break;
    case QShader::FragmentStage:
        t += QByteArrayLiteral("ps_");
        break;
    case QShader::ComputeStage:
        t += QByteArrayLiteral("cs_");
        break;
    default:
        break;
    }

    const int major = k.sourceVersion().version() / 10;
    const int minor = k.sourceVersion().version() % 10;
    t += QByteArray::number(major);
    t += '_';
    t += QByteArray::number(minor);

    return t;
}

static void replaceShaderContents(QShader *shaderPack,
                                  const QShaderKey &originalKey,
                                  QShader::Source newType,
                                  const QByteArray &contents,
                                  const QByteArray &entryPoint)
{
    QShaderKey newKey = originalKey;
    newKey.setSource(newType);
    QShaderCode shader(contents, entryPoint);
    shaderPack->setShader(newKey, shader);
    if (newKey != originalKey) {
        shaderPack->setResourceBindingMap(newKey, shaderPack->nativeResourceBindingMap(originalKey));
        shaderPack->removeResourceBindingMap(originalKey);
        shaderPack->setSeparateToCombinedImageSamplerMappingList(newKey, shaderPack->separateToCombinedImageSamplerMappingList(originalKey));
        shaderPack->removeSeparateToCombinedImageSamplerMappingList(originalKey);
        shaderPack->removeShader(originalKey);
    }
}

static bool generateDepfile(QFile &depfile, const QString &inputFilename, const QString &originalInputFilename,
                            const QString &outputFilename)
{
    constexpr QByteArrayView includeKeyword("include");
    // Assume that the minimalistic include statment should look as following: #include "x"
    constexpr qsizetype minIncludeStatementSize = includeKeyword.size() + 5;

    QFile inputFile(inputFilename);
    if (!inputFile.open(QFile::ReadOnly)) {
        printError("Unable to open input file: '%s'", qPrintable(inputFilename));
        return false;
    }
    depfile.write(outputFilename.toUtf8());
    depfile.write(": \\\n  "_ba);
    depfile.write(originalInputFilename.toUtf8());
    enum { ParseHash, ParseInclude, ParseFilename } parserState = ParseHash;

    QSet<QString> knownDeps;
    QByteArray outputBuffer;
    while (!inputFile.atEnd()) {
        QByteArray line = inputFile.readLine();
        if (line.size() < minIncludeStatementSize)
            continue;

        parserState = ParseHash;
        for (auto it = line.constBegin(); it < line.constEnd(); ++it) {
            const auto c = *it;
            if (c == '\t' || c == ' ')
                continue;

            if (parserState == ParseHash) {
                // Looking for #
                if (c == '#')
                    parserState = ParseInclude;
                else
                    break;
            } else if (parserState == ParseInclude) {
                // Looking for 'include'
                if (includeKeyword == QByteArrayView(it, includeKeyword.size()))
                    parserState = ParseFilename;
                else
                    break;
                it += includeKeyword.size();
            } else if (parserState == ParseFilename) {
                // Looking for wrapping quotes
                QString includeString = QString::fromUtf8(QByteArrayView(it, line.constEnd()));
                QChar quoteCounterpart;
                if (includeString.front() == '<') {
                    quoteCounterpart = '>';
                } else if (includeString.front() == '"') {
                    quoteCounterpart = '"';
                } else {
                    qWarning("Unrecognised include statement: '%s'", qPrintable(includeString));
                    break;
                }

                const auto filenameBegin = 1;
                const auto filenameEnd = includeString.indexOf(quoteCounterpart, filenameBegin);
                if (filenameEnd > filenameBegin) {
                    QString filename =
                            includeString.sliced(filenameBegin, filenameEnd - filenameBegin);
                    QFileInfo info(QFileInfo(inputFilename).absolutePath() + '/' + filename);

                    if (info.exists()) {
                        QString filePath = info.absoluteFilePath();
                        if (!knownDeps.contains(filePath)) {
                            outputBuffer.append(" \\\n  "_ba + filePath.toUtf8());
                            knownDeps.insert(filePath);
                        }
                    } else {
                        qWarning("File '%s' included in '%s' doesn't exist. Skip adding "
                                 "dependency.",
                                 qPrintable(filename), qPrintable(inputFilename));
                    }
                }
                break;
            }
        }
    }

    if (!outputBuffer.isEmpty())
        depfile.write(outputBuffer);
    return true;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QCommandLineParser cmdLineParser;
    const QString appDesc = QString::asprintf("Qt Shader Baker (using QShader from Qt %s)", qVersion());
    cmdLineParser.setApplicationDescription(appDesc);
    app.setApplicationVersion(QLatin1String(QT_VERSION_STR));
    cmdLineParser.addHelpOption();
    cmdLineParser.addVersionOption();
    cmdLineParser.addPositionalArgument(QLatin1String("file"),
                                        QObject::tr("Vulkan GLSL source file to compile. The file extension determines the shader stage, and can be one of "
                                                    ".vert, .tesc, .tese, .geom, .frag, .comp. "
                                                    "Note: Tessellation control/evaluation is not supported with HLSL, instead use -r to inject handcrafted hull/domain shaders. "
                                                    "Some targets may need special arguments to be set, e.g. MSL tessellation will likely need --msltess, --tess-vertex-count, --tess-mode, depending on the stage. "
                                                    "Geometry shaders are not supported with Metal."
                                                    ),
                                        QObject::tr("file"));
    QCommandLineOption batchableOption({ "b", "batchable" }, QObject::tr("Also generates rewritten vertex shader for Qt Quick scene graph batching."));
    cmdLineParser.addOption(batchableOption);
    QCommandLineOption batchLocOption("zorder-loc",
                                      QObject::tr("The extra vertex input location when rewriting for batching. Defaults to 7."),
                                      QObject::tr("location"));
    cmdLineParser.addOption(batchLocOption);
    QCommandLineOption glslOption("glsl",
                                  QObject::tr("Comma separated list of GLSL versions to generate. (for example, \"100 es,120,330\")"),
                                  QObject::tr("versions"));
    cmdLineParser.addOption(glslOption);
    QCommandLineOption hlslOption("hlsl",
                                  QObject::tr("Comma separated list of HLSL (Shader Model) versions to generate. F.ex. 50 is 5.0, 51 is 5.1."),
                                  QObject::tr("versions"));
    cmdLineParser.addOption(hlslOption);
    QCommandLineOption mslOption("msl",
                                 QObject::tr("Comma separated list of Metal Shading Language versions to generate. F.ex. 12 is 1.2, 20 is 2.0."),
                                 QObject::tr("versions"));
    cmdLineParser.addOption(mslOption);
    QCommandLineOption shortcutDefaultOption("qt6", QObject::tr("Equivalent to --glsl \"100 es,120,150\" --hlsl 50 --msl 12. "
                                                                "This set is commonly used with shaders for Qt Quick materials and effects."));
    cmdLineParser.addOption(shortcutDefaultOption);
    QCommandLineOption tessOption("msltess", QObject::tr("Indicates that a vertex shader is going to be used in a pipeline with tessellation. "
                                                         "Mandatory for vertex shaders planned to be used with tessellation when targeting Metal (--msl)."));
    cmdLineParser.addOption(tessOption);
    QCommandLineOption tessVertCountOption("tess-vertex-count", QObject::tr("The output vertex count from the tessellation control stage. "
                                                                            "Mandatory for tessellation evaluation shaders planned to be used with Metal. "
                                                                            "The default value is 3. "
                                                                            "If it does not match the tess.control stage, the generated MSL code will not function as expected."),
                                           QObject::tr("count"));
    cmdLineParser.addOption(tessVertCountOption);
    QCommandLineOption tessModeOption("tess-mode", QObject::tr("The tessellation mode: triangles or quads. Mandatory for tessellation control shaders planned to be used with Metal. "
                                                               "The default value is triangles. Isolines are not supported with Metal. "
                                                               "If it does not match the tess.evaluation stage, the generated MSL code will not function as expected."),
                                      QObject::tr("mode"));
    cmdLineParser.addOption(tessModeOption);
    QCommandLineOption multiViewCountOption("view-count", QObject::tr("The number of views the shader is used with. num_views must be >= 2. "
                                                                      "Mandatory when multiview rendering is used (gl_ViewIndex). "
                                                                      "Set only for vertex shaders that really do rely on multiview (as the resulting asset is tied to num_views). "
                                                                      "Can be set for fragment shaders too, to get QSHADER_VIEW_COUNT auto-defined. (useful for ensuring uniform buffer layouts)"),
                                            QObject::tr("num_views"));
    cmdLineParser.addOption(multiViewCountOption);
    QCommandLineOption debugInfoOption("g", QObject::tr("Generate full debug info for SPIR-V and DXBC"));
    cmdLineParser.addOption(debugInfoOption);
    QCommandLineOption spirvOptOption("O", QObject::tr("Invoke spirv-opt (external tool) to optimize SPIR-V for performance."));
    cmdLineParser.addOption(spirvOptOption);
    QCommandLineOption outputOption({ "o", "output" },
                                     QObject::tr("Output file for the shader pack."),
                                     QObject::tr("filename"));
    cmdLineParser.addOption(outputOption);
    QCommandLineOption origFnOption("orig-file",
                                    QObject::tr("Filename to be be used for dependency tracking instead of <file>."),
                                    QObject::tr("filename"));
    cmdLineParser.addOption(origFnOption);
    QCommandLineOption qsbVersionOption("qsbversion",
                                     QObject::tr("QSB version to use for the output file. By default the latest version is automatically used, "
                                                 "use only to bake compatibility versions. F.ex. 64 is Qt 6.4."),
                                     QObject::tr("version"));
    cmdLineParser.addOption(qsbVersionOption);
    QCommandLineOption fxcOption({ "c", "fxc" }, QObject::tr("In combination with --hlsl invokes fxc (SM 5.0/5.1) or dxc (SM 6.0+) to store DXBC or DXIL instead of HLSL."));
    cmdLineParser.addOption(fxcOption);
    QCommandLineOption mtllibOption({ "t", "metallib" },
                                    QObject::tr("In combination with --msl builds a Metal library with xcrun metal(lib) and stores that instead of the source. "
                                                "Suitable only when targeting macOS, not iOS."));
    cmdLineParser.addOption(mtllibOption);
    QCommandLineOption mtllibIosOption({ "T", "metallib-ios" },
                                       QObject::tr("In combination with --msl builds a Metal library with xcrun metal(lib) and stores that instead of the source. "
                                                   "Suitable only when targeting iOS, not macOS."));
    cmdLineParser.addOption(mtllibIosOption);
    QCommandLineOption defineOption({ "D", "define" }, QObject::tr("Define macro. This argument can be specified multiple times."), QObject::tr("name[=value]"));
    cmdLineParser.addOption(defineOption);
    QCommandLineOption perTargetCompileOption({ "p", "per-target" }, QObject::tr("Enable per-target compilation. (instead of source->SPIRV->targets, do "
                                                                                 "source->SPIRV->target separately for each target)"));
    cmdLineParser.addOption(perTargetCompileOption);
    QCommandLineOption dumpOption({ "d", "dump" }, QObject::tr("Switches to dump mode. Input file is expected to be a shader pack."));
    cmdLineParser.addOption(dumpOption);
    QCommandLineOption extractOption({ "x", "extract" }, QObject::tr("Switches to extract mode. Input file is expected to be a shader pack. "
                                                                     "Result is written to the output specified by -o. "
                                                                     "Pass -b to choose the batchable variant. "
                                                                     "<what>=reflect|spirv,<version>|glsl,<version>|..."),
                                     QObject::tr("what"));
    cmdLineParser.addOption(extractOption);
    QCommandLineOption replaceOption({ "r", "replace" },
                                     QObject::tr("Switches to replace mode. Replaces the specified shader in the shader pack with the contents of a file. "
                                                 "This argument can be specified multiple times. "
                                                 "Pass -b to choose the batchable variant. "
                                                 "Also supports adding a shader for a target/variant that was not present before. "
                                                 "<what>=<target>,<filename> where <target>=spirv,<version>|glsl,<version>|..."),
                                     QObject::tr("what"));
    cmdLineParser.addOption(replaceOption);
    QCommandLineOption eraseOption({ "e", "erase" },
                                     QObject::tr("Switches to erase mode. Removes the specified shader from the shader pack. "
                                                 "Pass -b to choose the batchable variant. "
                                                 "<what>=spirv,<version>|glsl,<version>|..."),
                                     QObject::tr("what"));
    cmdLineParser.addOption(eraseOption);
    QCommandLineOption silentOption({ "s", "silent" }, QObject::tr("Enables silent mode. Only fatal errors will be printed."));
    cmdLineParser.addOption(silentOption);

    QCommandLineOption depfileOption("depfile",
                                     QObject::tr("Enables generating the depfile for the input "
                                                 "shaders, using the #include statements."),
                                     QObject::tr("depfile"));

    QCommandLineOption mediumpOption("mediump", QObject::tr("Default to medium precision for floats in fragment shaders instead of high. "
                                                            "GLSL ES only; ignored for everything else, including GLSL."));
    cmdLineParser.addOption(mediumpOption);

    cmdLineParser.addOption(depfileOption);

    cmdLineParser.process(app);

    if (cmdLineParser.positionalArguments().isEmpty()) {
        cmdLineParser.showHelp();
        return 0;
    }

    silent = cmdLineParser.isSet(silentOption);

    QShaderBaker baker;

    QFile depfile;
    if (const QString depfilePath = cmdLineParser.value(depfileOption); !depfilePath.isEmpty()) {
        QDir().mkpath(QFileInfo(depfilePath).path());
        depfile.setFileName(depfilePath);
        if (!depfile.open(QFile::WriteOnly | QFile::Truncate)) {
            printError("Unable to create DEPFILE: '%s'", qPrintable(depfilePath));
            return 1;
        }
    }

    const bool depfileRequired = depfile.isOpen();

    for (const QString &fn : cmdLineParser.positionalArguments()) {
        auto qsbVersion = QShader::SerializedFormatVersion::Latest;
        if (cmdLineParser.isSet(qsbVersionOption)) {
            const QString qsbVersionString = cmdLineParser.value(qsbVersionOption);
            if (qsbVersionString == QStringLiteral("64")) {
                qsbVersion = QShader::SerializedFormatVersion::Qt_6_4;
            } else if (qsbVersionString == QStringLiteral("65")) {
                qsbVersion = QShader::SerializedFormatVersion::Qt_6_5;
            } else if (qsbVersionString.toLower() != QStringLiteral("latest")) {
                printError("Unknown Qt qsb version: %s", qPrintable(qsbVersionString));
                printError("Available versions: 64, 65, latest");
                return 1;
            }
        }

        if (cmdLineParser.isSet(dumpOption)
                || cmdLineParser.isSet(extractOption)
                || cmdLineParser.isSet(replaceOption)
                || cmdLineParser.isSet(eraseOption))
        {
            QByteArray buf = readFile(fn, FileType::Binary);
            if (!buf.isEmpty()) {
                QShader bs = QShader::fromSerialized(buf);
                if (bs.isValid()) {
                    const bool batchable = cmdLineParser.isSet(batchableOption);
                    const QShader::Variant variant = batchable ? QShader::BatchableVertexShader : QShader::StandardShader;
                    if (cmdLineParser.isSet(dumpOption)) {
                        dump(bs);
                    } else if (cmdLineParser.isSet(extractOption)) {
                        if (cmdLineParser.isSet(outputOption)) {
                            if (!extract(bs, cmdLineParser.value(extractOption), variant, cmdLineParser.value(outputOption)))
                                return 1;
                        } else {
                            printError("No output file specified");
                        }
                    } else if (cmdLineParser.isSet(replaceOption)) {
                        if (!addOrReplace(bs, cmdLineParser.values(replaceOption), variant, fn, qsbVersion))
                            return 1;
                    } else if (cmdLineParser.isSet(eraseOption)) {
                        if (!remove(bs, cmdLineParser.values(eraseOption), variant, fn, qsbVersion))
                            return 1;
                    }
                } else {
                    printError("Failed to deserialize %s (or the shader pack is empty)", qPrintable(fn));
                }
            }
            continue;
        }

        if (depfileRequired) {
            QString originalFilename = fn;
            if (cmdLineParser.isSet(origFnOption))
                originalFilename = cmdLineParser.value(origFnOption);
            if (!generateDepfile(depfile, fn, originalFilename, cmdLineParser.value(outputOption)))
                return 1;
        }

        baker.setSourceFileName(fn);

        baker.setPerTargetCompilation(cmdLineParser.isSet(perTargetCompileOption));

        QShaderBaker::SpirvOptions spirvOptions;
        // We either want full debug info, or none at all (so no variable names
        // either - that too can be stripped after the SPIRV-Cross stage).
        if (cmdLineParser.isSet(debugInfoOption))
            spirvOptions |= QShaderBaker::SpirvOption::GenerateFullDebugInfo;
        else
            spirvOptions |= QShaderBaker::SpirvOption::StripDebugAndVarInfo;

        baker.setSpirvOptions(spirvOptions);

        QShaderBaker::GlslOptions glslOptions;
        if (cmdLineParser.isSet(mediumpOption))
            glslOptions |= QShaderBaker::GlslOption::GlslEsFragDefaultFloatPrecisionMedium;

        baker.setGlslOptions(glslOptions);

        QList<QShader::Variant> variants;
        variants << QShader::StandardShader;
        if (cmdLineParser.isSet(batchableOption)) {
            variants << QShader::BatchableVertexShader;
            if (cmdLineParser.isSet(batchLocOption))
                baker.setBatchableVertexShaderExtraInputLocation(cmdLineParser.value(batchLocOption).toInt());
        }
        if (cmdLineParser.isSet(tessOption)) {
            variants << QShader::UInt16IndexedVertexAsComputeShader
                     << QShader::UInt32IndexedVertexAsComputeShader
                     << QShader::NonIndexedVertexAsComputeShader;
        }

        if (cmdLineParser.isSet(tessModeOption)) {
            const QString tessModeStr = cmdLineParser.value(tessModeOption).toLower();
            if (tessModeStr == QLatin1String("triangles"))
                baker.setTessellationMode(QShaderDescription::TrianglesTessellationMode);
            else if (tessModeStr == QLatin1String("quads"))
                baker.setTessellationMode(QShaderDescription::QuadTessellationMode);
            else
                qWarning("Unknown tessellation mode '%s'", qPrintable(tessModeStr));
        }

        if (cmdLineParser.isSet(tessVertCountOption))
            baker.setTessellationOutputVertexCount(cmdLineParser.value(tessVertCountOption).toInt());

        if (cmdLineParser.isSet(multiViewCountOption))
            baker.setMultiViewCount(cmdLineParser.value(multiViewCountOption).toInt());

        baker.setGeneratedShaderVariants(variants);

        QList<QShaderBaker::GeneratedShader> genShaders;

        genShaders << std::make_pair(QShader::SpirvShader, QShaderVersion(100));

        if (cmdLineParser.isSet(glslOption)) {
            const QStringList versions = cmdLineParser.value(glslOption).trimmed().split(',');
            for (QString version : versions) {
                QShaderVersion::Flags flags;
                if (version.endsWith(QLatin1String(" es"))) {
                    version = version.left(version.size() - 3);
                    flags |= QShaderVersion::GlslEs;
                } else if (version.endsWith(QLatin1String("es"))) {
                    version = version.left(version.size() - 2);
                    flags |= QShaderVersion::GlslEs;
                }
                bool ok = false;
                int v = version.toInt(&ok);
                if (ok)
                    genShaders << std::make_pair(QShader::GlslShader, QShaderVersion(v, flags));
                else
                    printError("Ignoring invalid GLSL version %s", qPrintable(version));
            }
        }

        if (cmdLineParser.isSet(hlslOption)) {
            const QStringList versions = cmdLineParser.value(hlslOption).trimmed().split(',');
            for (QString version : versions) {
                bool ok = false;
                int v = version.toInt(&ok);
                if (ok) {
                    genShaders << std::make_pair(QShader::HlslShader, QShaderVersion(v));
                } else {
                    printError("Ignoring invalid HLSL (Shader Model) version %s",
                               qPrintable(version));
                }
            }
        }

        if (cmdLineParser.isSet(mslOption)) {
            const QStringList versions = cmdLineParser.value(mslOption).trimmed().split(',');
            for (QString version : versions) {
                bool ok = false;
                int v = version.toInt(&ok);
                if (ok)
                    genShaders << std::make_pair(QShader::MslShader, QShaderVersion(v));
                else
                    printError("Ignoring invalid MSL version %s", qPrintable(version));
            }
        }

        if (cmdLineParser.isSet(shortcutDefaultOption)) {
            for (const QShaderBaker::GeneratedShader &genShaderEntry :
            {
                 std::make_pair(QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs)),
                 std::make_pair(QShader::GlslShader, QShaderVersion(120)),
                 std::make_pair(QShader::GlslShader, QShaderVersion(150)),
                 std::make_pair(QShader::HlslShader, QShaderVersion(50)),
                 std::make_pair(QShader::MslShader, QShaderVersion(12))
            })
            {
                if (!genShaders.contains(genShaderEntry))
                    genShaders << genShaderEntry;
            }
        }

        baker.setGeneratedShaders(genShaders);

        if (cmdLineParser.isSet(defineOption)) {
            QByteArray preamble;
            const QStringList defines = cmdLineParser.values(defineOption);
            for (const QString &def : defines) {
                const QStringList defs = def.split(QLatin1Char('='), Qt::SkipEmptyParts);
                if (!defs.isEmpty()) {
                    preamble.append("#define");
                    for (const QString &s : defs) {
                        preamble.append(' ');
                        preamble.append(s.toUtf8());
                    }
                    preamble.append('\n');
                }
            }
            baker.setPreamble(preamble);
        }

        QShader bs = baker.bake();
        if (!bs.isValid()) {
            printError("Shader baking failed: %s", qPrintable(baker.errorMessage()));
            return 1;
        }

        // post processing: run spirv-opt when requested for each entry with
        // type SpirvShader and replace the contents. Not having spirv-opt
        // available must not be a fatal error, skip it if the process fails.
        if (cmdLineParser.isSet(spirvOptOption)) {
            QTemporaryDir tempDir;
            if (!tempDir.isValid()) {
                printError("Failed to create temporary directory");
                return 1;
            }
            auto skeys = bs.availableShaders();
            for (QShaderKey &k : skeys) {
                if (k.source() == QShader::SpirvShader) {
                    QShaderCode s = bs.shader(k);

                    const QString tmpIn = writeTemp(tempDir, QLatin1String("qsb_spv_temp"), s, FileType::Binary);
                    const QString tmpOut = tempDir.path() + QLatin1String("/qsb_spv_temp_out");
                    if (tmpIn.isEmpty())
                        break;

                    const QStringList arguments({
                                                    QLatin1String("-O"),
                                                    QDir::toNativeSeparators(tmpIn),
                                                    QLatin1String("-o"), QDir::toNativeSeparators(tmpOut)
                                                });
                    QByteArray output;
                    QByteArray errorOutput;
                    bool success = runProcess(QLatin1String("spirv-opt"), arguments, &output, &errorOutput);
                    if (success) {
                        const QByteArray bytecode = readFile(tmpOut, FileType::Binary);
                        if (!bytecode.isEmpty())
                            replaceShaderContents(&bs, k, QShader::SpirvShader, bytecode, s.entryPoint());
                    } else {
                        if ((!output.isEmpty() || !errorOutput.isEmpty()) && !silent) {
                            printError("%s\n%s",
                                   qPrintable(output.constData()),
                                   qPrintable(errorOutput.constData()));
                        }
                    }
                }
            }
        }

        // post processing: run fxc/dxc when requested for each entry with type
        // HlslShader and add a new entry with type DxbcShader/DxilShader and remove the
        // original HlslShader entry
        if (cmdLineParser.isSet(fxcOption)) {
            QTemporaryDir tempDir;
            if (!tempDir.isValid()) {
                printError("Failed to create temporary directory");
                return 1;
            }
            auto skeys = bs.availableShaders();
            for (QShaderKey &k : skeys) {
                if (k.source() == QShader::HlslShader) {
                    // For Shader Model 6.0 and higher, use dxc, fxc will not compile that anymore.
                    const bool useDxc = k.sourceVersion().version() >= 60;
                    QShaderCode s = bs.shader(k);

                    const QString tmpIn = writeTemp(tempDir, QLatin1String("qsb_hlsl_temp"), s, FileType::Text);
                    const QString tmpOut = tempDir.path() + QLatin1String("/qsb_hlsl_temp_out");
                    if (tmpIn.isEmpty())
                        break;

                    const QByteArray typeArg = fxcProfile(bs, k);
                    QStringList arguments({
                                              QLatin1String("/nologo"),
                                              QLatin1String("/E"), QString::fromLocal8Bit(s.entryPoint()),
                                              QLatin1String("/T"), QString::fromLocal8Bit(typeArg),
                                              QLatin1String("/Fo"), QDir::toNativeSeparators(tmpOut)
                                          });
                    if (cmdLineParser.isSet(debugInfoOption))
                        arguments << QLatin1String("/Od") << QLatin1String("/Zi");
                    arguments.append(QDir::toNativeSeparators(tmpIn));
                    QByteArray output;
                    QByteArray errorOutput;
                    const QString compilerName = useDxc ? QLatin1String("dxc") : QLatin1String("fxc");
                    bool success = runProcess(compilerName, arguments, &output, &errorOutput);
                    if (success) {
                        const QByteArray bytecode = readFile(tmpOut, FileType::Binary);
                        if (!bytecode.isEmpty()) {
                            const QShader::Source bytecodeType = useDxc ? QShader::DxilShader : QShader::DxbcShader;
                            replaceShaderContents(&bs, k, bytecodeType, bytecode, s.entryPoint());
                        }
                    } else {
                        if ((!output.isEmpty() || !errorOutput.isEmpty()) && !silent) {
                            printError("%s\n%s",
                                   qPrintable(output.constData()),
                                   qPrintable(errorOutput.constData()));
                        }
                    }
                }
            }
        }

        // post processing: run xcrun metal and metallib when requested for
        // each entry with type MslShader and add a new entry with type
        // MetalLibShader and remove the original MslShader entry
        if (cmdLineParser.isSet(mtllibOption) || cmdLineParser.isSet(mtllibIosOption)) {
            const bool isIos = cmdLineParser.isSet(mtllibIosOption);
            QTemporaryDir tempDir;
            if (!tempDir.isValid()) {
                printError("Failed to create temporary directory");
                return 1;
            }
            auto skeys = bs.availableShaders();
            for (const QShaderKey &k : skeys) {
                if (k.source() == QShader::MslShader) {
                    QShaderCode s = bs.shader(k);

                    // having the .metal file extension may matter for the external tools here, so use that
                    const QString tmpIn = writeTemp(tempDir, QLatin1String("qsb_msl_temp.metal"), s, FileType::Text);
                    const QString tmpInterm = tempDir.path() + QLatin1String("/qsb_msl_temp_air");
                    const QString tmpOut = tempDir.path() + QLatin1String("/qsb_msl_temp_out");
                    if (tmpIn.isEmpty())
                        break;

                    const QString binary = QLatin1String("xcrun");
                    const QStringList baseArguments = {
                        QLatin1String("-sdk"),
                        isIos ? QLatin1String("iphoneos") : QLatin1String("macosx")
                    };
                    QStringList arguments = baseArguments;
                    const QString langVerFmt = QLatin1String("-std=%1-metal%2.%3");
                    const QString langPlatform = isIos ? QLatin1String("ios") : QLatin1String("macos");
                    const int langMajor = k.sourceVersion().version() / 10;
                    const int langMinor = k.sourceVersion().version() % 10;
                    const QString langVer = langVerFmt.arg(langPlatform).arg(langMajor).arg(langMinor);
                    arguments.append({
                            QLatin1String("metal"),
                            QLatin1String("-c"),
                            langVer,
                            QDir::toNativeSeparators(tmpIn),
                            QLatin1String("-o"),
                            QDir::toNativeSeparators(tmpInterm)
                        });
                    QByteArray output;
                    QByteArray errorOutput;
                    bool success = runProcess(binary, arguments, &output, &errorOutput);
                    if (success) {
                        arguments = baseArguments;
                        arguments.append({QLatin1String("metallib"), QDir::toNativeSeparators(tmpInterm),
                                    QLatin1String("-o"), QDir::toNativeSeparators(tmpOut)});
                        output.clear();
                        errorOutput.clear();
                        success = runProcess(binary, arguments, &output, &errorOutput);
                        if (success) {
                            const QByteArray bytecode = readFile(tmpOut, FileType::Binary);
                            if (!bytecode.isEmpty())
                                replaceShaderContents(&bs, k, QShader::MetalLibShader, bytecode, s.entryPoint());
                        } else {
                            if ((!output.isEmpty() || !errorOutput.isEmpty()) && !silent) {
                                printError("%s\n%s",
                                       qPrintable(output.constData()),
                                       qPrintable(errorOutput.constData()));
                            }
                        }
                    } else {
                        if ((!output.isEmpty() || !errorOutput.isEmpty()) && !silent) {
                            printError("%s\n%s",
                                     qPrintable(output.constData()),
                                     qPrintable(errorOutput.constData()));
                        }
                    }
                }
            }
        }

        if (cmdLineParser.isSet(outputOption))
            writeToFile(bs.serialized(qsbVersion), cmdLineParser.value(outputOption), FileType::Binary);
    }

    return 0;
}
