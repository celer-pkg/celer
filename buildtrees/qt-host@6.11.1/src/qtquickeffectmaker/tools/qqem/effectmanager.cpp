// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "effectmanager.h"
#include "propertyhandler.h"
#include "syntaxhighlighterdata.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QImageReader>
#include <QQmlContext>
#include <QtQml/qqmlfile.h>
#include <QXmlStreamWriter>

QQmlPropertyMap g_argData;

enum class FileType
{
    Binary,
    Text
};

static bool writeToFile(const QByteArray &buf, const QString &filename, FileType fileType)
{
    QDir().mkpath(QFileInfo(filename).path());
    QFile f(filename);
    QIODevice::OpenMode flags = QIODevice::WriteOnly | QIODevice::Truncate;
    if (fileType == FileType::Text)
        flags |= QIODevice::Text;
    if (!f.open(flags)) {
        qWarning() << "Failed to open file for writing:" << filename;
        return false;
    }
    f.write(buf);
    return true;
}

static void removeIfExists(const QString &filePath)
{
    QFile file(filePath);
    if (file.exists())
        file.remove();
}

// Returns the boolean value of QJsonValue. It can be either boolean
// (true, false) or string ("true", "false"). Returns the defaultValue
// if QJsonValue is undefined, empty, or some other type.
static bool getBoolValue(const QJsonValue &jsonValue, bool defaultValue)
{
    bool returnValue = defaultValue;
    if (jsonValue.isBool()) {
        returnValue = jsonValue.toBool();
    } else if (jsonValue.isString()) {
        QString s = jsonValue.toString().toLower();
        if (s == QStringLiteral("true"))
            returnValue = true;
        else if (s == QStringLiteral("false"))
            returnValue = false;
    }
    return returnValue;
}

EffectManager::EffectManager(QObject *parent) : QObject(parent)
{
    m_settings = new ApplicationSettings(this);
    setUniformModel(new UniformModel(this));
    m_addNodeModel = new AddNodeModel(this);
    m_codeHelper = new CodeHelper(this);

    m_vertexShaderFile.setFileTemplate(QDir::tempPath() + "/qqem_XXXXXX.vert.qsb");
    m_fragmentShaderFile.setFileTemplate(QDir::tempPath() + "/qqem_XXXXXX.frag.qsb");
    if (m_vertexShaderFile.open()) {
        m_vertexShaderFilename = m_vertexShaderFile.fileName();
        qInfo() << "Using temporary vs file:" << m_vertexShaderFilename;
    }
    if (m_fragmentShaderFile.open()) {
        m_fragmentShaderFilename = m_fragmentShaderFile.fileName();
        qInfo() << "Using temporary fs file:" << m_fragmentShaderFilename;
    }

    // Prepare baker
    m_baker.setGeneratedShaderVariants({ QShader::StandardShader });
    updateBakedShaderVersions();

    m_shaderBakerTimer.setInterval(1000);
    m_shaderBakerTimer.setSingleShot(true);
    connect(&m_shaderBakerTimer, &QTimer::timeout, this, &EffectManager::doBakeShaders);

    m_settings->updateRecentProjectsModel();

    connect(m_uniformModel, &UniformModel::qmlComponentChanged, this, [this]() {
        updateCustomUniforms();
        updateQmlComponent();
        // Also update the features as QML e.g. might have started using iTime
        m_shaderFeatures.update(generateVertexShader(false), generateFragmentShader(false), m_previewEffectPropertiesString);
    });
    connect(m_uniformModel, &UniformModel::uniformsChanged, this, [this]() {
        updateImageWatchers();
        bakeShaders();
    });
    connect(m_uniformModel, &UniformModel::addFSCode, this, [this](const QString &code) {
        if (auto selectedNode = m_nodeView->m_nodesModel->m_selectedNode) {
            selectedNode->fragmentCode += code;
            Q_EMIT m_nodeView->selectedNodeFragmentCodeChanged();
        }
    });

    connect(&m_fileWatcher, &QFileSystemWatcher::fileChanged, this, [this]() {
        // Update component with images not set.
        m_loadComponentImages = false;
        updateQmlComponent();
        // Then enable component images with a longer delay than
        // the component updating delay. This way Image elements
        // will relaod the changed image files.
        const int enableImagesDelay = effectUpdateDelay() + 100;
        QTimer::singleShot(enableImagesDelay, this, [this]() {
            m_loadComponentImages = true;
            updateQmlComponent();
        } );
    });
}

UniformModel *EffectManager::uniformModel() const
{
    return m_uniformModel;
}

void EffectManager::setUniformModel(UniformModel *newUniformModel)
{
    m_uniformModel = newUniformModel;
    if (m_uniformModel) {
        m_uniformModel->m_effectManager = this;
        m_uniformModel->setModelData(&m_uniformTable);
    }
    emit uniformModelChanged();
}

CodeHelper *EffectManager::codeHelper() const
{
    return m_codeHelper;
}

QString EffectManager::fragmentShader() const
{
    return m_fragmentShader;
}

void EffectManager::setFragmentShader(const QString &newFragmentShader)
{
    if (m_fragmentShader == newFragmentShader)
        return;

    m_fragmentShader = newFragmentShader;
    emit fragmentShaderChanged();
    setUnsavedChanges(true);
}

QString EffectManager::vertexShader() const
{
    return m_vertexShader;
}

void EffectManager::setVertexShader(const QString &newVertexShader)
{
    if (m_vertexShader == newVertexShader)
        return;

    m_vertexShader = newVertexShader;
    emit vertexShaderChanged();
    setUnsavedChanges(true);
}

bool EffectManager::unsavedChanges() const
{
    return m_unsavedChanges;
}

void EffectManager::setUnsavedChanges(bool newUnsavedChanges)
{
    if (m_unsavedChanges == newUnsavedChanges || m_firstBake)
        return;
    m_unsavedChanges = newUnsavedChanges;
    emit unsavedChangesChanged();
}

bool EffectManager::hasProjectFilename() const
{
    return !m_projectFilename.isEmpty();
}

QString EffectManager::projectFilename() const
{
    return m_projectFilename.toString();
}

QString EffectManager::exportFilename() const
{
    return m_exportFilename;
}

QString EffectManager::projectName() const
{
    return m_projectName;
}

void EffectManager::setProjectName(const QString &name)
{
    if (m_projectName == name)
        return;

    m_projectName = name;
    Q_EMIT projectNameChanged();
}

QString EffectManager::projectDirectory() const
{
    return m_projectDirectory;
}

QString EffectManager::exportDirectory() const
{
    return m_exportDirectory;
}

int EffectManager::exportFlags() const
{
    return m_exportFlags;
}

QStringList EffectManager::getDefaultRootVertexShader()
{
    if (m_defaultRootVertexShader.isEmpty()) {
        m_defaultRootVertexShader << "void main() {";
        m_defaultRootVertexShader << "    texCoord = qt_MultiTexCoord0;";
        m_defaultRootVertexShader << "    fragCoord = qt_Vertex.xy;";
        m_defaultRootVertexShader << "    vec2 vertCoord = qt_Vertex.xy;";
        m_defaultRootVertexShader << "    @nodes";
        m_defaultRootVertexShader << "    gl_Position = qt_Matrix * vec4(vertCoord, 0.0, 1.0);";
        m_defaultRootVertexShader << "}";
    }
    return m_defaultRootVertexShader;
}

QStringList EffectManager::getDefaultRootFragmentShader()
{
    if (m_defaultRootFragmentShader.isEmpty()) {
        m_defaultRootFragmentShader << "void main() {";
        m_defaultRootFragmentShader << "    fragColor = texture(iSource, texCoord);";
        m_defaultRootFragmentShader << "    @nodes";
        m_defaultRootFragmentShader << "    fragColor = fragColor * qt_Opacity;";
        m_defaultRootFragmentShader << "}";
    }
    return m_defaultRootFragmentShader;
}

QString EffectManager::processVertexRootLine(const QString &line)
{
    QString output;
    static QRegularExpression spaceReg("\\s+");
    QStringList lineList = line.split(spaceReg, Qt::SkipEmptyParts);
    if (lineList.length() > 1 && lineList.at(0) == QStringLiteral("out")) {
        lineList.removeFirst();
        QString outLine = lineList.join(' ');
        m_shaderVaryingVariables << outLine;
    } else {
        output = line + '\n';
    }
    return output;
}

QString EffectManager::processFragmentRootLine(const QString &line)
{
    QString output;
    static QRegularExpression spaceReg("\\s+");
    QStringList lineList = line.split(spaceReg, Qt::SkipEmptyParts);
    // Just skip all "in" variables. It is enough to have "out" variable in vertex.
    if (lineList.length() > 1 && lineList.at(0) == QStringLiteral("in"))
        return QString();
    output = line + '\n';
    return output;
}

// Outputs the custom varying variables.
// When outState is true, output vertex (out) version, when false output fragment (in) version.
QString EffectManager::getCustomShaderVaryings(bool outState)
{
    QString output;
    QString direction = outState ?  QStringLiteral("out") :  QStringLiteral("in");
    int varLocation = m_shaderFeatures.enabled(ShaderFeatures::FragCoord) ? 2 : 1;
    for (const auto &var : m_shaderVaryingVariables) {
        output += QString("layout(location = %1) %2 %3\n").arg(QString::number(varLocation), direction, var);
        varLocation++;
    }
    return output;
}

// Remove all post-processing tags ("@tag") from the code.
// Except "@nodes" tag as that is handled later.
QStringList EffectManager::removeTagsFromCode(const QStringList &codeLines) {
    QStringList s;
    for (const auto &line : codeLines) {
        const auto trimmedLine = line.trimmed();
        if (!trimmedLine.startsWith('@') || trimmedLine.startsWith("@nodes")) {
            s << line;
        } else {
            // Check if the tag is known
            bool validTag = false;
            auto tags = SyntaxHighlighterData::reservedTagNames();
            static QRegularExpression spaceReg("\\s+");
            QString firstWord = trimmedLine.split(spaceReg, Qt::SkipEmptyParts).first();
            for (const auto &tag : tags) {
                if (firstWord == QString::fromUtf8(tag)) {
                    validTag = true;
                    break;
                }
            }
            if (!validTag)
                setEffectError(QString("Unknown tag: %1").arg(trimmedLine), ErrorPreprocessor);
        }
    }
    return s;
}

QString EffectManager::removeTagsFromCode(const QString &code) {
    QStringList codeLines = removeTagsFromCode(code.split('\n'));
    return codeLines.join('\n');
}


