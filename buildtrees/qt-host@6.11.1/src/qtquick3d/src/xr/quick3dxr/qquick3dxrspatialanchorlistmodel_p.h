// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DXRSPATIALANCHORLISTMODEL_P_H
#define QQUICK3DXRSPATIALANCHORLISTMODEL_P_H

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

#include <QAbstractListModel>
#include <QtQmlIntegration/qqmlintegration.h>
#include <QtCore/qpointer.h>
#include <QtCore/qstringlist.h>
#include <QUuid>

QT_BEGIN_NAMESPACE

class QQuick3DXrAnchorManager;
class QQuick3DXrSpatialAnchor;

class QQuick3DXrSpatialAnchorListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(FilterMode filterMode READ filterMode WRITE setFilterMode NOTIFY filterModeChanged FINAL)
    Q_PROPERTY(ClassificationFlags classificationFilter READ classificationFilter WRITE setClassificationFilter NOTIFY classificationFilterChanged FINAL)
    Q_PROPERTY(QStringList classificationStringFilter READ classificationStringFilter WRITE setClassificationStringFilter NOTIFY classificationStringFilterChanged FINAL)
    Q_PROPERTY(QStringList identifierFilter READ identifierFilter WRITE setIdentifierFilter NOTIFY identifierFilterChanged FINAL)

    QML_NAMED_ELEMENT(XrSpatialAnchorListModel)
    QML_ADDED_IN_VERSION(6, 8)
public:
    enum class FilterMode {
        All,
        Classification,
        Identifier
    };
    Q_ENUM(FilterMode)

    enum ClassificationFlag : quint32 {
        Wall = 0x1,
        Ceiling = 0x2,
        Floor = 0x4,
        Table = 0x8,
        Seat = 0x10,
        Window = 0x20,
        Door = 0x40,
        Other = 0x80,
    };
    Q_ENUM(ClassificationFlag)
    Q_DECLARE_FLAGS(ClassificationFlags , ClassificationFlag)
    Q_FLAG(ClassificationFlags)

    enum SpatialAnchorRoles {
        Anchor = Qt::UserRole + 21
    };

    explicit QQuick3DXrSpatialAnchorListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void requestSceneCapture();
    Q_INVOKABLE void queryAnchors();

    FilterMode filterMode() const;
    void setFilterMode(FilterMode newFilterMode);

    QStringList identifierFilter() const;
    void setIdentifierFilter(const QStringList &filter);

    ClassificationFlags classificationFilter() const;
    void setClassificationFilter(ClassificationFlags newClassFilter);

    QStringList classificationStringFilter() const;
    void setClassificationStringFilter(const QStringList &newClassStringFilter);

signals:
    void filterModeChanged();
    void identifierFilterChanged();
    void classificationFilterChanged();
    void classificationStringFilterChanged();

private Q_SLOTS:
    void handleAnchorAdded(QQuick3DXrSpatialAnchor* anchor);
    void handleAnchorRemoved(QUuid uuid);
    void handleAnchorUpdated(QQuick3DXrSpatialAnchor* anchor);

private:
    bool matchesAnchorFilter(QQuick3DXrSpatialAnchor *anchor) const;
    QList<QQuick3DXrSpatialAnchor *> anchorsFiltered() const;

    QPointer<QQuick3DXrAnchorManager> m_anchorManager;
    FilterMode m_filterMode = FilterMode::All;
    QSet<QString> m_uuids;
    ClassificationFlags m_classFilter;
    QSet<QString> m_classStringFilter;
};

QT_END_NAMESPACE

#endif // QQUICK3DXRSPATIALANCHORLISTMODEL_P_H
