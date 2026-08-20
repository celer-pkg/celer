// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "materialadapter.h"

#include <QtQuick3D/private/qquick3dshaderutils_p.h>
#include <QtQuick3D/private/qquick3dobject_p.h>

#include <QtQuick3DRuntimeRender/private/qssgrendershadercache_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendercustommaterial_p.h>

#include <QtCore/qdebug.h>
#include <QtCore/qdir.h>
#include <QtCore/QDataStream>

#include <QtCore/qbuffer.h>

#include <QtQuick3DAssetUtils/private/qssgrtutilities_p.h>
#include <QtQuick3DAssetUtils/private/qssgqmlutilities_p.h>

#include <QtGui/QImageReader>

#include "uniformmodel.h"

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace  {
class CustomMaterialExposed : public QQuick3DCustomMaterial
{
public:
    using Dirty = QQuick3DCustomMaterial::Dirty;
    using QQuick3DCustomMaterial::markDirty;
    CustomMaterialExposed() = delete;
};
}

enum class ShaderType
{
    Vertex,
    Fragment
};

static QString getScheme() { return u"q3dres"_s; }
static QString getUserType() { return u"material"_s; }
static constexpr QStringView fileSuffix(ShaderType type) { return (type == ShaderType::Vertex) ? u".vert" : u".frag"; }

static QUrl defaultShaderUrl(ShaderType type)
{
    return QUrl(getScheme() + "://material@editor" + fileSuffix(type).toString());
}

using BuilderPtr = QPointer<MaterialAdapter>;
Q_GLOBAL_STATIC(BuilderPtr, builderInstance);

[[nodiscard]] static BuildMessage parseErrorMessage(const QString &errorMsg)
{
    // "ERROR: :1: '' :  syntax error, unexpected IDENTIFIER"
    const QString head = QString::fromLatin1("ERROR:");
    qint32 lineNr = -1;
    qint32 columnNr = -1;
    QString identifier;
    auto msg = errorMsg;
    if (errorMsg.startsWith(head)) {
        auto pos = head.size();
        auto idx = errorMsg.indexOf(u':', pos);
        if (idx > pos && idx < pos + 16 /* sanity check */) {
            pos = idx;
            idx = errorMsg.indexOf(u':', pos + 1);
            // Line nr
            if (idx > pos && idx < pos + 6 /* sanity check */) {
                auto mid = errorMsg.mid(pos + 1, idx - pos - 1);
                bool ok = false;
                auto v = mid.toInt(&ok);
                if (ok) {
                    lineNr = v;
                    pos = idx;
                }

                // check if we have a symbol (this might be empty)
                idx = errorMsg.indexOf(u'\'', pos + 1);
                if (idx > pos) {
                    pos = idx;
                    idx = errorMsg.indexOf(u'\'', pos + 1);
                    if (idx > pos && idx > pos + 1)
                        identifier = errorMsg.mid(pos + 1, idx - pos - 1);
                }

                // Find the message
                idx = errorMsg.indexOf(u':', pos + 1);
                if (idx > pos)
                    msg = errorMsg.mid(idx + 1).trimmed();
            }
        }
    }
    return BuildMessage{ msg, identifier, lineNr, columnNr, BuildMessage::Status::Error };
}