QString EffectManager::generateVertexShader(bool includeUniforms) {
    QString s;

    if (includeUniforms)
        s += getVSUniforms();

    // Remove tags when not generating for features check
    const bool removeTags = includeUniforms;

    s += getDefineProperties();
    s += getConstVariables();

    // When the node is complete, add shader code in correct nodes order
    // split to root and main parts
    QString s_root;
    QString s_main;
    QStringList s_sourceCode;
    m_shaderVaryingVariables.clear();
    if (m_nodeView->nodeGraphComplete()) {
        for (auto n : m_nodeView->m_activeNodesList) {
            if (!n->vertexCode.isEmpty() && !n->disabled) {
                if (n->type == 0) {
                    s_sourceCode = n->vertexCode.split('\n');
                } else if (n->type == 2) {
                    QStringList vertexCode = n->vertexCode.split('\n');
                    int mainIndex = getTagIndex(vertexCode, QStringLiteral("main"));
                    int line = 0;
                    for (const auto &ss : vertexCode) {
                        if (mainIndex == -1 || line > mainIndex)
                            s_main += QStringLiteral("    ") + ss + '\n';
                        else if (line < mainIndex)
                            s_root += processVertexRootLine(ss);
                        line++;
                    }
                }
            }
        }
    }

    if (s_sourceCode.isEmpty()) {
        // If source nodes doesn't contain any code, fail to the default one
        s_sourceCode << getDefaultRootVertexShader();
    }

    if (removeTags) {
        s_sourceCode = removeTagsFromCode(s_sourceCode);
        s_root = removeTagsFromCode(s_root);
        s_main = removeTagsFromCode(s_main);
    }

    s += getCustomShaderVaryings(true);
    s += s_root + '\n';

    int nodesIndex = getTagIndex(s_sourceCode, QStringLiteral("nodes"));
    int line = 0;
    for (const auto &ss : s_sourceCode) {
        if (line == nodesIndex)
            s += s_main;
        else
            s += ss + '\n';
        line++;
    }

    return s;
}

QString EffectManager::generateFragmentShader(bool includeUniforms) {
    QString s;

    if (includeUniforms)
        s += getFSUniforms();

    // Remove tags when not generating for features check
    const bool removeTags = includeUniforms;

    s += getDefineProperties();
    s += getConstVariables();

    // When the node is complete, add shader code in correct nodes order
    // split to root and main parts
    QString s_root;
    QString s_main;
    QStringList s_sourceCode;
    if (m_nodeView->nodeGraphComplete()) {
        for (auto n : m_nodeView->m_activeNodesList) {
            if (!n->fragmentCode.isEmpty() && !n->disabled) {
                if (n->type == 0) {
                    s_sourceCode = n->fragmentCode.split('\n');
                } else if (n->type == 2) {
                    QStringList fragmentCode = n->fragmentCode.split('\n');
                    int mainIndex = getTagIndex(fragmentCode, QStringLiteral("main"));
                    int line = 0;
                    for (const auto &ss : fragmentCode) {
                        if (mainIndex == -1 || line > mainIndex)
                            s_main += QStringLiteral("    ") + ss + '\n';
                        else if (line < mainIndex)
                            s_root += processFragmentRootLine(ss);
                        line++;
                    }
                }
            }
        }
    }

    if (s_sourceCode.isEmpty()) {
        // If source nodes doesn't contain any code, fail to the default one
        s_sourceCode << getDefaultRootFragmentShader();
    }

    if (removeTags) {
        s_sourceCode = removeTagsFromCode(s_sourceCode);
        s_root = removeTagsFromCode(s_root);
        s_main = removeTagsFromCode(s_main);
    }

    s += getCustomShaderVaryings(false);
    s += s_root + '\n';

    int nodesIndex = getTagIndex(s_sourceCode, QStringLiteral("nodes"));
    int line = 0;
    for (const auto &ss : s_sourceCode) {
        if (line == nodesIndex)
            s += s_main;
        else
            s += ss + '\n';
        line++;
    }

    return s;
}

int EffectManager::getTagIndex(const QStringList &code, const QString &tag)
{
    int index = -1;
    int line = 0;
    const QString tagString = QString("@%1").arg(tag);
    for (const auto &s : code) {
        auto st = s.trimmed();
        // Check if line or first non-space content of the line matches to tag
        static auto spaceReg = QRegularExpression("\\s");
        auto firstSpace = st.indexOf(spaceReg);
        QString firstWord = st;
        if (firstSpace > 0)
            firstWord = st.sliced(0, firstSpace);
        if (firstWord == tagString) {
            index = line;
            break;
        }
        line++;
    }
    return index;
}

void EffectManager::updateBakedShaderVersions()
{
    QList<QShaderBaker::GeneratedShader> targets;
    targets.append({ QShader::SpirvShader, QShaderVersion(100) }); // Vulkan 1.0
    targets.append({ QShader::HlslShader, QShaderVersion(50) }); // Shader Model 5.0
    targets.append({ QShader::MslShader, QShaderVersion(12) }); // Metal 1.2
    targets.append({ QShader::GlslShader, QShaderVersion(300, QShaderVersion::GlslEs) }); // GLES 3.0+
    targets.append({ QShader::GlslShader, QShaderVersion(410) }); // OpenGL 4.1+
    targets.append({ QShader::GlslShader, QShaderVersion(330) }); // OpenGL 3.3
    targets.append({ QShader::GlslShader, QShaderVersion(140) }); // OpenGL 3.1
    if (m_settings->useLegacyShaders()) {
        targets.append({ QShader::GlslShader, QShaderVersion(100, QShaderVersion::GlslEs) }); // GLES 2.0
        targets.append({ QShader::GlslShader, QShaderVersion(120) }); // OpenGL 2.1
    }
    m_baker.setGeneratedShaders(targets);
}

// Bake the shaders if they have changed
// When forced is true, will bake even when autoplay is off
void EffectManager::bakeShaders(bool forced)
{
    resetEffectError(ErrorPreprocessor);
    if (m_vertexShader == generateVertexShader()
            && m_fragmentShader == generateFragmentShader()) {
        setShadersUpToDate(true);
        return;
    }

    setShadersUpToDate(false);

    if (forced)
        doBakeShaders();
    else if (m_autoPlayEffect)
        m_shaderBakerTimer.start();
}

void EffectManager::doBakeShaders()
{
    // First update the features based on shader content
    // This will make sure that next calls to generate* will produce correct uniforms.
    m_shaderFeatures.update(generateVertexShader(false), generateFragmentShader(false), m_previewEffectPropertiesString);

    updateCustomUniforms();

    setVertexShader(generateVertexShader());
    QString vs = m_vertexShader;
    m_baker.setSourceString(vs.toUtf8(), QShader::VertexStage);

    QShader vertShader = m_baker.bake();
    if (!vertShader.isValid()) {
        qWarning() << "Shader baking failed:" << qPrintable(m_baker.errorMessage());
        setEffectError(m_baker.errorMessage().split('\n').first(), ErrorVert);
    } else {
        QString filename = m_vertexShaderFile.fileName();
        writeToFile(vertShader.serialized(), filename, FileType::Binary);
        resetEffectError(ErrorVert);
    }

    setFragmentShader(generateFragmentShader());
    QString fs = m_fragmentShader;
    m_baker.setSourceString(fs.toUtf8(), QShader::FragmentStage);

    QShader fragShader = m_baker.bake();
    if (!fragShader.isValid()) {
        qWarning() << "Shader baking failed:" << qPrintable(m_baker.errorMessage());
        setEffectError(m_baker.errorMessage().split('\n').first(), ErrorFrag);
    } else {
        QString filename = m_fragmentShaderFile.fileName();
        writeToFile(fragShader.serialized(), filename, FileType::Binary);
        resetEffectError(ErrorFrag);
    }

    if (vertShader.isValid() && fragShader.isValid()) {
        Q_EMIT shadersBaked();
        setShadersUpToDate(true);
    }
    m_firstBake = false;
}

const QString EffectManager::getBufUniform()
{
    QString s;
    s += "layout(std140, binding = 0) uniform buf {\n";
    s += "    mat4 qt_Matrix;\n";
    s += "    float qt_Opacity;\n";
    if (m_shaderFeatures.enabled(ShaderFeatures::Time))
        s += "    float iTime;\n";
    if (m_shaderFeatures.enabled(ShaderFeatures::Frame))
        s += "    int iFrame;\n";
    if (m_shaderFeatures.enabled(ShaderFeatures::Resolution))
        s += "    vec3 iResolution;\n";
    if (m_shaderFeatures.enabled(ShaderFeatures::Mouse))
        s += "    vec4 iMouse;\n";
    for (auto &uniform : m_uniformTable) {
        if (!m_nodeView->m_activeNodesIds.contains(uniform.nodeId))
            continue;
        if (uniform.exportProperty &&
                uniform.type != UniformModel::Uniform::Type::Sampler &&
                uniform.type != UniformModel::Uniform::Type::Define) {
            QString type = m_uniformModel->typeToUniform(uniform.type);
            QString props = "    " + type + " " + uniform.name + ";\n";
            s += props;
        }
    }
    s += "};\n";
    return s;
}

const QString EffectManager::getVSUniforms()
{
    QString s;
    s += "#version 440\n";
    s += '\n';
    s += "layout(location = 0) in vec4 qt_Vertex;\n";
    s += "layout(location = 1) in vec2 qt_MultiTexCoord0;\n";
    s += "layout(location = 0) out vec2 texCoord;\n";
    if (m_shaderFeatures.enabled(ShaderFeatures::FragCoord))
        s += "layout(location = 1) out vec2 fragCoord;\n";
    s += '\n';
    s += getBufUniform();
    s += '\n';
    s += "out gl_PerVertex { vec4 gl_Position; };\n";
    s += '\n';
    return s;
}

const QString EffectManager::getFSUniforms()
{
    QString s;
    s += "#version 440\n";
    s += '\n';
    s += "layout(location = 0) in vec2 texCoord;\n";
    if (m_shaderFeatures.enabled(ShaderFeatures::FragCoord))
        s += "layout(location = 1) in vec2 fragCoord;\n";
    s += "layout(location = 0) out vec4 fragColor;\n";
    s += '\n';
    s += getBufUniform();
    s += '\n';

    bool usesSource = m_shaderFeatures.enabled(ShaderFeatures::Source);
    if (usesSource)
        s += "layout(binding = 1) uniform sampler2D iSource;\n";

    // Add sampler uniforms
    int bindingIndex = usesSource ? 2 : 1;
    for (auto &uniform : m_uniformTable) {
        if (!m_nodeView->m_activeNodesIds.contains(uniform.nodeId))
            continue;
        if (uniform.type == UniformModel::Uniform::Type::Sampler) {
            // Start index from 2, 1 is source item
            QString props = QString("layout(binding = %1) uniform sampler2D %2").arg(bindingIndex).arg(uniform.name);
            s += props + ";\n";
            bindingIndex++;
        }
    }
    s += '\n';
    if (m_shaderFeatures.enabled(ShaderFeatures::BlurSources)) {
        const int blurItems = 5;
        for (int i = 1; i <= blurItems; i++) {
            QString props = QString("layout(binding = %1) uniform sampler2D iSourceBlur%2")
                    .arg(bindingIndex).arg(QString::number(i));
            s += props + ";\n";
            bindingIndex++;
        }
        s += '\n';
    }
    return s;
}

const QString EffectManager::getDefineProperties()
{
    QString s;
    for (auto &uniform : m_uniformTable) {
        if (!m_nodeView->m_activeNodesIds.contains(uniform.nodeId))
            continue;
        if (uniform.type == UniformModel::Uniform::Type::Define) {
            QString defineValue = uniform.value.toString();
            s += QString("#define %1 %2\n").arg(uniform.name, defineValue);
        }
    }
    if (!s.isEmpty())
        s += '\n';

    return s;
}

