// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef OPENAICHATMANAGER_H
#define OPENAICHATMANAGER_H

#include <QObject>
#include <QtQml/QtQml>

namespace QtOpenAPI {
    class ResponsesApi;
    class ModelsApi;
}
namespace QtOpenAPI::Examples {
//! [0]
class OpenAIChatManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString modelName READ modelName WRITE setModelName NOTIFY modelNameChanged FINAL)
    Q_PROPERTY(QString userRequest READ userRequest
               WRITE setUserRequest NOTIFY userRequestChanged FINAL)
    Q_PROPERTY(QStringList modelList READ modelList
               WRITE setModelList NOTIFY modelListChanged FINAL)
    Q_PROPERTY(CharacterMode characterId READ characterId
               WRITE setCharacterId NOTIFY characterIdChanged FINAL)
    Q_PROPERTY(QString customUserCharacter READ customUserCharacter
               WRITE setCustomUserCharacter NOTIFY customUserCharacterChanged FINAL)
//! [0]
public:
    enum class CharacterMode {
        Wizard = 0,
        Scientist,
        DefaultMode,
        UserCharacter
    };
    Q_ENUM(CharacterMode)
    explicit OpenAIChatManager(QObject *parent = nullptr);
    ~OpenAIChatManager();

    QString modelName() const;
    void setModelName(const QString &newName);

    QString userRequest() const;
    void setUserRequest(const QString &request);

    QStringList modelList();
    void setModelList(const QStringList &list);

    CharacterMode characterId() const;
    void setCharacterId(CharacterMode character);

    QString customUserCharacter() const;
    void setCustomUserCharacter(const QString &character);

//! [1]
    Q_INVOKABLE void sendUserRequest();
//! [1]

signals:
    void modelNameChanged();
    void userRequestChanged();
    void modelListChanged();
    void characterIdChanged();
    void customUserCharacterChanged();
    void responseReady(const QString &response);

private:
    void requestModelList();

    QString m_apiKey;
    QString m_modelName;
    QString m_userRequest;
    QString m_responseId;
    QtOpenAPI::ResponsesApi *m_responseApi;
    QtOpenAPI::ModelsApi *m_modelsApi;
    QStringList m_modelList;
    CharacterMode m_characterId = CharacterMode::Wizard;
    QString m_customUserCharacter;
};
}

#endif // OPENAICHATMANAGER_H
