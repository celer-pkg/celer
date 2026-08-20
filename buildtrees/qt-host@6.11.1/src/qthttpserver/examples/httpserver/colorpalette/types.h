// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#ifndef TYPES_H
#define TYPES_H

#include <QtGui/QColor>
#include <QtCore/QDateTime>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonParseError>
#include <QtCore/QString>
#include <QtCore/qtypes.h>

#include <algorithm>
#include <optional>

struct Jsonable
{
    virtual QJsonObject toJson() const = 0;
    virtual ~Jsonable() = default;
};

struct Updatable
{
    virtual bool update(const QJsonObject &json) = 0;
    virtual void updateFields(const QJsonObject &json) = 0;
    virtual ~Updatable() = default;
};

template<typename T>
struct FromJsonFactory
{
    virtual std::optional<T> fromJson(const QJsonObject &json) const = 0;
    virtual ~FromJsonFactory() = default;
};

struct User : public Jsonable, public Updatable
{
    qint64 id;
    QString email;
    QString firstName;
    QString lastName;
    QUrl avatarUrl;
    QDateTime createdAt;
    QDateTime updatedAt;

    explicit User(const QString &email, const QString &firstName, const QString &lastName,
                  const QUrl &avatarUrl,
                  const QDateTime &createdAt = QDateTime::currentDateTimeUtc(),
                  const QDateTime &updatedAt = QDateTime::currentDateTimeUtc())
        : id(nextId()),
          email(email),
          firstName(firstName),
          lastName(lastName),
          avatarUrl(avatarUrl),
          createdAt(createdAt),
          updatedAt(updatedAt)
    {
    }

    bool update(const QJsonObject &json) override
    {
        if (!json.contains("email") || !json.contains("first_name") || !json.contains("last_name")
            || !json.contains("avatar"))
            return false;

        email = json.value("email").toString();
        firstName = json.value("first_name").toString();
        lastName = json.value("last_name").toString();
        avatarUrl.setPath(json.value("avatar").toString());
        updateTimestamp();
        return true;
    }

    void updateFields(const QJsonObject &json) override
    {
        if (json.contains("email"))
            email = json.value("email").toString();
        if (json.contains("first_name"))
            firstName = json.value("first_name").toString();
        if (json.contains("last_name"))
            lastName = json.value("last_name").toString();
        if (json.contains("avatar"))
            avatarUrl.setPath(json.value("avatar").toString());
        updateTimestamp();
    }

    QJsonObject toJson() const override
    {
        return QJsonObject{ { "id", id },
                            { "email", email },
                            { "first_name", firstName },
                            { "last_name", lastName },
                            { "avatar", avatarUrl.toString() },
                            { "createdAt", createdAt.toString(Qt::ISODateWithMs) },
                            { "updatedAt", updatedAt.toString(Qt::ISODateWithMs) } };
    }

private:
    void updateTimestamp() { updatedAt = QDateTime::currentDateTimeUtc(); }

    static qint64 nextId()
    {
        static qint64 lastId = 1;
        return lastId++;
    }
};

struct UserFactory : public FromJsonFactory<User>
{
    UserFactory(const QString &scheme, const QString &hostName, int port)
        : scheme(scheme), hostName(hostName), port(port)
    {
    }

    std::optional<User> fromJson(const QJsonObject &json) const override
    {
        if (!json.contains("email") || !json.contains("first_name") || !json.contains("last_name")
            || !json.contains("avatar")) {
            return std::nullopt;
        }

        if (json.contains("createdAt") && json.contains("updatedAt")) {
            return User(
                    json.value("email").toString(), json.value("first_name").toString(),
                    json.value("last_name").toString(), json.value("avatar").toString(),
                    QDateTime::fromString(json.value("createdAt").toString(), Qt::ISODateWithMs),
                    QDateTime::fromString(json.value("updatedAt").toString(), Qt::ISODateWithMs));
        }
        QUrl avatarUrl(json.value("avatar").toString());
        if (!avatarUrl.isValid()) {
            avatarUrl.setPath(json.value("avatar").toString());
        }
        avatarUrl.setScheme(scheme);
        avatarUrl.setHost(hostName);
        avatarUrl.setPort(port);

        return User(json.value("email").toString(), json.value("first_name").toString(),
                    json.value("last_name").toString(), avatarUrl);
    }

private:
    QString scheme;
    QString hostName;
    int port;
};

struct Color : public Jsonable, public Updatable
{
    qint64 id;
    QString name;
    QColor color;
    QString pantone;
    QDateTime createdAt;
    QDateTime updatedAt;

    explicit Color(const QString &name, const QString &color, const QString &pantone,
                   const QDateTime &createdAt = QDateTime::currentDateTimeUtc(),
                   const QDateTime &updatedAt = QDateTime::currentDateTimeUtc())
        : id(nextId()),
          name(name),
          color(QColor(color)),
          pantone(pantone),
          createdAt(createdAt),
          updatedAt(updatedAt)
    {
    }