const QString EffectManager::getConstVariables()
{
    QString s;
    for (auto &uniform : m_uniformTable) {
        if (!m_nodeView->m_activeNodesIds.contains(uniform.nodeId))
            continue;
        if (!uniform.exportProperty) {
            QString constValue = m_uniformModel->valueAsVariable(uniform);
            QString type = m_uniformModel->typeToUniform(uniform.type);
            s += QString("const %1 %2 = %3;\n").arg(type, uniform.name, constValue);
        }
    }
    if (!s.isEmpty())
        s += '\n';

    return s;
}

// Returns name for image mipmap property.
// e.g. "myImage" -> "myImageMipmap".
QString EffectManager::mipmapPropertyName(const QString &name) const
{
    QString simplifiedName = name.simplified();
    simplifiedName = simplifiedName.remove(' ');
    simplifiedName += "Mipmap";
    return simplifiedName;
}

QString EffectManager::getQmlImagesString(bool localFiles)
{
    QString imagesString;
    for (auto &uniform : m_uniformTable) {
        if (!m_nodeView->m_activeNodesIds.contains(uniform.nodeId))
            continue;
        if (uniform.type == UniformModel::Uniform::Type::Sampler) {
            if (localFiles && !uniform.exportImage)
                continue;
            QString imagePath = uniform.value.toString();
            if (imagePath.isEmpty())
                continue;
            imagesString += "        Image {\n";
            QString simplifiedName = UniformModel::getImageElementName(uniform);
            imagesString += QString("            id: %1\n").arg(simplifiedName);
            imagesString += "            anchors.fill: parent\n";
            // File paths are absolute, return as local when requested
            if (localFiles) {
                QFileInfo fi(imagePath);
                imagePath = fi.fileName();
            }
            if (m_loadComponentImages)
                imagesString += QString("            source: \"%1\"\n").arg(imagePath);
            if (!localFiles) {
                QString mipmapProperty = mipmapPropertyName(uniform.name);
                imagesString += QString("            mipmap: g_propertyData.%1\n").arg(mipmapProperty);
            } else if (uniform.enableMipmap) {
                imagesString += "            mipmap: true\n";
            }
            imagesString += "            visible: false\n";
            imagesString += "        }\n";
        }
    }
    return imagesString;
}

// Generates string of the custom properties (uniforms) into ShaderEffect component
// Also generates QML images elements for samplers.
void EffectManager::updateCustomUniforms()
{
    QString exportedRootPropertiesString;
    QString previewEffectPropertiesString;
    QString exportedEffectPropertiesString;
    for (auto &uniform : m_uniformTable) {
        if (!m_nodeView->m_activeNodesIds.contains(uniform.nodeId))
            continue;
        if (!uniform.exportProperty)
            continue;
        const bool isDefine = uniform.type == UniformModel::Uniform::Type::Define;
        const bool isImage = uniform.type == UniformModel::Uniform::Type::Sampler;
        QString type = m_uniformModel->typeToProperty(uniform.type);
        QString value = m_uniformModel->valueAsString(uniform);
        QString bindedValue = m_uniformModel->valueAsBinding(uniform);
        // When user has set custom uniform value, use it as-is
        if (uniform.useCustomValue) {
            value = uniform.customValue;
            bindedValue = value;
        }
        // Note: Define type properties appear also as QML properties (in preview) in case QML side
        // needs to use them. This is used at least by BlurHelper BLUR_HELPER_MAX_LEVEL.
        QString propertyName = isDefine ? uniform.name.toLower() : uniform.name;
        if (!uniform.useCustomValue && !isDefine && !uniform.description.isEmpty()) {
            // When exporting, add API documentation for properties
            QStringList descriptionLines = uniform.description.split('\n');
            for (const auto &line: std::as_const(descriptionLines)) {
                if (line.trimmed().isEmpty())
                    exportedRootPropertiesString += QStringLiteral("    //\n");
                else
                    exportedRootPropertiesString += QStringLiteral("    // ") + line + '\n';
            }
        }
        QString valueString = value.isEmpty() ? QString() : QString(": %1").arg(value);
        QString bindedValueString = bindedValue.isEmpty() ? QString() : QString(": %1").arg(bindedValue);
        // Custom values are not readonly, others inside the effect can be
        QString readOnly = uniform.useCustomValue ? QString() : QStringLiteral("readonly ");
        previewEffectPropertiesString += "    " + readOnly + "property " + type + " " + propertyName + bindedValueString + '\n';
        // Define type properties are not added into exports
        if (!isDefine) {
            if (uniform.useCustomValue) {
                // Custom values are only inside the effect, with description comments
                if (!uniform.description.isEmpty()) {
                    QStringList descriptionLines = uniform.description.split('\n');
                    for (const auto &line: descriptionLines)
                        exportedEffectPropertiesString += QStringLiteral("        // ") + line + '\n';
                }
                exportedEffectPropertiesString += QStringLiteral("        ") + readOnly + "property " + type + " " + propertyName + bindedValueString + '\n';
            } else {
                // Custom values are not added into root
                if (isImage && !uniform.exportImage) {
                    // When exporting image is disabled, remove value from root property
                    valueString.clear();
                }
                exportedRootPropertiesString += "    property " + type + " " + propertyName + valueString + '\n';
                exportedEffectPropertiesString += QStringLiteral("        ") + readOnly + "property alias " + propertyName + ": rootItem." + uniform.name + '\n';
            }
        }
    }

    // See if any of the properties changed
    if (m_exportedRootPropertiesString != exportedRootPropertiesString
            || m_previewEffectPropertiesString != previewEffectPropertiesString
            || m_exportedEffectPropertiesString != exportedEffectPropertiesString) {
        setUnsavedChanges(true);
        m_exportedRootPropertiesString = exportedRootPropertiesString;
        m_previewEffectPropertiesString = previewEffectPropertiesString;
        m_exportedEffectPropertiesString = exportedEffectPropertiesString;
    }
}

QString EffectManager::getQmlEffectString()
{
    QString s;
    if (!m_effectHeadings.isEmpty()) {
        s += m_effectHeadings;
        s += '\n';
    }
    s += QString("// Created with Qt Quick Effect Maker (version %1), %2\n\n")
            .arg(qApp->applicationVersion(), QDateTime::currentDateTime().toString());
    s += "import QtQuick\n";
    s += '\n';
    s += "Item {\n";
    s += "    id: rootItem\n";
    s += '\n';
    if (m_shaderFeatures.enabled(ShaderFeatures::Source)) {
        s += "    // This is the main source for the effect\n";
        s += "    property Item source: null\n";
    }
    if (m_shaderFeatures.enabled(ShaderFeatures::Time) ||
            m_shaderFeatures.enabled(ShaderFeatures::Frame)) {
        s += "    // Enable this to animate iTime property\n";
        s += "    property bool timeRunning: false\n";
    }
    if (m_shaderFeatures.enabled(ShaderFeatures::Time)) {
        s += "    // When timeRunning is false, this can be used to control iTime manually\n";
        s += "    property real animatedTime: frameAnimation.elapsedTime\n";
    }
    if (m_shaderFeatures.enabled(ShaderFeatures::Frame)) {
        s += "    // When timeRunning is false, this can be used to control iFrame manually\n";
        s += "    property int animatedFrame: frameAnimation.currentFrame\n";
    }
    s += '\n';
    // Custom properties
    if (!m_exportedRootPropertiesString.isEmpty()) {
        s += m_exportedRootPropertiesString;
        s += '\n';
    }
    if (m_shaderFeatures.enabled(ShaderFeatures::Time) ||
            m_shaderFeatures.enabled(ShaderFeatures::Frame)) {
        s += "    FrameAnimation {\n";
        s += "        id: frameAnimation\n";
        s += "        running: rootItem.timeRunning\n";
        s += "    }\n";
        s += '\n';
    }
    if (m_shaderFeatures.enabled(ShaderFeatures::Mouse)) {
        s += "    // Mouse handling for iMouse variable\n";
        s += "    property real _effectMouseX: 0\n";
        s += "    property real _effectMouseY: 0\n";
        s += "    property real _effectMouseZ: 0\n";
        s += "    property real _effectMouseW: 0\n";
        s += "    MouseArea {\n";
        s += "        anchors.fill: parent\n";
        s += "        onPressed: (mouse)=> {\n";
        s += "            _effectMouseX = mouse.x\n";
        s += "            _effectMouseY = mouse.y\n";
        s += "            _effectMouseZ = mouse.x\n";
        s += "            _effectMouseW = mouse.y\n";
        s += "            clickTimer.restart();\n";
        s += "        }\n";
        s += "        onPositionChanged: (mouse)=> {\n";
        s += "            _effectMouseX = mouse.x\n";
        s += "            _effectMouseY = mouse.y\n";
        s += "        }\n";
        s += "        onReleased: (mouse)=> {\n";
        s += "            _effectMouseZ = -(_effectMouseZ)\n";
        s += "        }\n";
        s += "        Timer {\n";
        s += "            id: clickTimer\n";
        s += "            interval: 20\n";
        s += "            onTriggered: {\n";
        s += "                _effectMouseW = -(_effectMouseW)\n";
        s += "            }\n";
        s += "         }\n";
        s += "    }\n";
        s += '\n';
    }
    if (m_shaderFeatures.enabled(ShaderFeatures::BlurSources)) {
        s += "    BlurHelper {\n";
        s += "        id: blurHelper\n";
        s += "        anchors.fill: parent\n";
        int blurMax = 32;
        if (g_propertyData.contains("BLUR_HELPER_MAX_LEVEL"))
            blurMax = g_propertyData["BLUR_HELPER_MAX_LEVEL"].toInt();
        s += QString("        property int blurMax: %1\n").arg(blurMax);
        s += "        property real blurMultiplier: rootItem.blurMultiplier\n";
        s += "    }\n";
    }
    s += getQmlComponentString(true);
    s += "}\n";
    return s;
}

