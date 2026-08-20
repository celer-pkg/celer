// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "applicationsettings.h"
#include "effectmanager.h"

#include <QImageReader>
#include <QFileInfo>
#include <QLibraryInfo>

const QStringList defaultSources = { "defaultnodes/images/qt_logo_green_rgb.png",
                                     "defaultnodes/images/quit_logo.png",
                                     "defaultnodes/images/whitecircle.png",
                                     "defaultnodes/images/blackcircle.png" };

const QStringList defaultBackgrounds = { "images/background_dark.jpg",
                                         "images/background_light.jpg",
                                         "images/background_colorful.jpg" };

const QString KEY_CUSTOM_SOURCE_IMAGES = QStringLiteral("customSourceImages");
const QString KEY_RECENT_PROJECTS = QStringLiteral("recentProjects");
const QString KEY_PROJECT_NAME = QStringLiteral("projectName");
const QString KEY_PROJECT_FILE = QStringLiteral("projectFile");
const QString KEY_LEGACY_SHADERS = QStringLiteral("useLegacyShaders");
const QString KEY_CODE_FONT_FILE = QStringLiteral("codeFontFile");
const QString KEY_CODE_FONT_SIZE = QStringLiteral("codeFontSize");
const QString KEY_DEFAULT_RESOURCE_PATH = QStringLiteral("defaultResourcePath");
const QString KEY_CUSTOM_NODE_PATHS = QStringLiteral("customNodePaths");

const QString DEFAULT_CODE_FONT_FILE = QStringLiteral("fonts/SourceCodePro-Regular.ttf");
const int DEFAULT_CODE_FONT_SIZE = 14;

ImagesModel::ImagesModel(QObject *effectManager)
    : QAbstractListModel(effectManager)
{
    m_effectManager = static_cast<EffectManager *>(effectManager);
}

int ImagesModel::rowCount(const QModelIndex &) const
{
    return m_modelList.size();
}

QHash<int, QByteArray> ImagesModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Name] = "name";
    roles[File] = "file";
    roles[Width] = "width";
    roles[Height] = "height";
    roles[CanRemove] = "canRemove";
    return roles;
}

QVariant ImagesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_modelList.size())
        return QVariant();

    const auto &item = (m_modelList)[index.row()];

    if (role == Name)
        return QVariant::fromValue(item.name);
    else if (role == File)
        return QVariant::fromValue(item.file);
    else if (role == Width)
        return QVariant::fromValue(item.width);
    else if (role == Height)
        return QVariant::fromValue(item.height);
    else if (role == CanRemove)
        return QVariant::fromValue(item.canRemove);

    return QVariant();
}

void ImagesModel::setImageIndex(int index) {
    if (m_currentIndex == index)
        return;
    m_currentIndex = index;
    Q_EMIT currentImageFileChanged();
}

QString ImagesModel::currentImageFile() const
{
    if (m_modelList.size() > m_currentIndex)
        return m_modelList.at(m_currentIndex).file;
    return QString();
}

MenusModel::MenusModel(QObject *effectManager)
    : QAbstractListModel(effectManager)
{
    m_effectManager = static_cast<EffectManager *>(effectManager);
}

int MenusModel::rowCount(const QModelIndex &) const
{
    return m_modelList.size();
}

QHash<int, QByteArray> MenusModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Name] = "name";
    roles[File] = "file";
    return roles;
}

QVariant MenusModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_modelList.size())
        return QVariant();

    const auto &item = (m_modelList)[index.row()];

    if (role == Name)
        return QVariant::fromValue(item.name);
    else if (role == File)
        return QVariant::fromValue(item.file);

    return QVariant();
}

CustomNodesModel::CustomNodesModel(QObject *effectManager)
    : QAbstractListModel(effectManager)
{
    m_effectManager = static_cast<EffectManager *>(effectManager);
}

int CustomNodesModel::rowCount(const QModelIndex &) const
{
    return m_modelList.size();
}

QHash<int, QByteArray> CustomNodesModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Path] = "path";
    return roles;
}

QVariant CustomNodesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_modelList.size())
        return QVariant();

    const auto &item = (m_modelList)[index.row()];

    if (role == Path)
        return QVariant::fromValue(item.path);

    return QVariant();
}

