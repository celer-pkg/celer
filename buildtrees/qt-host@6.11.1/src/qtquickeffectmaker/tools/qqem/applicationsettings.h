// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef APPLICATIONSETTINGS_H
#define APPLICATIONSETTINGS_H

#include <QObject>
#include <QSettings>
#include <QList>
#include <QAbstractListModel>

class EffectManager;

class ImagesModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)
    Q_PROPERTY(QString currentImageFile READ currentImageFile NOTIFY currentImageFileChanged)

public:
    struct ImagesData {
        QString name;
        QString file;
        int width = 0;
        int height = 0;
        bool canRemove = true;
    };

    enum ModelRoles {
        Name = Qt::UserRole + 1,
        File,
        Width,
        Height,
        CanRemove
    };

    explicit ImagesModel(QObject *effectManager);

    int rowCount(const QModelIndex & = QModelIndex()) const final;
    QVariant data(const QModelIndex &index, int role) const final;
    QHash<int, QByteArray> roleNames() const final;

    QString currentImageFile() const;

public Q_SLOTS:
    void setImageIndex(int index);

Q_SIGNALS:
    void rowCountChanged();
    void currentImageFileChanged();

private:
    friend class ApplicationSettings;
    QList<ImagesData> m_modelList;
    EffectManager *m_effectManager = nullptr;
    int m_currentIndex = 0;
};

class MenusModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)

public:
    struct MenusData {
        QString name;
        QString file;
    };

    enum ModelRoles {
        Name = Qt::UserRole + 1,
        File
    };

    explicit MenusModel(QObject *effectManager);

    int rowCount(const QModelIndex & = QModelIndex()) const final;
    QVariant data(const QModelIndex &index, int role) const final;
    QHash<int, QByteArray> roleNames() const final;

Q_SIGNALS:
    void rowCountChanged();

private:
    friend class ApplicationSettings;
    QList<MenusData> m_modelList;
    EffectManager *m_effectManager = nullptr;
};

class CustomNodesModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)

public:
    struct NodesModelData {
        QString path;
    };

    enum ModelRoles {
        Path = Qt::UserRole + 1,
    };

    explicit CustomNodesModel(QObject *effectManager);

    int rowCount(const QModelIndex & = QModelIndex()) const final;
    QVariant data(const QModelIndex &index, int role) const final;
    QHash<int, QByteArray> roleNames() const final;

Q_SIGNALS:
    void rowCountChanged();

private:
    friend class ApplicationSettings;
    QList<NodesModelData> m_modelList;
    EffectManager *m_effectManager = nullptr;
};

class ApplicationSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ImagesModel *sourceImagesModel READ sourceImagesModel NOTIFY sourceImagesModelChanged)
    Q_PROPERTY(ImagesModel *backgroundImagesModel READ backgroundImagesModel NOTIFY backgroundImagesModelChanged)
    Q_PROPERTY(MenusModel *recentProjectsModel READ recentProjectsModel NOTIFY recentProjectsModelChanged)
    Q_PROPERTY(CustomNodesModel *customNodesModel READ customNodesModel NOTIFY customNodesModelChanged)
    Q_PROPERTY(bool useLegacyShaders READ useLegacyShaders WRITE setUseLegacyShaders NOTIFY useLegacyShadersChanged)
    Q_PROPERTY(QString codeFontFile READ codeFontFile WRITE setCodeFontFile NOTIFY codeFontFileChanged)
    Q_PROPERTY(int codeFontSize READ codeFontSize WRITE setCodeFontSize NOTIFY codeFontSizeChanged)
    Q_PROPERTY(QString defaultResourcePath READ defaultResourcePath)

public:
    explicit ApplicationSettings(QObject *parent = nullptr);

    ImagesModel *sourceImagesModel() const;
    ImagesModel *backgroundImagesModel() const;
    MenusModel *recentProjectsModel() const;
    CustomNodesModel *customNodesModel() const;
    bool useLegacyShaders() const;
    QString codeFontFile() const;
    int codeFontSize() const;

public Q_SLOTS:
    void refreshSourceImagesModel();
    bool addSourceImage(const QString &sourceImage, bool canRemove = true, bool updateSettings = true);
    bool removeSourceImageFromSettings(const QString &sourceImage);
    bool removeSourceImage(const QString &sourceImage);
    bool removeSourceImage(int index);
    void updateRecentProjectsModel(const QString &projectName = QString(), const QString &projectFile = QString());
    void clearRecentProjectsModel();
    void removeRecentProjectsModel(const QString &projectFile);
    void setUseLegacyShaders(bool legacyShaders);
    void setCodeFontFile(const QString &font);
    void setCodeFontSize(int size);
    void resetCodeFont();
    QString defaultResourcePath();
    QStringList customNodesPaths() const;
    void refreshCustomNodesModel();
    bool addCustomNodesPath(const QString &path, bool updateSettings = true);
    bool removeCustomNodesPath(int index);

Q_SIGNALS:
    void sourceImagesModelChanged();
    void backgroundImagesModelChanged();
    void recentProjectsModelChanged();
    void customNodesModelChanged();
    void useLegacyShadersChanged();
    void codeFontFileChanged();
    void codeFontSizeChanged();

private:
    QSettings m_settings;
    EffectManager *m_effectManager = nullptr;
    ImagesModel *m_sourceImagesModel = nullptr;
    ImagesModel *m_backgroundImagesModel = nullptr;
    MenusModel *m_recentProjectsModel = nullptr;
    CustomNodesModel *m_customNodesModel = nullptr;
};

#endif // APPLICATIONSETTINGS_H