QString EffectManager::getQmlComponentString(bool localFiles)
{
    auto addProperty = [localFiles](const QString &name, const QString &var, const QString &type, bool blurHelper = false)
    {
        if (localFiles) {
            const QString parent = blurHelper ? "blurHelper." : "rootItem.";
            return QString("readonly property alias %1: %2%3\n").arg(name, parent, var);
        } else {
            const QString parent = blurHelper ? "blurHelper." : QString();
            return QString("readonly property %1 %2: %3%4\n").arg(type, name, parent, var);
        }
    };

    QString customImagesString = getQmlImagesString(localFiles);
    QString vertexShaderFilename = "file:///" + m_vertexShaderFilename;
    QString fragmentShaderFilename = "file:///" + m_fragmentShaderFilename;
    QString s;
    QString l1 = localFiles ? QStringLiteral("    ") : QStringLiteral("");
    QString l2 = localFiles ? QStringLiteral("        ") : QStringLiteral("    ");
    QString l3 = localFiles ? QStringLiteral("            ") : QStringLiteral("        ");

    if (!localFiles)
        s += "import QtQuick\n";
    s += l1 + "ShaderEffect {\n";
    if (m_shaderFeatures.enabled(ShaderFeatures::Source))
        s += l2 + addProperty("iSource", "source", "Item");
    if (m_shaderFeatures.enabled(ShaderFeatures::Time))
        s += l2 + addProperty("iTime", "animatedTime", "real");
    if (m_shaderFeatures.enabled(ShaderFeatures::Frame))
        s += l2 + addProperty("iFrame", "animatedFrame", "int");
    if (m_shaderFeatures.enabled(ShaderFeatures::Resolution)) {
        // Note: Pixel ratio is currently always 1.0
        s += l2 + "readonly property vector3d iResolution: Qt.vector3d(width, height, 1.0)\n";
    }
    if (m_shaderFeatures.enabled(ShaderFeatures::Mouse)) {
        s += l2 + "readonly property vector4d iMouse: Qt.vector4d(rootItem._effectMouseX, rootItem._effectMouseY,\n";
        s += l2 + "                                               rootItem._effectMouseZ, rootItem._effectMouseW)\n";
    }
    if (m_shaderFeatures.enabled(ShaderFeatures::BlurSources)) {
        s += l2 + addProperty("iSourceBlur1", "blurSrc1", "Item", true);
        s += l2 + addProperty("iSourceBlur2", "blurSrc2", "Item", true);
        s += l2 + addProperty("iSourceBlur3", "blurSrc3", "Item", true);
        s += l2 + addProperty("iSourceBlur4", "blurSrc4", "Item", true);
        s += l2 + addProperty("iSourceBlur5", "blurSrc5", "Item", true);
    }
    // When used in editor preview component, we need property with value
    // and when in exported component, property with binding to root value.
    if (localFiles)
        s += m_exportedEffectPropertiesString;
    else
        s += m_previewEffectPropertiesString;

    if (!customImagesString.isEmpty())
        s += '\n' + customImagesString;

    // Add here all the custom QML code from nodes
    if (m_nodeView) {
        QString qmlCode;
        for (auto n : m_nodeView->m_activeNodesList) {
            if (!n->disabled && !n->qmlCode.isEmpty()) {
                QString spacing = localFiles ? QStringLiteral("        ") : QStringLiteral("    ");
                qmlCode += QStringLiteral("\n");
                qmlCode += spacing + QString("//%1\n").arg(n->name);
                QStringList qmlLines = n->qmlCode.split("\n");
                for (const auto &line: qmlLines)
                    qmlCode += spacing + line + "\n";
            }
        }
        s += qmlCode;
        if (qmlCode != m_qmlCode) {
            m_qmlCode = qmlCode;
            setUnsavedChanges(true);
        }
    }

    s += '\n';
    s += l2 + "vertexShader: '" + vertexShaderFilename + "'\n";
    s += l2 + "fragmentShader: '" + fragmentShaderFilename + "'\n";
    s += l2 + "anchors.fill: parent\n";
    if (m_shaderFeatures.enabled(ShaderFeatures::GridMesh)) {
        QString gridSize = QString("%1, %2").arg(m_shaderFeatures.m_gridMeshWidth).arg(m_shaderFeatures.m_gridMeshHeight);
        s += l2 + "mesh: GridMesh {\n";
        s += l3 + QString("resolution: Qt.size(%1)\n").arg(gridSize);
        s += l2 + "}\n";
    }
    s += l1 + "}\n";
    return s;
}

void EffectManager::updateQmlComponent() {
    // Clear possible QML runtime errors
    resetEffectError(ErrorQMLRuntime);
    QString s = getQmlComponentString(false);
    setQmlComponentString(s);
}

NodeView *EffectManager::nodeView() const
{
    return m_nodeView;
}

void EffectManager::setNodeView(NodeView *newNodeView)
{
    if (m_nodeView == newNodeView)
        return;
    m_nodeView = newNodeView;
    emit nodeViewChanged();

    if (m_nodeView) {
        m_nodeView->m_effectManager = this;
        connect(m_nodeView, &NodeView::activeNodesListChanged, this, [this]() {
            updateQmlComponent();
            bakeShaders(true);
        });
        connect(m_nodeView, &NodeView::selectedNodeIdChanged, this, [this]() {
            // Update visibility of properties based on the selected node
            m_uniformModel->beginResetModel();
            QList<UniformModel::Uniform>::iterator it = m_uniformModel->m_uniformTable->begin();
            while (it != m_uniformModel->m_uniformTable->end()) {
                if (m_nodeView->m_selectedNodeId == 0)
                    (*it).visible = true;
                else if ((*it).nodeId == m_nodeView->m_selectedNodeId)
                    (*it).visible = true;
                else
                    (*it).visible = false;
                it++;
            }
            m_uniformModel->endResetModel();
        });
    }
}

// This will be called once when nodesview etc. components exist
void EffectManager::initialize()
{
    bool projectOpened = false;
    QString overrideExportPath;
    if (g_argData.contains("export_path")) {
        // Set export path first, so it gets saved if new project is created
        overrideExportPath = g_argData.value("export_path").toString();
        m_exportDirectory = overrideExportPath;
    }

    if (g_argData.contains("effects_project_path")) {
        // Open or create project given as commandline parameter
        QString projectFile = g_argData.value("effects_project_path").toString();
        QString currentPath = QDir::currentPath();
        QString fullFilePath = relativeToAbsolutePath(projectFile, currentPath);
        bool createProject = g_argData.contains("create_project");
        if (createProject) {
            QFileInfo fi(fullFilePath);
            projectOpened = newProject(fi.path(), fi.baseName(), true, false);
        } else {
            projectOpened = loadProject(QUrl{fullFilePath});
        }
    }

    if (!overrideExportPath.isEmpty()) {
        // Re-apply export path as loading the project may have changed it
        m_exportDirectory = overrideExportPath;
        Q_EMIT exportDirectoryChanged();
    }

    if (!projectOpened) {
        // If project not open, reset the node view
        cleanupNodeView();
    }

    QQmlContext *rootContext = QQmlEngine::contextForObject(this);
    if (rootContext) {
        auto *engine = rootContext->engine();
        // Set up QML runtime error handling
        connect(engine, &QQmlEngine::warnings, this, [this](const QList<QQmlError> &warnings) {
            if (warnings.isEmpty())
                return;
            QQmlError error = warnings.first();
            QString errorMessage = error.toString();
            errorMessage.replace(QStringLiteral("<Unknown File>"), QStringLiteral("ERROR: "));
            qInfo() << "QML:" << error.line() << ":" << errorMessage;
            setEffectError(errorMessage, ErrorQMLRuntime, error.line());
        }
        );
    }

    // Select the main node
    m_nodeView->selectSingleNode(0);
    m_nodeView->updateCodeSelectorModel();

    m_nodeView->m_initialized = true;
}

// Detects common GLSL error messages and returns potential
// additional error information related to them.
QString EffectManager::detectErrorMessage(const QString &errorMessage)
{
    QHash<QString, QString> nodeErrors {
        { "'BLUR_HELPER_MAX_LEVEL' : undeclared identifier", "BlurHelper"},
        { "'iSourceBlur1' : undeclared identifier", "BlurHelper"},
        { "'hash23' : no matching overloaded function found", "NoiseHelper" },
        { "'HASH_BOX_SIZE' : undeclared identifier", "NoiseHelper" },
        { "'pseudo3dNoise' : no matching overloaded function found", "NoiseHelper" }
    };

    QString missingNodeError = QStringLiteral("Are you missing a %1 node?\n");
    QHash<QString, QString>::const_iterator i = nodeErrors.constBegin();
    while (i != nodeErrors.constEnd()) {
        if (errorMessage.contains(i.key()))
            return missingNodeError.arg(i.value());
        ++i;
    }
    return QString();
}

// Return first error message (if any)
EffectError EffectManager::effectError() const
{
    for (const auto &e : m_effectErrors) {
        if (!e.m_message.isEmpty())
            return e;
    }
    return EffectError();
}

// Set the effect error message with optional type and lineNumber.
// Type comes from ErrorTypes, defaulting to common errors (-1).
// Note that type must match with UI editor tab index.
void EffectManager::setEffectError(const QString &errorMessage, int type, int lineNumber)
{
    EffectError error;
    error.m_type = type;
    if (type == 1 || type == 2) {
        // For shaders, get the line number from baker output.
        // Which is something like "ERROR: :15: message"
        int glslErrorLineNumber = -1;
        static QRegularExpression spaceReg("\\s+");
        QStringList errorStringList = errorMessage.split(spaceReg, Qt::SkipEmptyParts);
        if (errorStringList.size() >= 2) {
            QString lineString  = errorStringList.at(1).trimmed();
            if (lineString.size() >= 3) {
                // String is ":[linenumber]:", get only the number.
                glslErrorLineNumber = lineString.sliced(1, lineString.size() - 2).toInt();
            }
        }
        error.m_line = glslErrorLineNumber;
    } else {
        // For QML (and others) use given linenumber
        error.m_line = lineNumber;
    }

    QString additionalErrorInfo = detectErrorMessage(errorMessage);
    error.m_message = additionalErrorInfo + errorMessage;
    m_effectErrors.insert(type, error);
    Q_EMIT effectErrorChanged();
}

void EffectManager::resetEffectError(int type)
{
    if (m_effectErrors.contains(type)) {
        m_effectErrors.remove(type);
        Q_EMIT effectErrorChanged();
    }
}

const QString &EffectManager::fragmentShaderFilename() const
{
    return m_fragmentShaderFilename;
}

void EffectManager::setFragmentShaderFilename(const QString &newFragmentShaderFilename)
{
    if (m_fragmentShaderFilename == newFragmentShaderFilename)
        return;
    m_fragmentShaderFilename = newFragmentShaderFilename;
    emit fragmentShaderFilenameChanged();
}

const QString &EffectManager::vertexShaderFilename() const
{
    return m_vertexShaderFilename;
}

void EffectManager::setVertexShaderFilename(const QString &newVertexShaderFilename)
{
    if (m_vertexShaderFilename == newVertexShaderFilename)
        return;
    m_vertexShaderFilename = newVertexShaderFilename;
    emit vertexShaderFilenameChanged();
}

const QString &EffectManager::qmlComponentString() const
{
    return m_qmlComponentString;
}

void EffectManager::setQmlComponentString(const QString &string)
{
    if (m_qmlComponentString == string)
        return;

    m_qmlComponentString = string;
    emit qmlComponentStringChanged();
}

NodesModel::Node EffectManager::loadEffectNode(const QString &filename)
{
    QFile loadFile(filename);

    NodesModel::Node node;

    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open node file.");
        return node;
    }

    if (m_nodeView)
        m_nodeView->initializeNode(node);

    QByteArray loadData = loadFile.readAll();
    QJsonParseError parseError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(loadData, &parseError));
    if (parseError.error != QJsonParseError::NoError) {
        QString error = QString("Error parsing the effect node: %1:").arg(filename);
        QString errorDetails = QString("%1: %2").arg(parseError.offset).arg(parseError.errorString());
        qWarning() << qPrintable(error);
        qWarning() << qPrintable(errorDetails);
        return node;
    }

    QJsonObject json = jsonDoc.object();
    QFileInfo fi(loadFile);
    createNodeFromJson(json, node, true, fi.absolutePath());

    return node;
}

