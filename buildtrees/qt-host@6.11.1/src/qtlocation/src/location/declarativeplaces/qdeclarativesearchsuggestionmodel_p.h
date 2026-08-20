// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDECLARATIVESEARCHSUGGESTIONMODEL_P_H
#define QDECLARATIVESEARCHSUGGESTIONMODEL_P_H

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

#include <QtLocation/private/qlocationglobal_p.h>
#include <QtLocation/private/qdeclarativesearchmodelbase_p.h>

#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

class QDeclarativeGeoServiceProvider;
class QGeoServiceProvider;

class Q_LOCATION_EXPORT QDeclarativeSearchSuggestionModel : public QDeclarativeSearchModelBase
{
    Q_OBJECT
    QML_NAMED_ELEMENT(PlaceSearchSuggestionModel)
    QML_ADDED_IN_VERSION(5, 0)

    Q_PROPERTY(QString searchTerm READ searchTerm WRITE setSearchTerm NOTIFY searchTermChanged)
    Q_PROPERTY(QStringList suggestions READ suggestions NOTIFY suggestionsChanged)

public:
    explicit QDeclarativeSearchSuggestionModel(QObject *parent = nullptr);
    ~QDeclarativeSearchSuggestionModel();

    QString searchTerm() const;
    void setSearchTerm(const QString &searchTerm);

    QStringList suggestions() const;

    void clearData(bool suppressSignal = false) override;

    // From QAbstractListModel
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    enum Roles {
        SearchSuggestionRole = Qt::UserRole
    };

protected Q_SLOTS:
    void queryFinished() override;

Q_SIGNALS:
    void searchTermChanged();
    void suggestionsChanged();

protected:
    QPlaceReply *sendQuery(QPlaceManager *manager, const QPlaceSearchRequest &request) override;

private:
    QStringList m_suggestions;
};

QT_END_NAMESPACE

#endif