// NOTE: We're being called from the render thread here...
void MaterialAdapter::bakerStatusCallback(const QByteArray &descKey, QtQuick3DEditorHelpers::ShaderBaker::Status status, const QString &err, QShader::Stage stage)
{
    (void)descKey;
    if (auto that = (*builderInstance)) {
        using namespace QtQuick3DEditorHelpers::ShaderBaker;
        if (status == Status::Success) {
            if (stage == QShader::Stage::VertexStage) {
                auto fileName = (!that->m_vertexUrl.isEmpty()) ? that->m_vertexUrl.path() : QLatin1String("<VERT_BUFFER>");
                that->m_vertexMsg= { BuildMessage{}, fileName, ShaderBuildMessage::Stage::Vertex };
                Q_EMIT that->vertexStatusChanged();
            } else {
                auto fileName = (!that->m_fragUrl.isEmpty()) ? that->m_fragUrl.path() : QLatin1String("<FRAG_BUFFER>");
                that->m_fragmentMsg = { BuildMessage{}, fileName, ShaderBuildMessage::Stage::Fragment };
                Q_EMIT that->fragmentStatusChanged();
            }
        } else if (status == Status::Error) {
            const auto errList = err.split(u'\n');
            if (errList.size() > 0) {
                auto statusMessage = parseErrorMessage(errList.first());
                if (stage == QShader::Stage::VertexStage) {
                    auto fileName = (!that->m_vertexUrl.isEmpty()) ? that->m_vertexUrl.path() : QLatin1String("<VERT_BUFFER>");
                    that->m_vertexMsg = { statusMessage, fileName, ShaderBuildMessage::Stage::Vertex };
                    Q_EMIT that->vertexStatusChanged();
                } else {
                    auto fileName = (!that->m_fragUrl.isEmpty()) ? that->m_fragUrl.path() : QLatin1String("<FRAG_BUFFER>");
                    that->m_fragmentMsg = { statusMessage, fileName, ShaderBuildMessage::Stage::Fragment };
                    Q_EMIT that->fragmentStatusChanged();
                }
    #if 0
                const auto shaderUrl =  (stage == QShader::Stage::VertexStage) ? that->m_vertexUrl : that->m_fragUrl;
                qDebug() << shaderUrl.host() << "=>" << that->m_error;
    #endif
            }
        } else {
            Q_UNREACHABLE();
        }
    }
}

// NOTE: Called from the sync phase.
static bool resolveShader(const QUrl &url, const QQmlContext *context, QByteArray &shaderData, QByteArray &shaderPathKey)
{
    Q_UNUSED(context);
    Q_UNUSED(shaderPathKey);
    if (auto that = (*builderInstance)) {
        if (url.scheme() == getScheme() && url.userInfo() == getUserType()) {
            const auto filenName = url.host();
            if (filenName.endsWith(fileSuffix(ShaderType::Fragment))) {
                shaderData = that->fragmentShader().toUtf8();
                return true;
            }

            if (filenName.endsWith(fileSuffix(ShaderType::Vertex))) {
                shaderData = that->vertexShader().toUtf8();
                return true;
            }
        } else {
            const QUrl loadUrl = context ? context->resolvedUrl(url) : url;
            const auto path = (loadUrl.scheme() == u"qrc") ? QDir::currentPath() + loadUrl.path()
                                                           : loadUrl.path();
            QFile f(path);
            if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                shaderData = f.readAll();
                if (path.endsWith(fileSuffix(ShaderType::Vertex)))
                    that->setVertexShader(shaderData);
                else if (path.endsWith(fileSuffix(ShaderType::Fragment)))
                    that->setFragmentShader(shaderData);
                return true;
            }
        }
    }

    return false;
}

void MaterialAdapter::updateShader(QQuick3DMaterial &target)
{
    if (QQuick3DObjectPrivate::get(&target)->type == QQuick3DObjectPrivate::Type::CustomMaterial) {
        QQuick3DCustomMaterial &material = static_cast<QQuick3DCustomMaterial &>(target);
        // We mark the material as dirty, this will trigger the material to reload the
        // shader source file, which then again will trigger our resolveShader() function.
        CustomMaterialExposed::markDirty(material, CustomMaterialExposed::Dirty::ShaderSettingsDirty);
        CustomMaterialExposed::markDirty(material, CustomMaterialExposed::Dirty::DynamicPropertiesDirty);
    }
}

void MaterialAdapter::updateMaterialDescription(CustomMaterial::Shaders shaders)
{
    // TODO: We might need to make some more clean-up of textures and front-end nodes
    // that are now replaced, but leaving as-is for now.
    auto oldMaterial = m_material;
    if (m_rootNode != nullptr) {
        if (auto v = m_materialDescr.create(*m_rootNode, uniformTable, m_properties, shaders)) {
            m_material = v;
            CustomMaterialExposed::markDirty(*m_material, CustomMaterialExposed::Dirty::ShaderSettingsDirty);
            CustomMaterialExposed::markDirty(*m_material, CustomMaterialExposed::Dirty::DynamicPropertiesDirty);
            Q_EMIT materialChanged();
        }
    }
}

void MaterialAdapter::updateMaterialDescription()
{
    updateMaterialDescription({ defaultShaderUrl(ShaderType::Vertex), defaultShaderUrl(ShaderType::Fragment) });
}

MaterialAdapter::MaterialAdapter(QObject *parent)
    : QObject(parent)
{
    // NOTE (todo?): As-is this means there can only be one ShaderAdapter per process.
    Q_ASSERT((*builderInstance).isNull());
    (*builderInstance) = this;
    QSSGShaderUtils::setResolveFunction(&resolveShader);
    QtQuick3DEditorHelpers::ShaderBaker::setStatusCallback(&bakerStatusCallback);
}