bool EffectManager::addEffectNode(const QString &filename, int startNodeId, int endNodeId)
{
    auto node = loadEffectNode(filename);
    addNodeIntoView(node, startNodeId, endNodeId);

    return true;
}

bool EffectManager::addNodeIntoView(NodesModel::Node &node, int startNodeId, int endNodeId)
{
    m_nodeView->m_nodesModel->beginResetModel();
    m_nodeView->m_arrowsModel->beginResetModel();

    if (startNodeId > -1 && endNodeId > -1) {
        auto n1 = m_nodeView->m_nodesModel->getNodeWithId(startNodeId);
        auto n2 = m_nodeView->m_nodesModel->getNodeWithId(endNodeId);
        if (n1 && n2) {
            // Remove already existing arrow
            for (auto &arrow : m_nodeView->m_arrowsModel->m_arrowsList) {
                if (arrow.endNodeId == endNodeId)
                    m_nodeView->m_arrowsModel->m_arrowsList.removeAll(arrow);
            }
            // Update next nodes
            n1->nextNodeId = node.nodeId;
            node.nextNodeId = n2->nodeId;
            // Add new arrow
            ArrowsModel::Arrow a1{0, 0, 0, 0, n1->nodeId, node.nodeId};
            m_nodeView->m_arrowsModel->m_arrowsList << a1;
            ArrowsModel::Arrow a2{0, 0, 0, 0, node.nodeId, n2->nodeId};
            m_nodeView->m_arrowsModel->m_arrowsList << a2;
            // Position the new node
            float centerX = (n1->x + n1->width / 2.0f + n2->x + n2->width / 2.0f) / 2.0f;
            float centerY = (n1->y + n1->height / 2.0f + n2->y + n2->height / 2.0f) / 2.0f;
            node.x = centerX - node.width / 2.0f;
            node.y = centerY - node.height / 2.0f;
        }
    }

    // Add Node uniforms into uniform model
    for (const auto &u : node.jsonUniforms)
        m_uniformModel->appendUniform(u);

    m_nodeView->m_nodesModel->m_nodesList << node;
    m_nodeView->m_nodesModel->endResetModel();
    m_nodeView->m_arrowsModel->endResetModel();

    m_nodeView->updateArrowsPositions();
    m_nodeView->updateActiveNodesList();
    // Select the newly created node
    m_nodeView->selectSingleNode(node.nodeId);

    setUnsavedChanges(true);

    return true;
}

bool EffectManager::addNodeConnection(int startNodeId, int endNodeId)
{
    auto n1 = m_nodeView->m_nodesModel->getNodeWithId(startNodeId);
    auto n2 = m_nodeView->m_nodesModel->getNodeWithId(endNodeId);
    if (!n1 || !n2) {
        qWarning("Can't connect unknown nodes");
        return false;
    }
    if (n1->nodeId == n2->nodeId) {
        qWarning("Can't connect node with itself");
        return false;
    }
    // Remove already existing arrow
    for (auto &arrow : m_nodeView->m_arrowsModel->m_arrowsList) {
        if (arrow.endNodeId == endNodeId)
            m_nodeView->m_arrowsModel->m_arrowsList.removeAll(arrow);
    }
    // Update next node
    n1->nextNodeId = n2->nodeId;
    // Add new arrow
    ArrowsModel::Arrow a1{0, 0, 0, 0, n1->nodeId, n2->nodeId};
    m_nodeView->m_arrowsModel->m_arrowsList << a1;

    return true;
}

QString EffectManager::codeFromJsonArray(const QJsonArray &codeArray)
{
    QString codeString;
    for (const auto& element : codeArray) {
        codeString += element.toString();
        codeString += '\n';
    }
    codeString.chop(1); // Remove last '\n'
    return codeString;
}

// Parameter fullNode means nodes from files with QEN etc.
// Parameter nodePath is the directory where node is loaded from.
bool EffectManager::createNodeFromJson(const QJsonObject &rootJson, NodesModel::Node &node, bool fullNode, const QString &nodePath)
{
    QJsonObject json;
    if (fullNode) {
        if (!rootJson.contains("QEN")) {
            qWarning("Invalid Node file");
            return false;
        }

        json = rootJson["QEN"].toObject();

        int version = -1;
        if (json.contains("version"))
            version = json["version"].toInt(-1);
        if (version != 1) {
            QString error = QString("Error: Unknown effect node version (%1)").arg(version);
            qWarning() << qPrintable(error);
            setEffectError(error);
            return false;
        }
    } else {
        json = rootJson;
    }

    if (json.contains("name")) {
        node.name = json["name"].toString();
    } else {
        QString error = QString("Error: Node missing a name");
        qWarning() << qPrintable(error);
        setEffectError(error);
        return false;
    }

    // When loading nodes they contain extra data
    if (json.contains("nodeId"))
        node.nodeId = json["nodeId"].toInt();
    if (json.contains("x"))
        node.x = json["x"].toDouble();
    if (json.contains("y"))
        node.y = json["y"].toDouble();
    if (json.contains("disabled"))
        node.disabled = getBoolValue(json["disabled"], false);

    if (m_nodeView) {
        // Update the node size based on its type
        m_nodeView->initializeNodeSize(node);
        node.name = m_nodeView->getUniqueNodeName(node.name);
    }

    node.description = json["description"].toString();

    if (json.contains("fragmentCode") && json["fragmentCode"].isArray())
        node.fragmentCode = codeFromJsonArray(json["fragmentCode"].toArray());
    if (json.contains("vertexCode") && json["vertexCode"].isArray())
        node.vertexCode = codeFromJsonArray(json["vertexCode"].toArray());
    if (json.contains("qmlCode") && json["qmlCode"].isArray())
        node.qmlCode = codeFromJsonArray(json["qmlCode"].toArray());

    if (json.contains("properties") && json["properties"].isArray()) {
        QJsonArray propertiesArray = json["properties"].toArray();
        for (const auto& element : propertiesArray) {
            auto propertyObject = element.toObject();
            UniformModel::Uniform u = {};
            u.nodeId = node.nodeId;
            u.name = propertyObject["name"].toString().toUtf8();
            u.description = propertyObject["description"].toString();
            u.type = m_uniformModel->typeFromString(propertyObject["type"].toString());
            u.exportProperty = getBoolValue(propertyObject["exported"], true);
            QString value, defaultValue, minValue, maxValue;
            defaultValue = propertyObject["defaultValue"].toString();
            if (u.type == UniformModel::Uniform::Type::Sampler) {
                if (!defaultValue.isEmpty())
                    defaultValue = relativeToAbsolutePath(defaultValue, nodePath);
                if (propertyObject.contains("enableMipmap"))
                    u.enableMipmap = getBoolValue(propertyObject["enableMipmap"], false);
                if (propertyObject.contains("exportImage"))
                    u.exportImage = getBoolValue(propertyObject["exportImage"], true);
                // Update the mipmap property
                QString mipmapProperty = mipmapPropertyName(u.name);
                g_propertyData[mipmapProperty] = u.enableMipmap;
            }
            if (propertyObject.contains("value")) {
                value = propertyObject["value"].toString();
                if (u.type == UniformModel::Uniform::Type::Sampler && !value.isEmpty())
                    value = relativeToAbsolutePath(value, nodePath);
            } else {
                // QEN files don't store the current value, so with those use default value
                value = defaultValue;
            }
            u.customValue = propertyObject["customValue"].toString();
            u.useCustomValue = getBoolValue(propertyObject["useCustomValue"], false);
            minValue = propertyObject["minValue"].toString();
            maxValue = propertyObject["maxValue"].toString();
            m_uniformModel->setUniformValueData(&u, value, defaultValue, minValue, maxValue);
            node.jsonUniforms << u;
        }
    }

    return true;
}

bool EffectManager::deleteEffectNodes(QList<int> nodeIds)
{
    if (nodeIds.isEmpty())
        return false;

    m_nodeView->m_nodesModel->beginResetModel();
    m_nodeView->m_arrowsModel->beginResetModel();
    m_uniformModel->beginResetModel();

    for (auto nodeId : nodeIds) {
        auto node = m_nodeView->m_nodesModel->getNodeWithId(nodeId);
        if (!node)
            continue;

        if (node->type != NodesModel::NodeType::CustomNode)
            continue;

        // Remove possibly existing arrows
        {
            QList<ArrowsModel::Arrow>::iterator it = m_nodeView->m_arrowsModel->m_arrowsList.begin();
            while (it != m_nodeView->m_arrowsModel->m_arrowsList.end()) {
                if ((*it).startNodeId == node->nodeId) {
                    // Update nextNode
                    node->nextNodeId = -1;
                    it = m_nodeView->m_arrowsModel->m_arrowsList.erase(it);
                } else if ((*it).endNodeId == node->nodeId) {
                    // Update nextNode
                    if (auto n = m_nodeView->m_nodesModel->getNodeWithId((*it).startNodeId))
                        n->nextNodeId = -1;
                    it = m_nodeView->m_arrowsModel->m_arrowsList.erase(it);
                } else {
                    it++;
                }
            }
        }

        // Remove properties
        {
            QList<UniformModel::Uniform>::iterator it = m_uniformModel->m_uniformTable->begin();
            while (it != m_uniformModel->m_uniformTable->end()) {
                if ((*it).nodeId == nodeId)
                    it = m_uniformModel->m_uniformTable->erase(it);
                else
                    it++;
            }
        }

        // Remove node
        m_nodeView->m_nodesModel->m_nodesList.removeAll(*node);
    }

    m_nodeView->m_nodesModel->endResetModel();
    m_nodeView->m_arrowsModel->endResetModel();
    m_uniformModel->endResetModel();

    m_nodeView->updateActiveNodesList();

    // Select the Main node
    m_nodeView->selectMainNode();

    return true;
}

QString EffectManager::getSupportedImageFormatsFilter() const
{
    auto formats = QImageReader::supportedImageFormats();
    QString imageFilter = QStringLiteral("Image files (");
    for (const auto &format : std::as_const(formats))
        imageFilter += QStringLiteral("*.") + format + QStringLiteral(" ");
    imageFilter += QStringLiteral(")");
    return imageFilter;
}

void EffectManager::cleanupProject()
{
    m_exportFilename.clear();
    Q_EMIT exportFilenameChanged();
    m_exportDirectory.clear();
    Q_EMIT exportDirectoryChanged();
    m_exportFlags = QMLComponent | QSBShaders | Images;
    Q_EMIT exportFlagsChanged();
    // Reset also settings
    setEffectPadding(QRect(0, 0, 0, 0));
    setEffectHeadings(QString());
    clearImageWatchers();
}