ApplicationSettings::ApplicationSettings(QObject *parent)
    : QObject{parent}
{
    // Get canonical path into default nodes
    QString resourcesPath = QLibraryInfo::path(QLibraryInfo::QmlImportsPath) +
                            QStringLiteral("/QtQuickEffectMaker");
    QFileInfo fi(resourcesPath);
    resourcesPath = fi.canonicalFilePath();
    m_settings.setValue(KEY_DEFAULT_RESOURCE_PATH, resourcesPath);

    m_effectManager = static_cast<EffectManager *>(parent);
    m_sourceImagesModel = new ImagesModel(m_effectManager);
    m_backgroundImagesModel = new ImagesModel(m_effectManager);
    m_recentProjectsModel = new MenusModel(m_effectManager);
    m_customNodesModel = new CustomNodesModel(m_effectManager);

    refreshSourceImagesModel();
    refreshCustomNodesModel();

    // Add default backgrounds
    for (const auto &source : defaultBackgrounds) {
        ImagesModel::ImagesData d;
        d.file = source;
        m_backgroundImagesModel->m_modelList.append(d);
    }
}

// Refresh the source images list
void ApplicationSettings::refreshSourceImagesModel()
{
    QStringList customSources = m_settings.value(KEY_CUSTOM_SOURCE_IMAGES).value<QStringList>();
    // Remove custom images that don't exist from settings
    for (const auto &source : customSources) {
        QUrl url(source);
        QString sourceImageFile = url.toLocalFile();
        if (!QFile::exists(sourceImageFile))
            removeSourceImageFromSettings(source);
    }

    m_sourceImagesModel->m_modelList.clear();

    // Add default Sources
    for (const auto &source : defaultSources) {
        QString absolutePath = m_effectManager->relativeToAbsolutePath(source, defaultResourcePath());
        addSourceImage(absolutePath, false, false);
    }

    // Add custom sources from settings
    customSources = m_settings.value(KEY_CUSTOM_SOURCE_IMAGES).value<QStringList>();
    for (const auto &source : customSources)
        addSourceImage(source, true, false);
}

bool ApplicationSettings::addSourceImage(const QString &sourceImage, bool canRemove, bool updateSettings)
{
    if (sourceImage.isEmpty())
        return false;

    // Check for duplicates
    for (const auto &source : m_sourceImagesModel->m_modelList) {
        if (source.file == sourceImage) {
            qWarning("Image already exists in the model, so not adding");
            return false;
        }
    }

    // Remove "file:/" from the path so it suits QImageReader
    QUrl url(sourceImage);
    QString sourceImageFile = url.toLocalFile();
    QImageReader imageReader(sourceImageFile);
    QSize imageSize(0, 0);
    if (imageReader.canRead()) {
        imageSize = imageReader.size();
    } else {
        qWarning("Can't read image: %s", qPrintable(sourceImage));
        return false;
    }

    m_sourceImagesModel->beginResetModel();
    ImagesModel::ImagesData d;
    d.file = sourceImage;
    d.width = imageSize.width();
    d.height = imageSize.height();
    d.canRemove = canRemove;
    m_sourceImagesModel->m_modelList.append(d);
    m_sourceImagesModel->endResetModel();

    if (updateSettings && canRemove) {
        // Non-default images are added also into settings
        QStringList customSources = m_settings.value(KEY_CUSTOM_SOURCE_IMAGES).value<QStringList>();
        if (!customSources.contains(d.file)) {
            customSources.append(d.file);
            m_settings.setValue(KEY_CUSTOM_SOURCE_IMAGES, customSources);
        }
    }
    return true;
}

// Removes sourceImage from the QSettings
bool ApplicationSettings::removeSourceImageFromSettings(const QString &sourceImage)
{
    QStringList customSources = m_settings.value(KEY_CUSTOM_SOURCE_IMAGES).value<QStringList>();
    if (customSources.contains(sourceImage)) {
        customSources.removeAll(sourceImage);
        m_settings.setValue(KEY_CUSTOM_SOURCE_IMAGES, customSources);
        return true;
    }
    return false;
}

