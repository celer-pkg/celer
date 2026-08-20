// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qspirvcompiler_p.h"
#include "qshaderrewriter_p.h"
#include <QFile>
#include <QFileInfo>

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-override")
#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <SPIRV/GlslangToSpv.h>
QT_WARNING_POP

//#define TOKENIZER_DEBUG

QT_BEGIN_NAMESPACE

struct QSpirvCompilerPrivate
{
    bool readFile(const QString &fn);
    bool compile();

    QString sourceFileName;
    QByteArray source;
    QByteArray batchableSource;
    EShLanguage stage = EShLangVertex;
    QSpirvCompiler::Flags flags;
    QByteArray preamble;
    int batchAttrLoc = 7;
    QByteArray spirv;
    QString log;
};

bool QSpirvCompilerPrivate::readFile(const QString &fn)
{
    QFile f(fn);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("QSpirvCompiler: Failed to open %s", qPrintable(fn));
        return false;
    }
    source = f.readAll();
    batchableSource.clear();
    sourceFileName = fn;
    f.close();
    return true;
}

using namespace QtShaderTools;

class Includer : public glslang::TShader::Includer
{
public:
    IncludeResult *includeLocal(const char *headerName,
                                const char *includerName,
                                size_t inclusionDepth) override
    {
        Q_UNUSED(inclusionDepth);
        return readFile(headerName, includerName);
    }

    IncludeResult *includeSystem(const char *headerName,
                                 const char *includerName,
                                 size_t inclusionDepth) override
    {
        Q_UNUSED(inclusionDepth);
        return readFile(headerName, includerName);
    }

    void releaseInclude(IncludeResult *result) override
    {
        if (result) {
            delete static_cast<QByteArray *>(result->userData);
            delete result;
        }
    }

private:
    IncludeResult *readFile(const char *headerName, const char *includerName);
};

glslang::TShader::Includer::IncludeResult *Includer::readFile(const char *headerName, const char *includerName)
{
    // Just treat the included name as relative to the includer:
    //   Take the path from the includer, append the included name, remove redundancies.
    // This should work also for qrc (source filenames with qrc:/ or :/ prefix).

    QString includer = QString::fromUtf8(includerName);
    if (includer.isEmpty())
        includer = QLatin1String(".");
    QString included = QFileInfo(includer).canonicalPath() + QLatin1Char('/') + QString::fromUtf8(headerName);
    included = QFileInfo(included).canonicalFilePath();
    if (included.isEmpty()) {
        qWarning("QSpirvCompiler: Failed to find include file %s", headerName);
        return nullptr;
    }
    QFile f(included);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("QSpirvCompiler: Failed to read include file %s", qPrintable(included));
        return nullptr;
    }

    QByteArray *data = new QByteArray;
    *data = f.readAll();
    return new IncludeResult(included.toStdString(), data->constData(), data->size(), data);
}

class GlobalInit
{
public:
    GlobalInit() { glslang::InitializeProcess(); }
    ~GlobalInit() { glslang::FinalizeProcess(); }
};

bool QSpirvCompilerPrivate::compile()
{
    log.clear();

    const bool useBatchable = (stage == EShLangVertex && flags.testFlag(QSpirvCompiler::RewriteToMakeBatchableForSG));
    const QByteArray *actualSource = useBatchable ? &batchableSource : &source;
    if (actualSource->isEmpty())
        return false;

    static GlobalInit globalInit;

    glslang::TShader shader(stage);
    const QByteArray fn = sourceFileName.toUtf8();
    const char *fnStr = fn.constData();
    const char *srcStr = actualSource->constData();
    const int size = actualSource->size();
    shader.setStringsWithLengthsAndNames(&srcStr, &size, &fnStr, 1);
    if (!preamble.isEmpty()) {
        // Line numbers in errors and #version are not affected by having a
        // preamble, which is just what we need.
        shader.setPreamble(preamble.constData());
    }

    shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, 100);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
    shader.setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_0);

    int messages = EShMsgDefault;
    if (flags.testFlag(QSpirvCompiler::FullDebugInfo)) // embed source
        messages |= EShMsgDebugInfo;

    Includer includer;
    if (!shader.parse(QtShaderTools::GetDefaultResources(), 100, false, EShMessages(messages), includer)) {
        qWarning("QSpirvCompiler: Failed to parse shader");
        log = QString::fromUtf8(shader.getInfoLog()).trimmed();
        return false;
    }

    glslang::TProgram program;
    program.addShader(&shader);
    if (!program.link(EShMsgDefault)) {
        qWarning("QSpirvCompiler: Link failed");
        log = QString::fromUtf8(shader.getInfoLog()).trimmed();
        return false;
    }

    // The only interesting option here is the debug info, optimizations and
    // such do not happen at this level.
    glslang::SpvOptions options;
    options.generateDebugInfo = flags.testFlag(QSpirvCompiler::FullDebugInfo);

    std::vector<unsigned int> spv;
    glslang::GlslangToSpv(*program.getIntermediate(stage), spv, &options);
    if (!spv.size()) {
        qWarning("Failed to generate SPIR-V");
        return false;
    }

    spirv.resize(int(spv.size() * 4));
    memcpy(spirv.data(), spv.data(), spirv.size());

    return true;
}