QQuick3DCustomMaterial *MaterialAdapter::material() const
{
    return m_material;
}

QString MaterialAdapter::fragmentShader() const
{
    return m_fragmentShader;
}

void MaterialAdapter::setFragmentShader(const QString &newFragmentShader)
{
    if (m_fragmentShader == newFragmentShader)
        return;

    m_fragmentShader = newFragmentShader;
    emit fragmentShaderChanged();
    setUnsavedChanges(true);

    if (m_material)
        updateShader(*m_material);
}

QString MaterialAdapter::vertexShader() const
{
    return m_vertexShader;
}

void MaterialAdapter::setVertexShader(const QString &newVertexShader)
{
    if (m_vertexShader == newVertexShader)
        return;

    m_vertexShader = newVertexShader;
    emit vertexShaderChanged();
    setUnsavedChanges(true);

    if (m_material)
        updateShader(*m_material);
}

ShaderBuildMessage MaterialAdapter::vertexStatus() const
{
    return m_vertexMsg;
}

ShaderBuildMessage MaterialAdapter::fragmentStatus() const
{
    return m_fragmentMsg;
}

QString MaterialAdapter::importShader(const QUrl &shaderFile)
{
    QString shaderContents;
    QFile file = resolveFileFromUrl(shaderFile);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        shaderContents = file.readAll();
    else
        qWarning() << "Could not open shader file: " << file.fileName();


    return shaderContents;
}

QFile MaterialAdapter::resolveFileFromUrl(const QUrl &fileUrl)
{
    const QQmlContext *context = qmlContext(this);
    const auto resolvedUrl = context ? context->resolvedUrl(fileUrl) : fileUrl;
    const auto qmlSource = QQmlFile::urlToLocalFileOrQrc(resolvedUrl);

    QFileInfo fileInfo(qmlSource);
    QString filePath = fileInfo.canonicalFilePath();
    if (filePath.isEmpty())
        filePath = fileInfo.absoluteFilePath();
    return QFile(filePath);
}

void MaterialAdapter::importFragmentShader(const QUrl &shaderFile)
{
    setFragmentShader(importShader(shaderFile));
}

void MaterialAdapter::importVertexShader(const QUrl &shaderFile)
{
    setVertexShader(importShader(shaderFile));
}

bool MaterialAdapter::save()
{
    if (!m_materialSaveFile.isEmpty())
        return saveMaterial(m_materialSaveFile);
    return false;
}

static const quint32 MATERIAL_MAGIC = 3365961549;
static const quint32 MATERIAL_VERSION = 1;

bool MaterialAdapter::saveMaterial(const QUrl &materialFile)
{
    auto saveFile = resolveFileFromUrl(materialFile);
    if (saveFile.open(QIODevice::WriteOnly)) {
        QDataStream out(&saveFile);
        out.setByteOrder(QDataStream::LittleEndian);
        out.setFloatingPointPrecision(QDataStream::SinglePrecision);
        out.setVersion(QDataStream::Qt_6_3);
        out << MATERIAL_MAGIC << MATERIAL_VERSION;
        out << m_vertexShader;
        out << m_fragmentShader;
        out << int(m_material->srcBlend());
        out << int(m_material->dstBlend());
        out << int(m_material->cullMode());
        out << int(m_material->depthDrawMode());
        out << int(m_material->shadingMode());
        // Uniforms
        out << uniformTable.size();
        for (const auto &uniform : std::as_const(uniformTable))
            out << uniform;
    } else {
        emit errorOccurred();
        return false;
    }

    setUnsavedChanges(false);
    setMaterialSaveFile(materialFile);
    emit postMaterialSaved();
    return true;
}

