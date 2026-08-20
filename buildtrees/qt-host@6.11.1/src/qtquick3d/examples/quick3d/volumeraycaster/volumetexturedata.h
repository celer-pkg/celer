// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef VOLUMETEXTUREDATA_H
#define VOLUMETEXTUREDATA_H

#include <QMutex>
#include <QtQuick3D/QQuick3DTextureData>
#include <QtQml/QQmlEngine>

#include <QtGui/QColor>
#include <QtCore/QByteArray>
#include <QUrl>
#include <QVector3D>

QT_BEGIN_NAMESPACE

class Worker;

class VolumeTextureData : public QQuick3DTextureData
{
    Q_OBJECT
    QML_ELEMENT

public:
    struct AsyncLoaderData
    {
        QUrl source;
        qsizetype width = 0;
        qsizetype height = 0;
        qsizetype depth = 0;
        QString dataType;
        QByteArray volumeData = {};
        bool success = false;
    };

    VolumeTextureData();
    ~VolumeTextureData();

    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(qsizetype width READ width WRITE setWidth NOTIFY widthChanged FINAL)
    Q_PROPERTY(qsizetype height READ height WRITE setHeight NOTIFY heightChanged FINAL)
    Q_PROPERTY(qsizetype depth READ depth WRITE setDepth NOTIFY depthChanged FINAL)
    Q_PROPERTY(QString dataType READ dataType WRITE setDataType NOTIFY dataTypeChanged FINAL)

    QUrl source() const;
    void setSource(const QUrl &newSource);

    qsizetype width() const;
    void setWidth(qsizetype newWidth);

    qsizetype height() const;
    void setHeight(qsizetype newHeight);

    qsizetype depth() const;
    void setDepth(qsizetype newDepth);

    QString dataType() const;
    void setDataType(const QString &newDataType);

    Q_INVOKABLE void loadAsync(QUrl source, qsizetype width, qsizetype height, qsizetype depth, QString dataType);

signals:
    void sourceChanged();
    void widthChanged();
    void heightChanged();
    void depthChanged();
    void dataTypeChanged();
    void loadSucceeded(QUrl source, qsizetype width, qsizetype height, qsizetype depth, QString dataType);
    void loadFailed(QUrl source, qsizetype width, qsizetype height, qsizetype depth, QString dataType);

private:
    void handleResults(VolumeTextureData::AsyncLoaderData result);
    void updateTextureDimensions();
    void initWorker();

    QUrl m_source;
    qsizetype m_width = 0;
    qsizetype m_height = 0;
    qsizetype m_depth = 0;
    qsizetype m_currentDataSize = 0;
    QString m_dataType;

    // Async variables
    AsyncLoaderData loaderData;
    bool m_isLoading = false;
    bool m_isAborting = false;
    Worker *m_worker = nullptr;
};

QT_END_NAMESPACE

#endif // VOLUMETEXTUREDATA_H