QString EffectManager::replaceOldTagsWithNew(const QString &code) {
    QString s = code;
    s = s.replace("//main", "@main");
    s = s.replace("//nodes", "@nodes");
    s = s.replace("//mesh", "@mesh");
    s = s.replace("//blursources", "@blursources");
    return s;
}

bool EffectManager::loadProject(const QUrl &filename)
{
    auto loadFile = resolveFileFromUrl(filename);
    resetEffectError();

    if (!loadFile.open(QIODevice::ReadOnly)) {
        QString error = QString("Couldn't open project file: '%1'").arg(filename.toString());
        qWarning() << qPrintable(error);
        setEffectError(error);
        m_settings->removeRecentProjectsModel(filename.toString());
        return false;
    }

    QByteArray data = loadFile.readAll();
    QJsonParseError parseError;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(data, &parseError));
    if (parseError.error != QJsonParseError::NoError) {
        QString error = QString("Error parsing the project file: %1: %2").arg(parseError.offset).arg(parseError.errorString());
        qWarning() << qPrintable(error);
        setEffectError(error);
        m_settings->removeRecentProjectsModel(filename.toString());
        return false;
    }
    QJsonObject rootJson = jsonDoc.object();
    if (!rootJson.contains("QEP")) {
        QString error = QStringLiteral("Error: Invalid project file");
        qWarning() << qPrintable(error);
        setEffectError(error);
        m_settings->removeRecentProjectsModel(filename.toString());
        return false;
    }

    QJsonObject json = rootJson["QEP"].toObject();

    int version = -1;
    if (json.contains("version"))
        version = json["version"].toInt(-1);

    if (version != 1) {
        QString error = QString("Error: Unknown project version (%1)").arg(version);
        qWarning() << qPrintable(error);
        setEffectError(error);
        m_settings->removeRecentProjectsModel(filename.toString());
        return false;
    }

    // Get the QQEM version this project was saved with.
    // As a number, so we can use it for comparisons.
    // Start with 0.4 as QQM 0.41 was the first version which started
    // saving these version numbers.
    double qqemVersion = 0.4;
    if (json.contains("QQEM")) {
        QString versionString = json["QQEM"].toString();
        bool ok;
        double versionNumber = versionString.toDouble(&ok);
        if (ok) {
            qqemVersion = versionNumber;
        } else {
            QString error = QString("Warning: Invalid QQEM version (%1)").arg(versionString);
            qWarning() << qPrintable(error);
            setEffectError(error);
        }
    }

    // At this point we consider project to be OK, so start cleanup & load
    cleanupProject();
    cleanupNodeView(false);
    m_uniformTable.clear();
    updateCustomUniforms();

    // Update project directory & name
    m_projectFilename = filename;
    Q_EMIT projectFilenameChanged();
    Q_EMIT hasProjectFilenameChanged();

    QFileInfo fi(loadFile);
    m_projectDirectory = fi.path();
    Q_EMIT projectDirectoryChanged();

    setProjectName(fi.baseName());

    m_settings->updateRecentProjectsModel(m_projectName, m_projectFilename.toString());

    // Get export directory & name
    if (json.contains("exportName")) {
        m_exportFilename = json["exportName"].toString();
        Q_EMIT exportFilenameChanged();
    }
    if (json.contains("exportDirectory")) {
        QString exportDirectory = json["exportDirectory"].toString();
        m_exportDirectory = relativeToAbsolutePath(exportDirectory, m_projectDirectory);
        Q_EMIT exportDirectoryChanged();
    }
    if (json.contains("exportFlags")) {
        m_exportFlags = json["exportFlags"].toInt();
        Q_EMIT exportFlagsChanged();
    }

    if (json.contains("settings") && json["settings"].isObject()) {
        QJsonObject settingsObject = json["settings"].toObject();
        // Effect item padding
        QRect padding(0, 0, 0, 0);
        padding.setX(settingsObject["paddingLeft"].toInt());
        padding.setY(settingsObject["paddingTop"].toInt());
        padding.setWidth(settingsObject["paddingRight"].toInt());
        padding.setHeight(settingsObject["paddingBottom"].toInt());
        setEffectPadding(padding);
        // Effect headings
        if (settingsObject.contains("headings") && settingsObject["headings"].isArray())
            setEffectHeadings(codeFromJsonArray(settingsObject["headings"].toArray()));
    }

    if (json.contains("nodes") && json["nodes"].isArray()) {
        QJsonArray nodesArray = json["nodes"].toArray();
        for (const auto& nodeElement : nodesArray) {
            QJsonObject nodeObject = nodeElement.toObject();

            int type = NodesModel::CustomNode;
            if (nodeObject.contains("type"))
                type = nodeObject["type"].toInt();
            if (type == NodesModel::CustomNode) {
                NodesModel::Node node;
                m_nodeView->initializeNode(node);
                createNodeFromJson(nodeObject, node, false, m_projectDirectory);
                addNodeIntoView(node);
            } else {
                // Source / Output node
                int nodeId = nodeObject["nodeId"].toInt();
                auto node = m_nodeView->m_nodesModel->getNodeWithId(nodeId);
                if (node) {
                    node->x = nodeObject["x"].toDouble();
                    node->y = nodeObject["y"].toDouble();
                    if (node->type == NodesModel::SourceNode) {
                        // Source can contain also shaders
                        if (nodeObject.contains("vertexCode") && nodeObject["vertexCode"].isArray())
                            node->vertexCode = codeFromJsonArray(nodeObject["vertexCode"].toArray());
                        else
                            node->vertexCode = getDefaultRootVertexShader().join('\n');
                        if (nodeObject.contains("fragmentCode") && nodeObject["fragmentCode"].isArray())
                            node->fragmentCode = codeFromJsonArray(nodeObject["fragmentCode"].toArray());
                        else
                            node->fragmentCode = getDefaultRootFragmentShader().join('\n');
                        // And QML
                        if (nodeObject.contains("qmlCode") && nodeObject["qmlCode"].isArray())
                            node->qmlCode = codeFromJsonArray(nodeObject["qmlCode"].toArray());
                        Q_EMIT m_nodeView->selectedNodeFragmentCodeChanged();
                        Q_EMIT m_nodeView->selectedNodeVertexCodeChanged();
                        Q_EMIT m_nodeView->selectedNodeQmlCodeChanged();
                    }
                }
            }
        }
    }

    if (json.contains("connections") && json["connections"].isArray()) {
        QJsonArray connectionsArray = json["connections"].toArray();
        for (const auto& connectionElement : connectionsArray) {
            QJsonObject connectionObject = connectionElement.toObject();
            int fromId = connectionObject["fromId"].toInt();
            int toId = connectionObject["toId"].toInt();
            addNodeConnection(fromId, toId);
        }
    }

    // Replace old tags ("//nodes") with new format ("@nodes")
    // Projects saved with version <= 0.40 use the old tags format
    if (qqemVersion <= 0.4) {
        m_nodeView->m_nodesModel->beginResetModel();
        for (auto &node : m_nodeView->m_nodesModel->m_nodesList) {
            node.vertexCode = replaceOldTagsWithNew(node.vertexCode);
            node.fragmentCode = replaceOldTagsWithNew(node.fragmentCode);
            if (node.selected) {
                Q_EMIT m_nodeView->selectedNodeVertexCodeChanged();
                Q_EMIT m_nodeView->selectedNodeFragmentCodeChanged();
            }
        }
        m_nodeView->m_nodesModel->endResetModel();
    }

    m_nodeView->updateActiveNodesList();
    // Layout nodes automatically to suit current view size
    // But wait that we are definitely in the design mode
    QTimer::singleShot(1, m_nodeView, [this]() {
        m_nodeView->layoutNodes(false);
    } );

    setUnsavedChanges(false);

    return true;
}

bool EffectManager::saveProject(const QUrl &filename)
{
    QUrl fileUrl = filename;
    // When this is called without a filename, use previous one
    if (filename.isEmpty())
        fileUrl = m_projectFilename;

    auto saveFile = resolveFileFromUrl(fileUrl);

    if (!saveFile.open(QIODevice::WriteOnly)) {
        QString error = QString("Error: Couldn't save project file: '%1'").arg(fileUrl.toString());
        qWarning() << qPrintable(error);
        setEffectError(error);
        return false;
    }

    m_projectFilename = fileUrl;
    Q_EMIT projectFilenameChanged();
    Q_EMIT hasProjectFilenameChanged();

    QFileInfo fi(saveFile);
    setProjectName(fi.baseName());

    QJsonObject json;
    // File format version
    json.insert("version", 1);
    // QQEM version
    json.insert("QQEM", qApp->applicationVersion());

    // Add project settings
    QJsonObject settingsObject;
    if (m_effectPadding.x() != 0)
        settingsObject.insert("paddingLeft", m_effectPadding.x());
    if (m_effectPadding.y() != 0)
        settingsObject.insert("paddingTop", m_effectPadding.y());
    if (m_effectPadding.width() != 0)
        settingsObject.insert("paddingRight", m_effectPadding.width());
    if (m_effectPadding.height() != 0)
        settingsObject.insert("paddingBottom", m_effectPadding.height());
    if (!m_effectHeadings.isEmpty()) {
        QJsonArray headingsArray;
        QStringList hLines = m_effectHeadings.split('\n');
        for (const auto &line: std::as_const(hLines))
            headingsArray.append(line);

        if (!headingsArray.isEmpty())
            settingsObject.insert("headings", headingsArray);
    }
    if (!settingsObject.isEmpty())
        json.insert("settings", settingsObject);

    // Add export directory & name
    if (!m_exportFilename.isEmpty())
        json.insert("exportName", m_exportFilename);
    if (!m_exportDirectory.isEmpty()) {
        // Export directory is stored as a relative path.
        QString relativeExportPath = absoluteToRelativePath(m_exportDirectory, m_projectDirectory);
        json.insert("exportDirectory", relativeExportPath);
    }
    json.insert("exportFlags", m_exportFlags);

    // Add nodes
    QJsonArray nodesArray;
    for (const auto &node : m_nodeView->m_nodesModel->m_nodesList) {
        QJsonObject nodeObject = nodeToJson(node, false, fi.absolutePath());
        nodesArray.append(nodeObject);
    }
    if (!nodesArray.isEmpty())
        json.insert("nodes", nodesArray);

    // Add connections
    QJsonArray connectionsArray;
    for (const auto &arrow : m_nodeView->m_arrowsModel->m_arrowsList) {
        QJsonObject arrowObject;
        arrowObject.insert("fromId", arrow.startNodeId);
        arrowObject.insert("toId", arrow.endNodeId);
        // Add connection into array
        connectionsArray.append(arrowObject);
    }
    if (!connectionsArray.isEmpty())
        json.insert("connections", connectionsArray);

    QJsonObject rootJson;
    rootJson.insert("QEP", json);
    QJsonDocument jsonDoc(rootJson);
    saveFile.write(jsonDoc.toJson());

    setUnsavedChanges(false);

    if (!filename.isEmpty()) {
        // When called with filename (so initial save or save as),
        // add into recent projects list.
        m_settings->updateRecentProjectsModel(m_projectName, m_projectFilename.toString());
    }

    return true;
}