bool MaterialAdapter::loadMaterial(const QUrl &materialFile)
{
    auto loadFile = resolveFileFromUrl(materialFile);
    if (loadFile.open(QIODevice::ReadOnly)) {
        QDataStream in(&loadFile);
        in.setByteOrder(QDataStream::LittleEndian);
        in.setFloatingPointPrecision(QDataStream::SinglePrecision);
        in.setVersion(QDataStream::Qt_6_3);
        quint32 magic = 0;
        quint32 version = 0;
        in >> magic >> version;
        if (magic != MATERIAL_MAGIC && version < MATERIAL_VERSION)
            return false;
        QString vertexShader;
        QString fragmentShader;
        in >> vertexShader >> fragmentShader;
        setVertexShader(vertexShader);
        setFragmentShader(fragmentShader);
        int sourceBlend;
        int destBlend;
        int cullMode;
        int depthDrawMode;
        int shadingMode;
        in >> sourceBlend >> destBlend >> cullMode >> depthDrawMode >> shadingMode;
        m_material->setSrcBlend(QQuick3DCustomMaterial::BlendMode(sourceBlend));
        m_material->setDstBlend(QQuick3DCustomMaterial::BlendMode(destBlend));
        m_material->setCullMode(QQuick3DMaterial::CullMode(cullMode));
        m_material->setDepthDrawMode(QQuick3DMaterial::DepthDrawMode(depthDrawMode));
        m_material->setShadingMode(QQuick3DCustomMaterial::ShadingMode(shadingMode));
        // Uniforms
        qsizetype uniformsCount = 0;
        in >> uniformsCount;
        uniformTable.clear();
        for (qsizetype i = 0; i < uniformsCount; ++i) {
            CustomMaterial::Uniform uniform = { };
            in >> uniform;
            uniformTable.append(uniform);
        }
        // We have a new table, so update the table model
        if (m_uniformModel) {
            m_uniformModel->setModelData(&uniformTable);
            updateMaterialDescription();
        }
        // Set filename to loaded one
        setMaterialSaveFile(materialFile);
    } else {
        return false;
    }

    setUnsavedChanges(false);

    return true;
}

bool MaterialAdapter::exportQmlComponent(const QUrl &componentFile, const QString &vertName, const QString &fragName)
{
    QFileInfo fi(componentFile.toLocalFile());
    auto filename = fi.fileName();
    if (filename.isEmpty())
        return false;

    // Some sanity checks
    // Ensure the component starts with an upper-case letter.
    const auto firstLetter = filename.at(0);
    if (!firstLetter.isLetter()) {
        qWarning() << "Component name needs to start with an upper-case letter!";
        return false;
    }

    // Assume this is what the user wanted and fix it now.
    if (firstLetter.isLower()) {
        qWarning() << "Component name needs to start with an upper-case letter!";
        filename[0] = firstLetter.toUpper();
    }

    static const auto saveShader = [](const QDir &dir, const QString &filename, const QString &text) {
        const QString savePath = dir.path() + QDir::separator() + filename;
        auto saveFile = QFile(savePath);
        bool ret = false;
        if (saveFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QTextStream out(&saveFile);
            out << text;
            ret = true;
        } else {
            qWarning("Unable to open \'%s\' for writing", qPrintable(savePath));
        }

        return ret;
    };

    static const auto relShaderUrl = [](const QString &name, ShaderType type) {
        QString relPath;
        if (name.size() > 0) {
            auto suffix = fileSuffix(type);
            if (!name.endsWith(suffix))
                relPath = name + suffix.toString();
            else
                relPath = name;
        }

        return QUrl(relPath);
    };

    bool ret = false;
    const auto &dir = fi.dir();
    auto dirPath = dir.path();
    if (!dirPath.isEmpty()) {
        if (m_materialDescr.isValid()) {
            // NOTE: Relative paths. The shaders are exported with the component and we assume they live in the same location.
            CustomMaterial::Shaders shaders = { relShaderUrl(vertName, ShaderType::Vertex), relShaderUrl(fragName, ShaderType::Fragment) };
            const bool vertShaderOk = (m_vertexShader.size() > 0) ? saveShader(dir, shaders.vert.fileName(), m_vertexShader) : true;
            const bool fragShaderOk = (m_fragmentShader.size() > 0) ? saveShader(dir, shaders.frag.fileName(), m_fragmentShader) : true;
            if (vertShaderOk && fragShaderOk) {
                updateMaterialDescription(shaders);
                auto saveFile = QFile(dirPath + QDir::separator() + filename);
                if (saveFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
                    const auto orgPath = QDir::current().path();
                    QDir::setCurrent(dirPath);
                    QTextStream out(&saveFile);
                    out << m_materialDescr;
                    QDir::setCurrent(orgPath);
                    ret = true;
                }
            } else {
                emit errorOccurred();
                ret = false;
            }

            // Re-set the shader urls
            updateMaterialDescription();
        }
    }

    return ret;
}