QSpirvCompiler::QSpirvCompiler()
    : d(new QSpirvCompilerPrivate)
{
}

QSpirvCompiler::~QSpirvCompiler()
{
    delete d;
}

void QSpirvCompiler::setSourceFileName(const QString &fileName)
{
    if (!d->readFile(fileName))
        return;

    const QString suffix = QFileInfo(fileName).suffix();
    if (suffix == QStringLiteral("vert")) {
        d->stage = EShLangVertex;
    } else if (suffix == QStringLiteral("frag")) {
        d->stage = EShLangFragment;
    } else if (suffix == QStringLiteral("tesc")) {
        d->stage = EShLangTessControl;
    } else if (suffix == QStringLiteral("tese")) {
        d->stage = EShLangTessEvaluation;
    } else if (suffix == QStringLiteral("geom")) {
        d->stage = EShLangGeometry;
    } else if (suffix == QStringLiteral("comp")) {
        d->stage = EShLangCompute;
    } else {
        qWarning("QSpirvCompiler: Unknown shader stage, defaulting to vertex");
        d->stage = EShLangVertex;
    }
}

static inline EShLanguage mapShaderStage(QShader::Stage stage)
{
    switch (stage) {
    case QShader::VertexStage:
        return EShLangVertex;
    case QShader::TessellationControlStage:
        return EShLangTessControl;
    case QShader::TessellationEvaluationStage:
        return EShLangTessEvaluation;
    case QShader::GeometryStage:
        return EShLangGeometry;
    case QShader::FragmentStage:
        return EShLangFragment;
    case QShader::ComputeStage:
        return EShLangCompute;
    default:
        return EShLangVertex;
    }
}

void QSpirvCompiler::setSourceFileName(const QString &fileName, QShader::Stage stage)
{
    if (!d->readFile(fileName))
        return;

    d->stage = mapShaderStage(stage);
}

void QSpirvCompiler::setSourceDevice(QIODevice *device, QShader::Stage stage, const QString &fileName)
{
    setSourceString(device->readAll(), stage, fileName);
}

void QSpirvCompiler::setSourceString(const QByteArray &sourceString, QShader::Stage stage, const QString &fileName)
{
    d->sourceFileName = fileName; // for error messages, include handling, etc.
    d->source = sourceString;
    d->batchableSource.clear();
    d->stage = mapShaderStage(stage);
}

void QSpirvCompiler::setFlags(Flags flags)
{
    d->flags = flags;
}

void QSpirvCompiler::setPreamble(const QByteArray &preamble)
{
    d->preamble = preamble;
}

void QSpirvCompiler::setSGBatchingVertexInputLocation(int location)
{
    d->batchAttrLoc = location;
}

QByteArray QSpirvCompiler::compileToSpirv()
{
#ifdef TOKENIZER_DEBUG
    QShaderRewriter::debugTokenizer(d->source);
#endif

    if (d->stage == EShLangVertex && d->flags.testFlag(RewriteToMakeBatchableForSG) && d->batchableSource.isEmpty())
        d->batchableSource = QShaderRewriter::addZAdjustment(d->source, d->batchAttrLoc);

    return d->compile() ? d->spirv : QByteArray();
}

QString QSpirvCompiler::errorMessage() const
{
    return d->log;
}

QT_END_NAMESPACE
