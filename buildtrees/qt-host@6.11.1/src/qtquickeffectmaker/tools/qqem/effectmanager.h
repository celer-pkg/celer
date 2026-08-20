// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef EFFECTMANAGER_H
#define EFFECTMANAGER_H

#include <QObject>
#include <QTemporaryFile>
#include <QTimer>
#include <QQmlPropertyMap>
#include <QFileSystemWatcher>
#include "uniformmodel.h"
#include "nodeview.h"
#include "shaderfeatures.h"
#include <rhi/qshaderbaker.h>
#include <QtQuick/private/qquicktextedit_p_p.h>
#include "addnodemodel.h"
#include "applicationsettings.h"
#include "codehelper.h"

// The delay in ms to wait until updating the effect
const int EFFECT_UPDATE_DELAY = 200;

// This will be used for commandline arguments.
extern QQmlPropertyMap g_argData;

struct EffectError {
    Q_GADGET
    Q_PROPERTY(QString message MEMBER m_message)
    Q_PROPERTY(int line MEMBER m_line)
    Q_PROPERTY(int type MEMBER m_type)
public:
    QString m_message;
    int m_line = -1;
    int m_type = -1;
};

class EffectManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(UniformModel *uniformModel READ uniformModel WRITE setUniformModel NOTIFY uniformModelChanged)
    Q_PROPERTY(NodeView *nodeView READ nodeView WRITE setNodeView NOTIFY nodeViewChanged)
    Q_PROPERTY(CodeHelper *codeHelper READ codeHelper CONSTANT)
    Q_PROPERTY(QString fragmentShader READ fragmentShader WRITE setFragmentShader NOTIFY fragmentShaderChanged)
    Q_PROPERTY(QString vertexShader READ vertexShader WRITE setVertexShader NOTIFY vertexShaderChanged)
    Q_PROPERTY(QString fragmentShaderFilename READ fragmentShaderFilename WRITE setFragmentShaderFilename NOTIFY fragmentShaderFilenameChanged)
    Q_PROPERTY(QString vertexShaderFilename READ vertexShaderFilename WRITE setVertexShaderFilename NOTIFY vertexShaderFilenameChanged)
    Q_PROPERTY(QString qmlComponentString READ qmlComponentString WRITE setQmlComponentString NOTIFY qmlComponentStringChanged)

    Q_PROPERTY(EffectError effectError READ effectError NOTIFY effectErrorChanged)
    Q_PROPERTY(bool unsavedChanges READ unsavedChanges WRITE setUnsavedChanges NOTIFY unsavedChangesChanged)
    Q_PROPERTY(bool hasProjectFilename READ hasProjectFilename NOTIFY hasProjectFilenameChanged)
    Q_PROPERTY(QString projectName READ projectName NOTIFY projectNameChanged)
    Q_PROPERTY(QString projectFilename READ projectFilename NOTIFY projectFilenameChanged)
    Q_PROPERTY(QString projectDirectory READ projectDirectory NOTIFY projectDirectoryChanged)
    Q_PROPERTY(QString exportFilename READ exportFilename NOTIFY exportFilenameChanged)
    Q_PROPERTY(QString exportDirectory READ exportDirectory NOTIFY exportDirectoryChanged)
    Q_PROPERTY(int exportFlags READ exportFlags NOTIFY exportFlagsChanged)
    Q_PROPERTY(bool shadersUpToDate READ shadersUpToDate WRITE setShadersUpToDate NOTIFY shadersUpToDateChanged)
    Q_PROPERTY(bool autoPlayEffect READ autoPlayEffect WRITE setAutoPlayEffect NOTIFY autoPlayEffectChanged)
    Q_PROPERTY(AddNodeModel *addNodeModel READ addNodeModel NOTIFY addNodeModelChanged)
    Q_PROPERTY(QRect effectPadding READ effectPadding WRITE setEffectPadding NOTIFY effectPaddingChanged)
    Q_PROPERTY(QString effectHeadings READ effectHeadings WRITE setEffectHeadings NOTIFY effectHeadingsChanged)
    Q_PROPERTY(ApplicationSettings * settings READ settings NOTIFY settingsChanged)
    QML_ELEMENT