void MaterialAdapter::reset()
{
    m_properties = CustomMaterial::Properties{};

    if (m_material == nullptr)
        return;

    delete m_material;

    uniformTable = {};

    if (m_uniformModel)
        m_uniformModel->setModelData(&uniformTable);

    // Set Default Shader Templates
    setFragmentShader(QString());
    setVertexShader(QString());

    updateMaterialDescription();
}

UniformModel *MaterialAdapter::uniformModel() const
{
    return m_uniformModel;
}

bool MaterialAdapter::unsavedChanges() const
{
    return m_unsavedChanges;
}

void MaterialAdapter::setUnsavedChanges(bool newUnsavedChanges)
{
    if (m_unsavedChanges == newUnsavedChanges)
        return;
    m_unsavedChanges = newUnsavedChanges;
    emit unsavedChangesChanged();
}

const QUrl &MaterialAdapter::materialSaveFile() const
{
    return m_materialSaveFile;
}

void MaterialAdapter::setMaterialSaveFile(const QUrl &newMaterialSaveFile)
{
    if (m_materialSaveFile == newMaterialSaveFile)
        return;
    m_materialSaveFile = newMaterialSaveFile;
    emit materialSaveFileChanged();
}

QQuick3DNode *MaterialAdapter::rootNode() const
{
    return m_rootNode;
}

void MaterialAdapter::setRootNode(QQuick3DNode *newResourceNode)
{
    if (m_rootNode == newResourceNode)
        return;
    m_rootNode = newResourceNode;
    emit rootNodeChanged();

    updateMaterialDescription();
}

MaterialAdapter::CullMode MaterialAdapter::cullMode() const
{
    return m_properties.cullMode;
}

void MaterialAdapter::setCullMode(CullMode newCullMode)
{
    if (m_properties.cullMode == newCullMode)
        return;
    m_properties.cullMode = newCullMode;
    emit cullModeChanged();

    updateMaterialDescription();
}

MaterialAdapter::DepthDrawMode MaterialAdapter::depthDrawMode() const
{
    return m_properties.depthDrawMode;
}

void MaterialAdapter::setDepthDrawMode(DepthDrawMode newDepthDrawMode)
{
    if (m_properties.depthDrawMode == newDepthDrawMode)
        return;
    m_properties.depthDrawMode = newDepthDrawMode;
    emit depthDrawModeChanged();

    updateMaterialDescription();
}

MaterialAdapter::ShadingMode MaterialAdapter::shadingMode() const
{
    return m_properties.shadingMode;
}

void MaterialAdapter::setShadingMode(ShadingMode newShadingMode)
{
    if (m_properties.shadingMode == newShadingMode)
        return;
    m_properties.shadingMode = newShadingMode;
    emit shadingModeChanged();

    updateMaterialDescription();
}

MaterialAdapter::BlendMode MaterialAdapter::srcBlend() const
{
    return m_properties.sourceBlend;
}

void MaterialAdapter::setSrcBlend(BlendMode newSourceBlend)
{
    if (m_properties.sourceBlend == newSourceBlend)
        return;
    m_properties.sourceBlend = newSourceBlend;
    emit srcBlendChanged();

    updateMaterialDescription();
}

MaterialAdapter::BlendMode MaterialAdapter::dstBlend() const
{
    return m_properties.destinationBlend;
}

void MaterialAdapter::setDstBlend(BlendMode newDestinationBlend)
{
    if (m_properties.destinationBlend == newDestinationBlend)
        return;
    m_properties.destinationBlend = newDestinationBlend;
    emit dstBlendChanged();

    updateMaterialDescription();
}

void MaterialAdapter::setUniformModel(UniformModel *newUniformModel)
{
    m_uniformModel = newUniformModel;
    if (m_uniformModel) {
        m_uniformModel->setModelData(&uniformTable);
        connect(m_uniformModel, &UniformModel::dataChanged, this, [this]() {
            updateMaterialDescription();
        });
    }
    emit uniformModelChanged();
}

QString MaterialAdapter::getSupportedImageFormatsFilter() const
{
    auto formats = QImageReader::supportedImageFormats();
    QString imageFilter = QStringLiteral("Image files (");
    for (const auto &format : std::as_const(formats))
        imageFilter += QStringLiteral("*.") + format + QStringLiteral(" ");
    imageFilter += QStringLiteral(")");
    return imageFilter;
}


QT_END_NAMESPACE