// Removes sourceImage from the model
bool ApplicationSettings::removeSourceImage(const QString &sourceImage)
{
    for (int i = 0; i < m_sourceImagesModel->m_modelList.size(); i++) {
        const auto &d = m_sourceImagesModel->m_modelList.at(i);
        if (d.file == sourceImage)
            return removeSourceImage(i);
    }
    return false;
}
bool ApplicationSettings::removeSourceImage(int index)
{
    if (index < 0 || index >= m_sourceImagesModel->m_modelList.size())
        return false;

    m_sourceImagesModel->beginResetModel();
    m_sourceImagesModel->m_modelList.removeAt(index);
    m_sourceImagesModel->endResetModel();

    if (index >= defaultSources.size()) {
        QStringList customSources = m_settings.value(KEY_CUSTOM_SOURCE_IMAGES).value<QStringList>();
        customSources.removeAt(index - defaultSources.size());
        m_settings.setValue(KEY_CUSTOM_SOURCE_IMAGES, customSources);
    }

    return true;
}

// Updates the recent projects model by adding / moving projectFile to first.
void ApplicationSettings::updateRecentProjectsModel(const QString &projectName, const QString &projectFile) {

    int projectListIndex = -1;
    QList<MenusModel::MenusData> recentProjectsList;
    // Recent projects menu will contain max this amount of item
    const int max_items = 10;

    if (!projectFile.isEmpty() && !m_recentProjectsModel->m_modelList.isEmpty()
            && m_recentProjectsModel->m_modelList.first().file == projectFile) {
        // First element of the recent projects list is already the
        // selected project, so nothing to update here.
        return;
    }

    // Read from settings
    int size = m_settings.beginReadArray(KEY_RECENT_PROJECTS);
    for (int i = 0; i < size; ++i) {
        if (i >= max_items)
            break;
        m_settings.setArrayIndex(i);
        MenusModel::MenusData d;
        d.name = m_settings.value(KEY_PROJECT_NAME).toString();
        d.file = m_settings.value(KEY_PROJECT_FILE).toString();
        if (!d.name.isEmpty() && !d.file.isEmpty()) {
            recentProjectsList.append(d);
            if (d.file == projectFile) {
                // Note: Can't use 'i' here as settings index may be different than QList index.
                projectListIndex = (recentProjectsList.size() - 1);
            }
        }
    }
    m_settings.endArray();

    // Update model if entry was given
    if (!projectName.isEmpty() && !projectFile.isEmpty()) {
        if (projectListIndex == -1) {
            // If file isn't in the list, add it first
            MenusModel::MenusData d;
            d.file = projectFile;
            d.name = projectName;
            recentProjectsList.prepend(d);
        } else if (projectListIndex > 0) {
            // Or move it on top
            recentProjectsList.move(projectListIndex, 0);
        }

        if (recentProjectsList.size() > max_items)
            recentProjectsList.removeLast();

        // Write to settings
        m_settings.beginWriteArray(KEY_RECENT_PROJECTS);
        for (int i = 0; i < recentProjectsList.size(); ++i) {
            m_settings.setArrayIndex(i);
            const auto &d = recentProjectsList.at(i);
            m_settings.setValue(KEY_PROJECT_NAME, d.name);
            m_settings.setValue(KEY_PROJECT_FILE, d.file);
        }
        m_settings.endArray();
    }

    m_recentProjectsModel->beginResetModel();
    m_recentProjectsModel->m_modelList = recentProjectsList;
    m_recentProjectsModel->endResetModel();
}

void ApplicationSettings::clearRecentProjectsModel()
{
    m_settings.beginWriteArray(KEY_RECENT_PROJECTS);
    m_settings.endArray();
    m_recentProjectsModel->beginResetModel();
    m_recentProjectsModel->m_modelList.clear();
    m_recentProjectsModel->endResetModel();
}

void ApplicationSettings::removeRecentProjectsModel(const QString &projectFile)
{
    int size = m_settings.beginReadArray(KEY_RECENT_PROJECTS);
    for (int i = 0; i < size; ++i) {
        m_settings.setArrayIndex(i);
        QString filename = m_settings.value(KEY_PROJECT_FILE).toString();
        if (filename == projectFile) {
            m_settings.remove(KEY_PROJECT_NAME);
            m_settings.remove(KEY_PROJECT_FILE);
            m_recentProjectsModel->beginResetModel();
            m_recentProjectsModel->m_modelList.removeAt(i);
            m_recentProjectsModel->endResetModel();
            break;
        }
    }
    m_settings.endArray();
}

