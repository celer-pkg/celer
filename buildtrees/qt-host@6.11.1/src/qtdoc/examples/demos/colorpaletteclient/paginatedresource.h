// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#ifndef PAGINATEDRESOURCE_H
#define PAGINATEDRESOURCE_H

#include "abstractresource.h"

#include <QtQml/qqml.h>
#include <QtCore/qjsonobject.h>

// This class manages a simple paginated Crud resource,
// where the resource is a paginated list of JSON items
class PaginatedResource : public AbstractResource
{
    Q_OBJECT
    Q_PROPERTY(QList<QJsonObject> data READ data NOTIFY dataUpdated)
    Q_PROPERTY(int page READ page WRITE setPage NOTIFY pageUpdated)
    Q_PROPERTY(int pages READ pages NOTIFY pagesUpdated)
    Q_PROPERTY(QString path MEMBER m_path REQUIRED)
    QML_ELEMENT

public:
    explicit PaginatedResource(QObject *parent = nullptr);
    ~PaginatedResource() override = default;

    QList<QJsonObject> data() const;

    // Total number of pages according to the server
    int pages() const;

    // Current page this resource is on. Changing the page will initiate a refresh.
    // The default page is 1
    int page() const;
    void setPage(int page);

    Q_INVOKABLE void refreshCurrentPage();
    Q_INVOKABLE void update(const QVariantMap &data, int id);
    Q_INVOKABLE void add(const QVariantMap &data);
    Q_INVOKABLE void remove(int id);

signals:
    void dataUpdated();
    void pageUpdated();
    void pagesUpdated();

private:
    void refreshRequestFinished(const QJsonDocument &json);
    void refreshRequestFailed();
    QList<QJsonObject> m_data;
    // The total number of pages as reported by the server responses
    int m_pages = 0;
    // The default page we request if the user hasn't set otherwise
    int m_currentPage = 1;
    QString m_path;
};

#endif // PAGINATEDRESOURCE_H
