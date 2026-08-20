// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef EXTRUDEDTEXTGEOMETRY_P_H
#define EXTRUDEDTEXTGEOMETRY_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtQuick3DHelpers/qtquick3dhelpersexports.h>
#include <QQuick3DGeometry>
#include <QQmlEngine>
#include <QVector3D>
#include <QFont>

#if QT_CONFIG(concurrent)
#include <QFuture>
#include <QFutureWatcher>
#endif

QT_BEGIN_NAMESPACE

class Q_QUICK3DHELPERS_EXPORT ExtrudedTextGeometry : public QQuick3DGeometry
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)
    Q_PROPERTY(float depth READ depth WRITE setDepth NOTIFY depthChanged)
    Q_PROPERTY(float scale READ scale WRITE setScale NOTIFY scaleChanged FINAL)
    Q_PROPERTY(bool asynchronous READ asynchronous WRITE setAsynchronous NOTIFY asynchronousChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    QML_ELEMENT
    QML_ADDED_IN_VERSION(6, 9)
public:
    enum Status { Null, Ready, Loading, Error };
    Q_ENUM(Status)

    explicit ExtrudedTextGeometry(QQuick3DObject *parent = nullptr);
    ~ExtrudedTextGeometry() override;

    QString text() const;
    void setText(const QString &newText);

    QFont font() const;
    void setFont(const QFont &newFont);

    float depth() const;
    void setDepth(float newDepth);

    float scale() const;
    void setScale(float newScale);

    bool asynchronous() const;
    void setAsynchronous(bool newAsynchronous);

    Status status() const;

    using IndexType = quint32;

private Q_SLOTS:
    void doUpdateGeometry();
    void requestFinished();

Q_SIGNALS:
    void textChanged();
    void fontChanged();
    void depthChanged();
    void scaleChanged();
    void asynchronousChanged();
    void statusChanged();

private:
    struct GeometryData {
        QByteArray vertexData;
        QByteArray indexData;
        QVector3D boundsMin;
        QVector3D boundsMax;
    };

    void scheduleGeometryUpdate();
    void updateGeometry(const GeometryData &geometryData);

    static ExtrudedTextGeometry::GeometryData generateExtrudedTextGeometry(const QString &text,
                                                                           const QFont &font,
                                                                           float depth,
                                                                           float scale);
#if QT_CONFIG(concurrent)
    static void generateExtrudedTextGeometryAsync(QPromise<ExtrudedTextGeometry::GeometryData> &promise,
                                                  const QString &text,
                                                  const QFont &font,
                                                  float depth,
                                                  float scale);
#endif


    QString m_text;
    QFont m_font = QFont(QStringLiteral("Arial"), 4);
    float m_depth = 1.0f;
    float m_scale = 1.0f;
    bool m_asynchronous = true;
    Status m_status = Null;
#if QT_CONFIG(concurrent)
    QFuture<GeometryData> m_geometryDataFuture;
    QFutureWatcher<GeometryData> m_geometryDataWatcher;
#endif
    bool m_geometryUpdateRequested = false;
    bool m_pendingAsyncUpdate = false;
};

QT_END_NAMESPACE

#endif // EXTRUDEDTEXTGEOMETRY_P_H