public:
    explicit EffectManager(QObject *parent = nullptr);

    UniformModel *uniformModel() const;
    void setUniformModel(UniformModel *newUniformModel);

    CodeHelper *codeHelper() const;

    QString fragmentShader() const;
    void setFragmentShader(const QString &newFragmentShader);

    QString vertexShader() const;
    void setVertexShader(const QString &newVertexShader);

    bool unsavedChanges() const;
    void setUnsavedChanges(bool newUnsavedChanges);

    bool hasProjectFilename() const;
    QString projectFilename() const;
    QString projectName() const;
    void setProjectName(const QString &name);
    QString projectDirectory() const;
    QString exportFilename() const;
    QString exportDirectory() const;
    int exportFlags() const;

    NodeView *nodeView() const;
    void setNodeView(NodeView *newNodeView);

    EffectError effectError() const;

    const QString &fragmentShaderFilename() const;
    void setFragmentShaderFilename(const QString &newFragmentShaderFilename);

    const QString &vertexShaderFilename() const;
    void setVertexShaderFilename(const QString &newVertexShaderFilename);

    const QString &qmlComponentString() const;
    void setQmlComponentString(const QString &string);

    bool shadersUpToDate() const;
    void setShadersUpToDate(bool upToDate);

    bool autoPlayEffect() const;
    void setAutoPlayEffect(bool autoPlay);

    AddNodeModel *addNodeModel() const;

    const QRect &effectPadding() const;
    QString effectHeadings() const;

    ApplicationSettings *settings() const {
        return m_settings;
    }

    Q_INVOKABLE QString mipmapPropertyName(const QString &name) const;
    Q_INVOKABLE QString getSupportedImageFormatsFilter() const;

    Q_INVOKABLE int effectUpdateDelay() const {
        return EFFECT_UPDATE_DELAY;
    }

public Q_SLOTS:
    QString generateVertexShader(bool includeUniforms = true);
    QString generateFragmentShader(bool includeUniforms = true);
    void updateQmlComponent();
    void initialize();
    NodesModel::Node loadEffectNode(const QString &filename);
    bool addEffectNode(const QString &filename, int startNodeId = -1, int endNodeId = -1);
    bool deleteEffectNodes(QList<int> nodeIds);
    bool loadProject(const QUrl &filename);
    bool saveProject(const QUrl &filename = {});
    bool newProject(const QString &filepath, const QString &filename, bool clearNodeView, bool createProjectDir = false);
    void closeProject();
    bool exportEffect(const QString &dirPath, const QString &filename, int exportFlags, int qsbVersionIndex);
    void cleanupProject();
    void cleanupNodeView(bool initialize = true);
    bool saveSelectedNode(const QUrl &filename);
    void updateBakedShaderVersions();
    void bakeShaders(bool forced = false);

    QString getHelpTextString();
    void updateAddNodeData();
    void showHideAddNodeGroup(const QString &groupName, bool show);
    void refreshAddNodesList();
    void setEffectPadding(const QRect &newEffectPadding);
    void setEffectHeadings(const QString &newEffectHeadings);
    QString stripFileFromURL(const QString &urlString) const;
    QString addFileToURL(const QString &urlString) const;
    QString getDirectory(const QString &path, bool useFileScheme = true) const;
    QString getDefaultImagesDirectory(bool useFileScheme = true) const;
    void autoIndentCurrentCode(int codeTab, const QString &code);
    bool processKey(int codeTab, int keyCode, int modifiers, QQuickTextEdit *textEdit);
    void setEffectError(const QString &errorMessage, int type = -1, int lineNumber = -1);
    void resetEffectError(int type = -1);

signals:
    void uniformModelChanged();
    void fragmentShaderChanged();
    void vertexShaderChanged();
    void unsavedChangesChanged();
    void hasProjectFilenameChanged();
    void projectFilenameChanged();
    void projectNameChanged();
    void projectDirectoryChanged();
    void exportFilenameChanged();
    void exportDirectoryChanged();
    void exportFlagsChanged();
    void shadersBaked();

    void nodeViewChanged();
    void effectErrorChanged();

    void fragmentShaderFilenameChanged();
    void vertexShaderFilenameChanged();
    void qmlComponentStringChanged();
    void shadersUpToDateChanged();
    void autoPlayEffectChanged();
    void addNodeModelChanged();

    void effectPaddingChanged();
    void effectHeadingsChanged();
    void settingsChanged();