// Takes in absolute path (e.g. "file:///C:/myimages/steel1.jpg") and
// path to convert to (e.g. "C:/qqem/defaultnodes".
// Retuns relative path (e.g. "../myimages/steel1.jpg")
QString EffectManager::absoluteToRelativePath(const QString &path, const QString &toPath) {
    if (path.isEmpty())
        return QString();
    QUrl url(path);
    QString filePath = (url.scheme() == QStringLiteral("file")) ? url.toLocalFile() : url.toString();
    QDir dir(toPath);
    QString localPath = dir.relativeFilePath(filePath);
    return localPath;
}

// Takes in path relative to project path (e.g. "../myimages/steel1.jpg") and
// path to convert to (e.g. "C:/qqem/defaultnodes".
// Returns absolute path (e.g. "file:///C:/qqem/myimages/steel1.jpg")
QString EffectManager::relativeToAbsolutePath(const QString &path, const QString &toPath) {
    QString filePath = path;
    QDir dir(toPath);
    QString absPath = dir.absoluteFilePath(filePath);
    absPath = QDir::cleanPath(absPath);
    absPath = QUrl::fromLocalFile(absPath).toString();
    return absPath;
}

// Removes "file:" from the URL path.
// So e.g. "file:///C:/myimages/steel1.jpg" -> "C:/myimages/steel1.jpg"
QString EffectManager::stripFileFromURL(const QString &urlString) const
{
    QUrl url(urlString);
    QString filePath = (url.scheme() == QStringLiteral("file")) ? url.toLocalFile() : url.toString();
    return filePath;
}

// Adds "file:" scheme to the URL path.
// So e.g. "C:/myimages/steel1.jpg" -> "file:///C:/myimages/steel1.jpg"
QString EffectManager::addFileToURL(const QString &urlString) const
{
    if (!urlString.startsWith("file:"))
        return QStringLiteral("file:///") + urlString;
    return urlString;
}

// Returns absolute directory of the path.
// When useFileScheme is true, "file:" scheme is added into result.
// e.g. "file:///C:/temp/temp.txt" -> "file:///C:/temp"
QString EffectManager::getDirectory(const QString &path, bool useFileScheme) const
{
    QString filePath = stripFileFromURL(path);
    QFileInfo fi(filePath);
    QString dir = fi.canonicalPath();
    if (useFileScheme)
        dir = addFileToURL(dir);
    return dir;
}

QString EffectManager::getDefaultImagesDirectory(bool useFileScheme) const
{
    QString dir = m_settings->defaultResourcePath() + QStringLiteral("/defaultnodes/images");
    if (useFileScheme)
        dir = addFileToURL(dir);
    return dir;
}

// Creates JSON presentation of the \a node.
// When simplified is true, temporary UI data is ignored (position, nodeId etx.)
// For projects these are saved, for node components not.
QJsonObject EffectManager::nodeToJson(const NodesModel::Node &node, bool simplified, const QString &nodePath)
{
    QJsonObject nodeObject;
    nodeObject.insert("name", node.name);
    if (!node.description.isEmpty())
        nodeObject.insert("description", node.description);
    if (!simplified) {
        nodeObject.insert("type", node.type);
        nodeObject.insert("nodeId", node.nodeId);
        nodeObject.insert("x", node.x);
        nodeObject.insert("y", node.y);
        if (node.disabled)
            nodeObject.insert("disabled", true);
    } else {
        nodeObject.insert("version", 1);
    }
    // Add properties
    QJsonArray propertiesArray;
    for (auto &uniform : m_uniformTable) {
        if (uniform.nodeId != node.nodeId)
            continue;
        QJsonObject uniformObject;
        uniformObject.insert("name", QString(uniform.name));
        QString type = m_uniformModel->stringFromType(uniform.type);
        uniformObject.insert("type", type);
        if (!simplified) {
            QString value = m_uniformModel->variantAsDataString(uniform.type, uniform.value);
            if (uniform.type == UniformModel::Uniform::Type::Sampler)
                value = absoluteToRelativePath(value, nodePath);
            uniformObject.insert("value", value);
        }
        QString defaultValue = m_uniformModel->variantAsDataString(uniform.type, uniform.defaultValue);
        if (uniform.type == UniformModel::Uniform::Type::Sampler) {
            defaultValue = absoluteToRelativePath(defaultValue, nodePath);
            if (uniform.enableMipmap)
                uniformObject.insert("enableMipmap", uniform.enableMipmap);
            if (!uniform.exportImage)
                uniformObject.insert("exportImage", false);
        }
        uniformObject.insert("defaultValue", defaultValue);
        if (!uniform.description.isEmpty())
            uniformObject.insert("description", uniform.description);
        if (uniform.type == UniformModel::Uniform::Type::Float
                || uniform.type == UniformModel::Uniform::Type::Int
                || uniform.type == UniformModel::Uniform::Type::Vec2
                || uniform.type == UniformModel::Uniform::Type::Vec3
                || uniform.type == UniformModel::Uniform::Type::Vec4) {
            uniformObject.insert("minValue", m_uniformModel->variantAsDataString(uniform.type, uniform.minValue));
            uniformObject.insert("maxValue", m_uniformModel->variantAsDataString(uniform.type, uniform.maxValue));
        }
        if (!uniform.customValue.isEmpty())
            uniformObject.insert("customValue", uniform.customValue);
        if (uniform.useCustomValue)
            uniformObject.insert("useCustomValue", true);

        if (!uniform.exportProperty)
            uniformObject.insert("exported", false);

        propertiesArray.append(uniformObject);
    }
    if (!propertiesArray.isEmpty())
        nodeObject.insert("properties", propertiesArray);

    // Add shaders
    if (!node.fragmentCode.trimmed().isEmpty()) {
        QJsonArray fragmentCodeArray;
        QStringList fsLines = node.fragmentCode.split('\n');
        for (const auto &line: fsLines)
            fragmentCodeArray.append(line);

        if (!fragmentCodeArray.isEmpty())
            nodeObject.insert("fragmentCode", fragmentCodeArray);
    }
    if (!node.vertexCode.trimmed().isEmpty()) {
        QJsonArray vertexCodeArray;
        QStringList vsLines = node.vertexCode.split('\n');
        for (const auto &line: vsLines)
            vertexCodeArray.append(line);

        if (!vertexCodeArray.isEmpty())
            nodeObject.insert("vertexCode", vertexCodeArray);
    }
    // Add QML code
    if (!node.qmlCode.trimmed().isEmpty()) {
        QJsonArray qmlCodeArray;
        QStringList qmlLines = node.qmlCode.split('\n');
        for (const auto &line: qmlLines)
            qmlCodeArray.append(line);

        if (!qmlCodeArray.isEmpty())
            nodeObject.insert("qmlCode", qmlCodeArray);
    }

    if (simplified) {
        QJsonObject rootJson;
        rootJson.insert("QEN", nodeObject);
        return rootJson;
    }
    return nodeObject;
}

bool EffectManager::newProject(const QString &filepath, const QString &filename, bool clearNodeView, bool createProjectDir)
{
    if (filepath.isEmpty()) {
        qWarning("No path");
        return false;
    }

    if (filename.isEmpty()) {
        qWarning("No filename");
        return false;
    }

    if (clearNodeView) {
        resetEffectError();
        cleanupProject();
        cleanupNodeView(true);
    }

    QString dirPath = filepath;

    if (createProjectDir)
        dirPath += "/" + filename;

    // Make sure that dir exists
    QDir dir(dirPath);
    if (!dir.exists())
        dir.mkpath(".");

    m_projectDirectory = dirPath;
    Q_EMIT projectDirectoryChanged();

    // Create project file
    QString projectFilename;
    if (!dirPath.startsWith("file:"))
        projectFilename += "file:///";
    projectFilename += dirPath + "/" + filename + ".qep";
    m_projectFilename = QUrl{projectFilename};
    Q_EMIT projectFilenameChanged();
    Q_EMIT hasProjectFilenameChanged();

    m_nodeView->updateActiveNodesList();
    // Layout nodes automatically to suit current view size
    // But wait that we are definitely in the design mode
    QTimer::singleShot(1, m_nodeView, [this]() {
        m_nodeView->layoutNodes(false);
    } );

    // Save the new project, with whatever nodes user had at this point
    saveProject(m_projectFilename);

    return true;
}

void EffectManager::closeProject()
{
    cleanupProject();
    cleanupNodeView(true);

    // Update project directory & name
    m_projectFilename.clear();
    Q_EMIT projectFilenameChanged();
    Q_EMIT hasProjectFilenameChanged();

    m_projectDirectory.clear();
    Q_EMIT projectDirectoryChanged();

    setProjectName(QString());
}

