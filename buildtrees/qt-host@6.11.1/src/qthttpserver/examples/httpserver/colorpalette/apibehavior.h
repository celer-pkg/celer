// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#ifndef APIBEHAVIOR_H
#define APIBEHAVIOR_H

#include "types.h"
#include "utils.h"

#include <QtHttpServer/QHttpServer>
#include <QtConcurrent/qtconcurrentrun.h>

#include <optional>

template<typename T, typename = void>
class CrudApi
{
};

template<typename T>
class CrudApi<T,
              std::enable_if_t<std::conjunction_v<std::is_base_of<Jsonable, T>,
                                                  std::is_base_of<Updatable, T>>>>
{
public:
    explicit CrudApi(const IdMap<T> &data, std::unique_ptr<FromJsonFactory<T>> factory)
        : data(data), factory(std::move(factory))
    {
    }

    QFuture<QHttpServerResponse> getPaginatedList(const QHttpServerRequest &request) const
    {
        using PaginatorType = Paginator<IdMap<T>>;
        std::optional<qsizetype> maybePage;
        std::optional<qsizetype> maybePerPage;
        std::optional<qint64> maybeDelay;
        if (request.query().hasQueryItem("page"))
            maybePage = request.query().queryItemValue("page").toLongLong();
        if (request.query().hasQueryItem("per_page"))
            maybePerPage = request.query().queryItemValue("per_page").toLongLong();
        if (request.query().hasQueryItem("delay"))
            maybeDelay = request.query().queryItemValue("delay").toLongLong();

        if ((maybePage && *maybePage < 1) || (maybePerPage && *maybePerPage < 1)) {
            return QtConcurrent::run([]() {
                return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);
            });
        }

        PaginatorType paginator(data, maybePage ? *maybePage : PaginatorType::defaultPage,
                                maybePerPage ? *maybePerPage : PaginatorType::defaultPageSize);

        return QtConcurrent::run([paginator = std::move(paginator), maybeDelay]() {
            if (maybeDelay)
                QThread::sleep(*maybeDelay);
            return paginator.isValid()
                    ? QHttpServerResponse(paginator.toJson())
                    : QHttpServerResponse(QHttpServerResponder::StatusCode::NoContent);
        });
    }

    QHttpServerResponse getItem(qint64 itemId) const
    {
        const auto item = data.find(itemId);
        return item != data.end() ? QHttpServerResponse(item->toJson())
                                  : QHttpServerResponse(QHttpServerResponder::StatusCode::NotFound);
    }

    //! [POST return different status code example]
    QHttpServerResponse postItem(const QHttpServerRequest &request)
    {
        const std::optional<QJsonObject> json = byteArrayToJsonObject(request.body());
        if (!json)
            return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);

        const std::optional<T> item = factory->fromJson(*json);
        if (!item)
            return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);
        if (data.contains(item->id))
            return QHttpServerResponse(QHttpServerResponder::StatusCode::AlreadyReported);

        const auto entry = data.insert(item->id, *item);
        return QHttpServerResponse(entry->toJson(), QHttpServerResponder::StatusCode::Created);
    }
    //! [POST return different status code example]

    QHttpServerResponse updateItem(qint64 itemId, const QHttpServerRequest &request)
    {
        const std::optional<QJsonObject> json = byteArrayToJsonObject(request.body());
        if (!json)
            return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);

        auto item = data.find(itemId);
        if (item == data.end())
            return QHttpServerResponse(QHttpServerResponder::StatusCode::NoContent);
        if (!item->update(*json))
            return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);

        return QHttpServerResponse(item->toJson());
    }

    QHttpServerResponse updateItemFields(qint64 itemId, const QHttpServerRequest &request)
    {
        const std::optional<QJsonObject> json = byteArrayToJsonObject(request.body());
        if (!json)
            return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);

        auto item = data.find(itemId);
        if (item == data.end())
            return QHttpServerResponse(QHttpServerResponder::StatusCode::NoContent);
        item->updateFields(*json);

        return QHttpServerResponse(item->toJson());
    }

    QHttpServerResponse deleteItem(qint64 itemId)
    {
        if (!data.remove(itemId))
            return QHttpServerResponse(QHttpServerResponder::StatusCode::NoContent);
        return QHttpServerResponse(QHttpServerResponder::StatusCode::Ok);
    }

private:
    IdMap<T> data;
    std::unique_ptr<FromJsonFactory<T>> factory;
};

class SessionApi
{
public:
    explicit SessionApi(const IdMap<SessionEntry> &sessions,
                        std::unique_ptr<FromJsonFactory<SessionEntry>> factory)
        : sessions(sessions), factory(std::move(factory))
    {
    }

    QHttpServerResponse registerSession(const QHttpServerRequest &request)
    {
        const auto json = byteArrayToJsonObject(request.body());
        if (!json)
            return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);
        const auto item = factory->fromJson(*json);
        if (!item)
            return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);

        const auto session = sessions.insert(item->id, *item);
        session->startSession();
        return QHttpServerResponse(session->toJson());
    }

    QHttpServerResponse login(const QHttpServerRequest &request)
    {
        const auto json = byteArrayToJsonObject(request.body());

        if (!json || !json->contains("email") || !json->contains("password"))
            return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);

        auto maybeSession = std::find_if(
                sessions.begin(), sessions.end(),
                [email = json->value("email").toString(),
                 password = json->value("password").toString()](const auto &it) {
                    return it.password == password && it.email == email;
                });
        if (maybeSession == sessions.end()) {
            return QHttpServerResponse(QHttpServerResponder::StatusCode::NotFound);
        }
        maybeSession->startSession();
        return QHttpServerResponse(maybeSession->toJson());
    }

    QHttpServerResponse logout(const QHttpServerRequest &request)
    {
        const auto maybeToken = getTokenFromRequest(request);
        if (!maybeToken)
            return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);

        auto maybeSession = std::find(sessions.begin(), sessions.end(), *maybeToken);
        if (maybeSession != sessions.end())
            maybeSession->endSession();
        return QHttpServerResponse(QHttpServerResponder::StatusCode::Ok);
    }

    bool authorize(const QHttpServerRequest &request) const
    {
        const auto maybeToken = getTokenFromRequest(request);
        if (maybeToken) {
            const auto maybeSession = std::find(sessions.begin(), sessions.end(), *maybeToken);
            return maybeSession != sessions.end() && *maybeSession == *maybeToken;
        }
        return false;
    }

private:
    IdMap<SessionEntry> sessions;
    std::unique_ptr<FromJsonFactory<SessionEntry>> factory;
};

#endif // APIBEHAVIOR_H