private:
    friend class AddNodeModel;
    friend class ApplicationSettings;

    enum ExportFlags {
        QMLComponent = 1,
        QSBShaders = 2,
        TextShaders = 4,
        Images = 8,
        QRCFile = 16
    };

    enum ErrorTypes {
        ErrorCommon = -1,
        ErrorQMLParsing,
        ErrorVert,
        ErrorFrag,
        ErrorQMLRuntime,
        ErrorPreprocessor
    };

    const QString getBufUniform();
    const QString getFSUniforms();
    const QString getVSUniforms();
    const QString getDefineProperties();
    const QString getConstVariables();
    void updateCustomUniforms();
    QString getQmlImagesString(bool localFiles);
    QString getQmlComponentString(bool localFiles);
    QString getQmlEffectString();
    QStringList getDefaultRootVertexShader();
    QStringList getDefaultRootFragmentShader();
    QString processVertexRootLine(const QString &line);
    QString processFragmentRootLine(const QString &line);
    QString getCustomShaderVaryings(bool outState);
    QStringList removeTagsFromCode(const QStringList &codeLines);
    QString removeTagsFromCode(const QString &code);
    QString replaceOldTagsWithNew(const QString &code);
    QString detectErrorMessage(const QString &errorMessage);

    void doBakeShaders();
    QFile resolveFileFromUrl(const QUrl &fileUrl);
    bool createNodeFromJson(const QJsonObject &rootJson, NodesModel::Node &node, bool fullNode, const QString &nodePath);
    bool addNodeIntoView(NodesModel::Node &node, int startNodeId = -1, int endNodeId = -1);
    bool addNodeConnection(int startNodeId, int endNodeId);
    int getTagIndex(const QStringList &code, const QString &tag);
    QJsonObject nodeToJson(const NodesModel::Node &node, bool simplified, const QString &nodePath);
    QString codeFromJsonArray(const QJsonArray &codeArray);
    QString absoluteToRelativePath(const QString &path, const QString &toPath = QString());
    QString relativeToAbsolutePath(const QString &path, const QString &toPath = QString());
    void updateImageWatchers();
    void clearImageWatchers();

    UniformModel *m_uniformModel = nullptr;
    UniformModel::UniformTable m_uniformTable;
    CodeHelper *m_codeHelper = nullptr;
    QString m_fragmentShader;
    QString m_vertexShader;
    QString m_qmlCode;
    // Used in exported QML, at root of the file
    QString m_exportedRootPropertiesString;
    // Used in exported QML, at ShaderEffect component of the file
    QString m_exportedEffectPropertiesString;
    // Used in preview QML, at ShaderEffect component of the file
    QString m_previewEffectPropertiesString;
    bool m_unsavedChanges = false;
    bool m_firstBake = true;
    // Full path of the project file
    QUrl m_projectFilename;
    // Path to where project file is located
    QString m_projectDirectory;
    // Exported effect name, without the prefixes
    QString m_exportFilename;
    // Path to where effect is exported
    QString m_exportDirectory;
    // Name of the project
    // E.g. "MyEffect"
    QString m_projectName;
    int m_exportFlags = QMLComponent | QSBShaders | Images;

    QTemporaryFile m_fragmentShaderFile;
    QTemporaryFile m_vertexShaderFile;

    ApplicationSettings *m_settings = nullptr;
    QShaderBaker m_baker;
    NodeView *m_nodeView = nullptr;
    QMap<int, EffectError> m_effectErrors;
    QString m_fragmentShaderFilename;
    QString m_vertexShaderFilename;
    QString m_qmlComponentString;
    QStringList m_defaultRootVertexShader;
    QStringList m_defaultRootFragmentShader;
    QTimer m_shaderBakerTimer;
    // True when shaders haven't changed since last baking
    bool m_shadersUpToDate = true;
    // When true, shaders are baked automatically after a
    // short delay when they change.
    bool m_autoPlayEffect = true;
    bool m_loadComponentImages = true;

    ShaderFeatures m_shaderFeatures;
    AddNodeModel *m_addNodeModel = nullptr;
    QStringList m_shaderVaryingVariables;
    QRect m_effectPadding;
    QString m_effectHeadings;
    QFileSystemWatcher m_fileWatcher;
};

#endif // EFFECTMANAGER_H