    QJsonObject toJson() const override
    {
        return QJsonObject{ { "id", id },
                            { "name", name },
                            { "color", color.name() },
                            { "pantone_value", pantone },
                            { "createdAt", createdAt.toString(Qt::ISODateWithMs) },
                            { "updatedAt", updatedAt.toString(Qt::ISODateWithMs) } };
    }

    bool update(const QJsonObject &json) override
    {
        if (!json.contains("name") || !json.contains("color") || !json.contains("pantone_value"))
            return false;

        name = json.value("name").toString();
        color = QColor(json.value("color").toString());
        pantone = json.value("pantone_value").toString();
        updateTimestamp();
        return true;
    }

    void updateFields(const QJsonObject &json) override
    {
        if (json.contains("name"))
            name = json.value("name").toString();
        if (json.contains("color"))
            color = QColor(json.value("color").toString());
        if (json.contains("pantone_value"))
            pantone = json.value("pantone_value").toString();
        updateTimestamp();
    }

private:
    void updateTimestamp() { updatedAt = QDateTime::currentDateTimeUtc(); }

    static qint64 nextId()
    {
        static qint64 lastId = 1;
        return lastId++;
    }
};

struct ColorFactory : public FromJsonFactory<Color>
{
    std::optional<Color> fromJson(const QJsonObject &json) const override
    {
        if (!json.contains("name") || !json.contains("color") || !json.contains("pantone_value"))
            return std::nullopt;
        if (json.contains("createdAt") && json.contains("updatedAt")) {
            return Color(
                    json.value("name").toString(), json.value("color").toString(),
                    json.value("pantone_value").toString(),
                    QDateTime::fromString(json.value("createdAt").toString(), Qt::ISODateWithMs),
                    QDateTime::fromString(json.value("updatedAt").toString(), Qt::ISODateWithMs));
        }
        return Color(json.value("name").toString(), json.value("color").toString(),
                     json.value("pantone_value").toString());
    }
};

struct SessionEntry : public Jsonable
{
    qint64 id;
    QString email;
    QString password;
    std::optional<QUuid> token;

    explicit SessionEntry(const QString &email, const QString &password)
        : id(nextId()), email(email), password(password)
    {
    }

    void startSession() { token = generateToken(); }

    void endSession() { token = std::nullopt; }

    QJsonObject toJson() const override
    {
        return token
                ? QJsonObject{ { "id", id },
                               { "token", token->toString(QUuid::StringFormat::WithoutBraces) } }
                : QJsonObject{};
    }

    bool operator==(const QString &otherToken) const
    {
        return token && *token == QUuid::fromString(otherToken);
    }

private:
    QUuid generateToken() { return QUuid::createUuid(); }

    static qint64 nextId()
    {
        static qint64 lastId = 1;
        return lastId++;
    }
};

struct SessionEntryFactory : public FromJsonFactory<SessionEntry>
{
    std::optional<SessionEntry> fromJson(const QJsonObject &json) const override
    {
        if (!json.contains("email") || !json.contains("password"))
            return std::nullopt;

        return SessionEntry(json.value("email").toString(), json.value("password").toString());
    }
};

template<typename T>
class IdMap : public QMap<qint64, T>
{
public:
    IdMap() = default;
    explicit IdMap(const FromJsonFactory<T> &factory, const QJsonArray &array) : QMap<qint64, T>()
    {
        for (const auto &jsonValue : array) {
            if (jsonValue.isObject()) {
                const auto maybeT = factory.fromJson(jsonValue.toObject());
                if (maybeT) {
                    QMap<qint64, T>::insert(maybeT->id, *maybeT);
                }
            }
        }
    }
};

template<typename T>
class Paginator : public Jsonable
{
public:
    static constexpr qsizetype defaultPage = 1;
    static constexpr qsizetype defaultPageSize = 6;

    explicit Paginator(const T &container, qsizetype page, qsizetype size)
    {
        const auto containerSize = container.size();
        const auto pageIndex = page - 1;
        const auto pageSize = qMin(size, containerSize);
        const auto totalPages = (containerSize % pageSize) == 0 ? (containerSize / pageSize)
                                                                : (containerSize / pageSize) + 1;
        valid = containerSize > (pageIndex * pageSize);
        if (valid) {
            QJsonArray data;

            auto iter = container.begin();
            std::advance(iter, std::min(pageIndex * pageSize, containerSize));
            for (qsizetype i = 0; i < pageSize && iter != container.end(); ++i, ++iter) {
                data.push_back(iter->toJson());
            }
            json = QJsonObject{ { "page", pageIndex + 1 },
                                { "per_page", pageSize },
                                { "total", containerSize },
                                { "total_pages", totalPages },
                                { "data", data } };
        } else {
            json = QJsonObject{};
        }
    }

    QJsonObject toJson() const { return json; }

    constexpr bool isValid() const { return valid; }

private:
    QJsonObject json;
    bool valid;
};

#endif // TYPES_H
