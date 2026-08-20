// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qsbinspectorhelper.h"
#include <QFile>
#include <QCollator>
#include <QUrl>

// Note: These methods should be kept up-to-date with the qsb tool.

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

QsbInspectorHelper::QsbInspectorHelper(QObject *parent)
    : QObject{parent}
{

}

QsbShaderData QsbInspectorHelper::shaderData() const
{
    return m_shaderData;
}

QVariantList QsbInspectorHelper::sourceSelectorModel() const
{
    return m_sourceSelectorModel;
}

int QsbInspectorHelper::currentSourceIndex() const
{
    return m_currentSourceIndex;
}

void QsbInspectorHelper::setCurrentSourceIndex(int index)
{
    if (m_currentSourceIndex == index)
        return;
    m_currentSourceIndex = index;
    Q_EMIT currentSourceIndexChanged();
    // When index changes, update source also
    Q_EMIT currentSourceCodeChanged();
}

QString QsbInspectorHelper::currentSourceCode() const
{
    if (m_currentSourceIndex == -1 || m_currentSourceIndex >= m_sourceCodes.size())
        return QString();
    return m_sourceCodes.at(m_currentSourceIndex);
}

// Loads QSB from filename and updates all data
bool QsbInspectorHelper::loadQsb(const QString &filename)
{
    QString qsbFilename = filename;
    QUrl url(filename);
    if (url.scheme() == QStringLiteral("file"))
        qsbFilename = url.toLocalFile();

    QFile qsbFile(qsbFilename);
    if (!qsbFile.open(QIODevice::ReadOnly)) {
        qWarning("Failed to open %s", qPrintable(qsbFilename));
        return false;
    }

    QByteArray buf = qsbFile.readAll();
    if (buf.isEmpty()) {
        qWarning("Empty QSB file %s", qPrintable(qsbFilename));
        return false;
    }

    QShader bs = QShader::fromSerialized(buf);
    if (!bs.isValid()) {
        qWarning("Invalid QSB file %s", qPrintable(qsbFilename));
        return false;
    }

    const auto keys = bs.availableShaders();

    m_shaderData.m_currentFile = qsbFile.fileName();
    m_shaderData.m_stage = stageStr(bs.stage());
    m_shaderData.m_size = qsbFile.size();
    m_shaderData.m_shaderCount = keys.count();
    m_shaderData.m_qsbVersion = QShaderPrivate::get(&bs)->qsbVersion;
    m_shaderData.m_reflectionInfo = bs.description().toJson();

    m_sourceCodes.clear();
    QVariantList sourceSelectorModel;
    for (int i = 0; i < m_shaderData.m_shaderCount; ++i) {
        const auto shaderKey = keys[i];
        ShaderSelectorData selectorData;
        selectorData.m_sourceIndex = i;
        QString name = QString("%1 %2").arg(sourceStr(shaderKey.source()))
                .arg(sourceVersionStr(shaderKey.sourceVersion()));
        selectorData.m_name = name;
        sourceSelectorModel << QVariant::fromValue(selectorData);

        QShaderCode shaderCode = bs.shader(keys[i]);
        switch (shaderKey.source()) {
        case QShader::SpirvShader:
            Q_FALLTHROUGH();
        case QShader::DxbcShader:
            Q_FALLTHROUGH();
        case QShader::DxilShader:
            Q_FALLTHROUGH();
        case QShader::MetalLibShader:
            // For binary shaders show just information
            m_sourceCodes << QString("%1 binary of %2 bytes").arg(selectorData.m_name).arg(shaderCode.shader().size());
            break;
        default:
            m_sourceCodes << shaderCode.shader();
            break;
        }
    }

    // QSB can contain shaders in any order, for our use
    // sort them to alphaberical order in UI.
    struct {
        bool operator()(QVariant a, QVariant b) const {
            ShaderSelectorData t1 = a.value<ShaderSelectorData>();
            ShaderSelectorData t2 = b.value<ShaderSelectorData>();
            return t1.m_name < t2.m_name;
        }
    } selectorDataSort;
    std::sort(sourceSelectorModel.begin(), sourceSelectorModel.end(), selectorDataSort);

    // Add info as the first element, with index -1
    ShaderSelectorData infoSelectorData;
    infoSelectorData.m_sourceIndex = -1;
    infoSelectorData.m_name = "DETAILS";
    sourceSelectorModel.prepend(QVariant::fromValue(infoSelectorData));

    if (sourceSelectorModel.size() != m_sourceSelectorModel.size()) {
        // When the QSB shaders model has changed, select the deltails
        m_sourceSelectorModel = sourceSelectorModel;
        setCurrentSourceIndex(-1);
        Q_EMIT sourceSelectorModelChanged();
    }

    // Source codes and shader data are always updated
    Q_EMIT currentSourceCodeChanged();
    Q_EMIT shaderDataChanged();

    return true;
}