ImagesModel *ApplicationSettings::sourceImagesModel() const
{
    return m_sourceImagesModel;
}

ImagesModel *ApplicationSettings::backgroundImagesModel() const
{
    return m_backgroundImagesModel;
}

MenusModel *ApplicationSettings::recentProjectsModel() const
{
    return m_recentProjectsModel;
}

CustomNodesModel *ApplicationSettings::customNodesModel() const
{
    return m_customNodesModel;
}

bool ApplicationSettings::useLegacyShaders() const
{
    return m_settings.value(KEY_LEGACY_SHADERS, false).toBool();
}

void ApplicationSettings::setUseLegacyShaders(bool legacyShaders)
{
    if (useLegacyShaders() == legacyShaders)
        return;

    m_settings.setValue(KEY_LEGACY_SHADERS, legacyShaders);
    Q_EMIT useLegacyShadersChanged();
    m_effectManager->updateBakedShaderVersions();
    m_effectManager->doBakeShaders();
}

QString ApplicationSettings::codeFontFile() const
{
    return m_settings.value(KEY_CODE_FONT_FILE, DEFAULT_CODE_FONT_FILE).toString();
}

int ApplicationSettings::codeFontSize() const
{
    return m_settings.value(KEY_CODE_FONT_SIZE, DEFAULT_CODE_FONT_SIZE).toInt();
}

void ApplicationSettings::setCodeFontFile(const QString &font)
{
    if (codeFontFile() == font)
        return;

    m_settings.setValue(KEY_CODE_FONT_FILE, font);
    Q_EMIT codeFontFileChanged();
}

void ApplicationSettings::setCodeFontSize(int size)
{
    if (codeFontSize() == size)
        return;

    m_settings.setValue(KEY_CODE_FONT_SIZE, size);
    Q_EMIT codeFontSizeChanged();
}

void ApplicationSettings::resetCodeFont()
{
    setCodeFontFile(DEFAULT_CODE_FONT_FILE);
    setCodeFontSize(DEFAULT_CODE_FONT_SIZE);
}

QString ApplicationSettings::defaultResourcePath()
{
    return m_settings.value(KEY_DEFAULT_RESOURCE_PATH).value<QString>();
}

QStringList ApplicationSettings::customNodesPaths() const
{
    return m_settings.value(KEY_CUSTOM_NODE_PATHS).value<QStringList>();
}

// Refresh the custom nodes path list
void ApplicationSettings::refreshCustomNodesModel()
{
    // Add custom sources from settings
    QStringList customNodes = m_settings.value(KEY_CUSTOM_NODE_PATHS).value<QStringList>();
    for (const auto &source : customNodes)
        addCustomNodesPath(source, false);
}

bool ApplicationSettings::addCustomNodesPath(const QString &path, bool updateSettings)
{
    if (path.isEmpty())
        return false;

    QString newPath = m_effectManager->stripFileFromURL(path);
    // Check for duplicates
    for (const auto &item : m_customNodesModel->m_modelList) {
        if (item.path == newPath) {
            qWarning("Path already exists in the model, so not adding");
            return false;
        }
    }

    m_customNodesModel->beginResetModel();
    CustomNodesModel::NodesModelData d;
    d.path = newPath;
    m_customNodesModel->m_modelList.append(d);
    m_customNodesModel->endResetModel();

    if (updateSettings) {
        // Add also into settings
        QStringList customNodes = m_settings.value(KEY_CUSTOM_NODE_PATHS).value<QStringList>();
        if (!customNodes.contains(d.path)) {
            customNodes.append(d.path);
            m_settings.setValue(KEY_CUSTOM_NODE_PATHS, customNodes);
        }
    }

    return true;
}

bool ApplicationSettings::removeCustomNodesPath(int index)
{
    if (index < 0 || index >= m_customNodesModel->m_modelList.size())
        return false;

    m_customNodesModel->beginResetModel();
    m_customNodesModel->m_modelList.removeAt(index);
    m_customNodesModel->endResetModel();

    QStringList customSources = m_settings.value(KEY_CUSTOM_NODE_PATHS).value<QStringList>();
    if (index < customSources.size()) {
        customSources.removeAt(index);
        m_settings.setValue(KEY_CUSTOM_NODE_PATHS, customSources);
    }

    return true;
}
