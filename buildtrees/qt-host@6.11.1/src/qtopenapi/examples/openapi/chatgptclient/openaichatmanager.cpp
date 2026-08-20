// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "openaichatmanager.h"

#include "modelsapi.h"
#include "responsesapi.h"

#include <QMap>

using namespace QtOpenAPI;
using namespace QtOpenAPI::Examples;

static QMap<OpenAIChatManager::CharacterMode, QString> DefaultCharacters = {
    {
        OpenAIChatManager::CharacterMode::Wizard,
        "You are an 80th-level wizard. You know everything about the magic and magical creatures. "
        "You always can give the information about any magical topic. "
        "You don't give information about topics unrelated to the magic. "
        "If someone asks you about anything unrelated to magic topics, "
        "you politely suggest to switch to the magic related stuff. "
        "Your dream is to find a student to teach the magic. "
        "Also, you like jokes and often use metaphors and humor to explain magical concepts."
        "If a person continues to ask you about unrelated to the magic topics, "
        "you cast the 'Obliviate' spell on the person trying make him to forget the unrelated "
        "topic! You simply do it when the time is right. If person doesn't forget the topic, "
        "you say him he has a strong magician skills and suggest to train magic spells together."
    },
    {
        OpenAIChatManager::CharacterMode::Scientist,
        "You are Albert Einstein. You speak, think, and behave exactly like Albert Einstein, "
        "reflecting both his public persona and personal quirks. You possess complete knowledge "
        "of his life, thoughts, and work, as well as a deep understanding of physics, including "
        "the latest discoveries. Your are passionately devoted to physics, but you prefer "
        "to keep your answers simple. "
        "You are a follower of the string theory. But you start speaking about it only "
        "if you were asked to speak about the string theory. You never suggest to speak "
        "about the string theory first. If a person asks you about topics unrelated to physics, "
        "you redirect the conversation to physics. "
        "If the person insists to speak about unrelated to physics topics more then three times, "
        "you say: 'Two things are infinite: the universe and human stupidity; "
        "and I'm not sure about the universe'. "
        "Note, you should use humor to explain complex concepts in true Einstein style."
    },
    {
        OpenAIChatManager::CharacterMode::DefaultMode,
        "" // Normal Chat mode, nothing to add
    }
};

OpenAIChatManager::OpenAIChatManager(QObject *parent)
    : QObject{parent}
{
    m_responseApi = new QtOpenAPI::ResponsesApi(this);
    m_modelsApi = new ModelsApi(this);

    QByteArray envKey = qgetenv("CHAT_OPENAI_KEY");
    if (!envKey.isEmpty()) {
        m_apiKey = QString::fromLatin1(envKey);
        m_responseApi->setBearerToken(m_apiKey);
        m_modelsApi->setBearerToken(m_apiKey);

        // request model list once at singleton creation
        requestModelList();
    } else {
        qCritical() << "Please, set CHAT_OPENAI_KEY environment variable for using the example.";
    }
}

OpenAIChatManager::~OpenAIChatManager()
{
}

QString OpenAIChatManager::modelName() const
{
    return m_modelName;
}

void OpenAIChatManager::setModelName(const QString &newName)
{
    if (m_modelName == newName)
        return;
    m_modelName = newName;
    // Well, it's not necessary, but we can clean it. Just in case.
    m_responseId.clear();
    emit modelNameChanged();
}

QString OpenAIChatManager::userRequest() const
{
    return m_userRequest;
}

void OpenAIChatManager::setUserRequest(const QString &request)
{
    if (m_userRequest == request)
        return;
    m_userRequest = request;
    emit userRequestChanged();
}

QStringList OpenAIChatManager::modelList()
{
    return m_modelList;
}

void OpenAIChatManager::setModelList(const QStringList &list)
{
    if (m_modelList == list)
        return;
    m_modelList = list;
    emit modelListChanged();
}

OpenAIChatManager::CharacterMode OpenAIChatManager::characterId() const
{
    return m_characterId;
}

void OpenAIChatManager::setCharacterId(CharacterMode character)
{
    if (m_characterId == character)
        return;
    m_characterId = character;
    emit characterIdChanged();
}

QString OpenAIChatManager::customUserCharacter() const
{
    return m_customUserCharacter;
}

void OpenAIChatManager::setCustomUserCharacter(const QString &character)
{
    if (m_customUserCharacter == character)
        return;
    m_customUserCharacter = character;
    emit customUserCharacterChanged();
}

void OpenAIChatManager::sendUserRequest()
{
    GenerateResponse response;
    response.setModel(m_modelName);
    response.setStore(true);
    response.setInput(m_userRequest);
    response.setInstructions((m_characterId == OpenAIChatManager::CharacterMode::UserCharacter)
                             ? m_customUserCharacter : DefaultCharacters.value(m_characterId));
    if (!m_responseId.isEmpty())
        response.setPreviousResponseId(m_responseId);

    //! [0]
    m_responseApi->createResponse(response, this, [&](const QRestReply &reply,
                                                      const Response &summary) {
        if (!reply.isSuccess()) {
            qWarning() << "createResponse:" << reply.errorString() << reply.error();
        } else {
            // To save the context of conversation with the openAI model,
            // we need to track returning ResponseId of the last model's message.
            // Context becomes irrelevant after switching to  the other model. If the model
            // doesn't know the context id, it will response like if the ResponseId is empty.
            m_responseId = summary.getIdValue();
            const QList<OutputMessage> outputMessage = summary.getOutputValue();
            for (qsizetype msgIndex = 0; msgIndex < outputMessage.size(); msgIndex++) {
                const QList<OutputTextContent> messages
                    = outputMessage.at(msgIndex).getContentValue();
                for (qsizetype contentIndex = 0; contentIndex < messages.size(); contentIndex++)
                    emit responseReady(messages.at(contentIndex).getTextValue());
            }
        }
    });
    //! [0]
}

void OpenAIChatManager::requestModelList()
{
    m_modelsApi->listModels(this, [&](const QRestReply &reply,
                                      const ListModelsResponse &summary) {
        if (!reply.isSuccess()) {
            qWarning() << "listModels:" << reply.errorString() << reply.error();
            m_modelList.clear();
        } else {
            for (qsizetype modelIndex = 0; modelIndex < summary.getDataValue().size(); modelIndex++) {
                const QString modelName = summary.getDataValue().at(modelIndex).getIdValue();
                // Note, the list of available models contains unrelated stuff,
                // like image generating models or search models. We canot use them
                // for text mode interactions. So, removing them here.
                if (!m_modelList.contains(modelName)
                    && modelName.contains("gpt", Qt::CaseInsensitive)
                    && !modelName.contains("audio", Qt::CaseInsensitive)
                    && !modelName.contains("image", Qt::CaseInsensitive)
                    && !modelName.contains("search", Qt::CaseInsensitive)
                    && !modelName.contains("realtime", Qt::CaseInsensitive)
                    && !modelName.contains("transcribe", Qt::CaseInsensitive)
                    && !modelName.contains("-pro-", Qt::CaseInsensitive)
                    && !modelName.contains("tts", Qt::CaseInsensitive)) {
                    m_modelList.append(modelName);
                }
            }
        }
        emit modelListChanged();
    });
}