bool EffectManager::exportEffect(const QString &dirPath, const QString &filename, int exportFlags, int qsbVersionIndex)
{
    if (dirPath.isEmpty() || filename.isEmpty()) {
        QString error = QString("Error: Couldn't export the effect: '%1/%2'").arg(dirPath, filename);
        qWarning() << qPrintable(error);
        setEffectError(error);
        return false;
    }

    // Update export filename & path
    m_exportFilename = filename;
    Q_EMIT exportFilenameChanged();
    m_exportDirectory = dirPath;
    Q_EMIT exportDirectoryChanged();

    // Make sure that uniforms are up-to-date
    updateCustomUniforms();

    // Make sure that dir exists
    QDir dir(dirPath);
    if (!dir.exists())
        dir.mkpath(".");

    QString qmlFilename = filename + ".qml";
    QString vsFilename = filename + ".vert.qsb";
    QString fsFilename = filename + ".frag.qsb";
    QString vsSourceFilename = filename + ".vert";
    QString fsSourceFilename = filename + ".frag";
    QString qrcFilename = filename + ".qrc";
    QStringList exportedFilenames;

    // Make sure that QML component starts with capital letter
    qmlFilename[0] = qmlFilename[0].toUpper();
    // Shaders & qrc on the other hand will be all lowercase
    vsFilename = vsFilename.toLower();
    fsFilename = fsFilename.toLower();
    vsSourceFilename = vsSourceFilename.toLower();
    fsSourceFilename = fsSourceFilename.toLower();
    qrcFilename = qrcFilename.toLower();

    // Bake shaders with correct settings
    if (exportFlags & QSBShaders) {
        auto qsbVersion = QShader::SerializedFormatVersion::Latest;
        if (qsbVersionIndex == 1)
            qsbVersion = QShader::SerializedFormatVersion::Qt_6_5;
        else if (qsbVersionIndex == 2)
            qsbVersion = QShader::SerializedFormatVersion::Qt_6_4;

        QString vsFilePath = dirPath + "/" + vsFilename;
        removeIfExists(vsFilePath);
        m_baker.setSourceString(m_vertexShader.toUtf8(), QShader::VertexStage);
        QShader vertShader = m_baker.bake();
        if (vertShader.isValid())
            writeToFile(vertShader.serialized(qsbVersion), vsFilePath, FileType::Binary);

        exportedFilenames << vsFilename;
        QString fsFilePath = dirPath + "/" + fsFilename;
        removeIfExists(fsFilePath);
        m_baker.setSourceString(m_fragmentShader.toUtf8(), QShader::FragmentStage);
        QShader fragShader = m_baker.bake();
        if (fragShader.isValid())
            writeToFile(fragShader.serialized(qsbVersion), fsFilePath, FileType::Binary);

        exportedFilenames << fsFilename;
    }

    // Copy images
    if (exportFlags & Images) {
        for (auto &uniform : m_uniformTable) {
            if (uniform.type == UniformModel::Uniform::Type::Sampler &&
                !uniform.value.toString().isEmpty() &&
                uniform.exportImage) {
                QString imagePath = uniform.value.toString();
                QFileInfo fi(imagePath);
                QString imageFilename = fi.fileName();
                QString imageFilePath = dirPath + "/" + imageFilename;
                imagePath = stripFileFromURL(imagePath);
                if (imagePath.compare(imageFilePath, Qt::CaseInsensitive) == 0)
                    continue; // Exporting to same dir, so skip
                removeIfExists(imageFilePath);
                QFile imageFile(imagePath);
                if (!imageFile.copy(imageFilePath))
                    qWarning("Unable to copy image file: %s", qPrintable(imageFilename));
                else
                    exportedFilenames << imageFilename;
            }
        }
    }

    if (exportFlags & QMLComponent) {
        QString qmlComponentString = getQmlEffectString();
        QStringList qmlStringList = qmlComponentString.split('\n');

        // Replace shaders with local versions
        for (int i = 1; i < qmlStringList.size(); i++) {
            QString line = qmlStringList.at(i).trimmed();
            if (line.startsWith("vertexShader")) {
                QString vsLine = "        vertexShader: '" + vsFilename + "'";
                qmlStringList[i] = vsLine;
            } else  if (line.startsWith("fragmentShader")) {
                QString fsLine = "        fragmentShader: '" + fsFilename + "'";
                qmlStringList[i] = fsLine;
            }
        }

        QString qmlString = qmlStringList.join('\n');
        QString qmlFilePath = dirPath + "/" + qmlFilename;
        writeToFile(qmlString.toUtf8(), qmlFilePath, FileType::Text);
        exportedFilenames << qmlFilename;

        // Copy blur helpers
        if (m_shaderFeatures.enabled(ShaderFeatures::BlurSources)) {
            QString blurHelperFilename("BlurHelper.qml");
            QString blurFsFilename("bluritems.frag.qsb");
            QString blurVsFilename("bluritems.vert.qsb");
            QString blurHelperPath(m_settings->defaultResourcePath() + "/defaultnodes/common/");
            QString blurHelperSource(blurHelperPath + blurHelperFilename);
            QString blurFsSource(blurHelperPath + blurFsFilename);
            QString blurVsSource(blurHelperPath + blurVsFilename);
            QFile blurHelperFile(blurHelperSource);
            QFile blurFsFile(blurFsSource);
            QFile blurVsFile(blurVsSource);
            QString blurHelperFilePath = dirPath + "/" + "BlurHelper.qml";
            QString blurFsFilePath = dirPath + "/" + "bluritems.frag.qsb";
            QString blurVsFilePath = dirPath + "/" + "bluritems.vert.qsb";
            removeIfExists(blurHelperFilePath);
            removeIfExists(blurFsFilePath);
            removeIfExists(blurVsFilePath);
            if (!blurHelperFile.copy(blurHelperFilePath))
                qWarning("Unable to copy file: %s", qPrintable(blurHelperFilePath));
            if (!blurFsFile.copy(blurFsFilePath))
                qWarning("Unable to copy file: %s", qPrintable(blurFsFilePath));
            if (!blurVsFile.copy(blurVsFilePath))
                qWarning("Unable to copy file: %s", qPrintable(blurVsFilePath));
            exportedFilenames << blurHelperFilename;
            exportedFilenames << blurFsFilename;
            exportedFilenames << blurVsFilename;
        }
    }

    // Export shaders as plain-text
    if (exportFlags & TextShaders) {
        QString vsSourceFilePath = dirPath + "/" + vsSourceFilename;
        writeToFile(m_vertexShader.toUtf8(), vsSourceFilePath, FileType::Text);
        QString fsSourceFilePath = dirPath + "/" + fsSourceFilename;
        writeToFile(m_fragmentShader.toUtf8(), fsSourceFilePath, FileType::Text);
    }

    if (exportFlags & QRCFile) {
        QString qrcXmlString;
        QString qrcFilePath = dirPath + "/" + qrcFilename;
        QXmlStreamWriter stream(&qrcXmlString);
        stream.setAutoFormatting(true);
        stream.writeStartElement("RCC");
        stream.writeStartElement("qresource");
        stream.writeAttribute("prefix", "/");
        for (const auto &filename : exportedFilenames)
            stream.writeTextElement("file", filename);
        stream.writeEndElement(); // qresource
        stream.writeEndElement(); // RCC
        writeToFile(qrcXmlString.toUtf8(), qrcFilePath, FileType::Text);
    }

    // Update exportFlags
    if (m_exportFlags != exportFlags) {
        m_exportFlags = exportFlags;
        Q_EMIT exportFlagsChanged();
    }
    return true;
}

QFile EffectManager::resolveFileFromUrl(const QUrl &fileUrl)
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

void EffectManager::cleanupNodeView(bool initialize)
{
    QList<int> nodes;
    for (const auto &node : m_nodeView->m_nodesModel->m_nodesList) {
        if (node.type == 2)
            nodes << node.nodeId;
    }
    deleteEffectNodes(nodes);

    // Clear also arrows, so source->output connection is removed
    m_nodeView->m_arrowsModel->m_arrowsList.clear();

    if (initialize) {
        // Add first connection
        addNodeConnection(0, 1);
        // Add default root shaders
        int nodeId = 0;
        auto node = m_nodeView->m_nodesModel->getNodeWithId(nodeId);
        if (node) {
            node->vertexCode = getDefaultRootVertexShader().join('\n');
            node->fragmentCode = getDefaultRootFragmentShader().join('\n');
            node->qmlCode.clear();
            m_nodeView->selectMainNode();
            Q_EMIT m_nodeView->selectedNodeFragmentCodeChanged();
            Q_EMIT m_nodeView->selectedNodeVertexCodeChanged();
            Q_EMIT m_nodeView->selectedNodeQmlCodeChanged();
        }
    }
    m_nodeView->updateArrowsPositions();
    m_nodeView->updateActiveNodesList();
}

bool EffectManager::saveSelectedNode(const QUrl &filename)
{
    QUrl fileUrl = filename;

    auto saveFile = resolveFileFromUrl(fileUrl);

    if (!saveFile.open(QIODevice::WriteOnly)) {
        QString error = QString("Error: Couldn't save node file: '%1'").arg(fileUrl.toString());
        qWarning() << qPrintable(error);
        setEffectError(error);
        return false;
    }

    auto node = m_nodeView->m_nodesModel->m_selectedNode;
    if (!node) {
        QString error = QStringLiteral("Error: No node selected'");
        qWarning() << qPrintable(error);
        setEffectError(error);
        return false;
    }

    QFileInfo fi(saveFile);
    QJsonObject nodeObject = nodeToJson(*node, true, fi.absolutePath());
    QJsonDocument jsonDoc(nodeObject);
    saveFile.write(jsonDoc.toJson());

    return true;
}

bool EffectManager::shadersUpToDate() const
{
    return m_shadersUpToDate;
}

void EffectManager::setShadersUpToDate(bool upToDate)
{
    if (m_shadersUpToDate == upToDate)
        return;
    m_shadersUpToDate = upToDate;
    Q_EMIT shadersUpToDateChanged();
}

bool EffectManager::autoPlayEffect() const
{
    return m_autoPlayEffect;
}

void EffectManager::setAutoPlayEffect(bool autoPlay)
{
    if (m_autoPlayEffect == autoPlay)
        return;
    m_autoPlayEffect = autoPlay;
    Q_EMIT autoPlayEffectChanged();
}

QString EffectManager::getHelpTextString()
{
    QFile helpFile(":/qqem_help.html");

    if (!helpFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open help file.");
        return QString();
    }

    QByteArray helpData = helpFile.readAll();
    return QString::fromLatin1(helpData);
}

AddNodeModel *EffectManager::addNodeModel() const
{
    return m_addNodeModel;
}

// This is called when the AddNodeDialog is shown.
void EffectManager::updateAddNodeData()
{
    if (m_addNodeModel && m_nodeView) {
        updateQmlComponent();
        // Collect already used properties
        QStringList existingPropertyNames;
        for (auto &uniform : m_uniformTable)
            existingPropertyNames.append(uniform.name);
        m_addNodeModel->updateCanBeAdded(existingPropertyNames);
    }
}

void EffectManager::showHideAddNodeGroup(const QString &groupName, bool show)
{
    if (m_addNodeModel)
        m_addNodeModel->updateShowHide(groupName, show);
}

void EffectManager::refreshAddNodesList()
{
    if (m_addNodeModel)
        m_addNodeModel->updateNodesList();
}

const QRect &EffectManager::effectPadding() const
{
    return m_effectPadding;
}

void EffectManager::setEffectPadding(const QRect &newEffectPadding)
{
    if (m_effectPadding == newEffectPadding)
        return;
    m_effectPadding = newEffectPadding;
    Q_EMIT effectPaddingChanged();
}

QString EffectManager::effectHeadings() const
{
    return m_effectHeadings;
}

void EffectManager::setEffectHeadings(const QString &newEffectHeadings)
{
    if (m_effectHeadings == newEffectHeadings)
        return;
    m_effectHeadings = newEffectHeadings;
    Q_EMIT effectHeadingsChanged();
}

void EffectManager::autoIndentCurrentCode(int codeTab, const QString &code)
{
    if (codeTab == 0) {
        // Note: Indent for QML not implemented yet
    } else if (codeTab == 1) {
        const QString indentedCode = m_codeHelper->autoIndentGLSLCode(code);
        m_nodeView->setSelectedNodeVertexCode(indentedCode);
    } else {
        const QString indentedCode = m_codeHelper->autoIndentGLSLCode(code);
        m_nodeView->setSelectedNodeFragmentCode(indentedCode);
    }
}

bool EffectManager::processKey(int codeTab, int keyCode, int modifiers, QQuickTextEdit *textEdit)
{
    if (!textEdit)
        return false;

    bool isAccepted = false;
    if (codeTab == 1 || codeTab == 2)
        isAccepted = m_codeHelper->processKey(textEdit, keyCode, modifiers);

    return isAccepted;
}

void EffectManager::updateImageWatchers()
{
    for (const auto &uniform : std::as_const(m_uniformTable)) {
        if (uniform.type == UniformModel::Uniform::Type::Sampler) {
            // Watch all image properties files
            QString imagePath = stripFileFromURL(uniform.value.toString());
            if (imagePath.isEmpty())
                continue;
            m_fileWatcher.addPath(imagePath);
        }
    }
}

void EffectManager::clearImageWatchers()
{
    const auto watchedFiles = m_fileWatcher.files();
    if (!watchedFiles.isEmpty())
        m_fileWatcher.removePaths(watchedFiles);
}
