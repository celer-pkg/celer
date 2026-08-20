// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "paginatedresource.h"

#include <QtNetwork/qrestaccessmanager.h>
#include <QtNetwork/qrestreply.h>

#include <QtCore/qjsonarray.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qurlquery.h>

using namespace Qt::StringLiterals;

static constexpr auto totalPagesField = "total_pages"_L1;
static constexpr auto currentPageField = "page"_L1;
static constexpr auto resourceId = "/%1"_L1;

PaginatedResource::PaginatedResource(QObject *parent)
    : AbstractResource(parent)
{
}

QList<QJsonObject> PaginatedResource::data() const
{
    return m_data;
}

int PaginatedResource::pages() const
{
    return m_pages;
}

int PaginatedResource::page() const
{
    return m_currentPage;
}

void PaginatedResource::setPage(int page)
{
    if (m_currentPage == page || page < 1)
        return;
    m_currentPage = page;
    emit pageUpdated();
    refreshCurrentPage();
}

void PaginatedResource::refreshCurrentPage()
{
    QUrlQuery query{{"page"_L1, QString::number(m_currentPage)}};
    m_manager->get(m_api->createRequest(m_path, query), this, [this](QRestReply &reply) {
                       if (const auto json = reply.readJson()) {
                           refreshRequestFinished(*json);
                       } else {
                           refreshRequestFailed();
                       }
                });
}

void PaginatedResource::refreshRequestFinished(const QJsonDocument &json)
{
    m_data.clear();
    const QJsonArray data = json["data"_L1].toArray();
    for (const auto &entry : data)
        m_data.append(entry.toObject());
    m_pages = json[totalPagesField].toInt();
    m_currentPage = json[currentPageField].toInt();
    emit pageUpdated();
    emit pagesUpdated();
    emit dataUpdated();
}

void PaginatedResource::refreshRequestFailed()
{
    if (m_currentPage != 1) {
        // A failed refresh. If we weren't on page 1, try that.
        // Last resource on currentPage might have been deleted, causing a failure
        setPage(1);
    } else {
        // Refresh failed and we we're already on page 1 => clear data
        m_pages = 0;
        emit pagesUpdated();
        if (!m_data.isEmpty()) {
            m_data.clear();
            emit dataUpdated();
        }
    }
}

void PaginatedResource::update(const QVariantMap &data, int id)
{
    m_manager->put(m_api->createRequest(m_path + resourceId.arg(QString::number(id))), data,
                   this, [this](QRestReply &reply) {
                       if (reply.isSuccess())
                           refreshCurrentPage();
                   });
}

void PaginatedResource::add(const QVariantMap &data)
{
    m_manager->post(m_api->createRequest(m_path), data, this, [this](QRestReply &reply) {
        if (reply.isSuccess())
            refreshCurrentPage();
    });
}

void PaginatedResource::remove(int id)
{
    m_manager->deleteResource(m_api->createRequest(m_path + resourceId.arg(QString::number(id))),
                              this, [this](QRestReply &reply) {
                                  if (reply.isSuccess())
                                      refreshCurrentPage();
                              });
}
