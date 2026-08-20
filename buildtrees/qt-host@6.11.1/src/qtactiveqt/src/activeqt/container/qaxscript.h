// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:significant reason:default


#ifndef QAXSCRIPT_H
#define QAXSCRIPT_H

#include <QtAxContainer/qaxobject.h>

struct IActiveScript;

QT_BEGIN_NAMESPACE

class QAxBase;
class QAxScript;
class QAxScriptSite;
class QAxScriptEngine;
class QAxScriptManager;
class QAxScriptManagerPrivate;

class QAxScriptEngine : public QAxObject
{
public:
    enum State {
        Uninitialized = 0,
        Initialized = 5,
        Started = 1,
        Connected = 2,
        Disconnected = 3,
        Closed = 4
    };

    QAxScriptEngine(const QString &language, QAxScript *script);
    ~QAxScriptEngine() override;

    bool isValid() const;
    bool hasIntrospection() const;

    QString scriptLanguage() const;

    State state() const;
    void setState(State st);

    void addItem(const QString &name);

    long queryInterface(const QUuid &, void**) const;

protected:
    bool initialize(IUnknown** ptr) override;

private:
    QAxScript *script_code;
    IActiveScript *engine;

    QString script_language;
};

class QAxScript : public QObject
{
    Q_OBJECT

public:
    enum FunctionFlags {
        FunctionNames = 0,
        FunctionSignatures
    };

    QAxScript(const QString &name, QAxScriptManager *manager);
    ~QAxScript() override;

    bool load(const QString &code, const QString &language = QString());

    QStringList functions(FunctionFlags = FunctionNames) const;

    QString scriptCode() const;
    QString scriptName() const;
    QAxScriptEngine *scriptEngine() const;

    QVariant call(const QString &function, const QVariant &v1 = QVariant(),
                                           const QVariant &v2 = QVariant(),
                                           const QVariant &v3 = QVariant(),
                                           const QVariant &v4 = QVariant(),
                                           const QVariant &v5 = QVariant(),
                                           const QVariant &v6 = QVariant(),
                                           const QVariant &v7 = QVariant(),
                                           const QVariant &v8 = QVariant());
    QVariant call(const QString &function, QList<QVariant> &arguments);

Q_SIGNALS:
    void entered();
    void finished();
    void finished(const QVariant &result);
    void finished(int code, const QString &source,const QString &description, const QString &help);
    void stateChanged(int state);
    void error(int code, const QString &description, int sourcePosition, const QString &sourceText);

private:
    friend class QAxScriptSite;
    friend class QAxScriptEngine;

    void updateObjects();
    QAxBase *findObject(const QString &name);

    QString script_name;
    QString script_code;
    QAxScriptManager *script_manager;
    QAxScriptEngine *script_engine;
    QAxScriptSite *script_site;
};

class QAxScriptManager : public QObject
{
    Q_OBJECT

public:
    explicit QAxScriptManager(QObject *parent = nullptr);
    ~QAxScriptManager() override;

    void addObject(QAxBase *object);
    void addObject(QObject *object);

    QStringList functions(QAxScript::FunctionFlags = QAxScript::FunctionNames) const;
    QStringList scriptNames() const;
    QAxScript *script(const QString &name) const;

    QAxScript* load(const QString &code, const QString &name, const QString &language);
    QAxScript* load(const QString &file, const QString &name);

    QVariant call(const QString &function, const QVariant &v1 = QVariant(),
                                           const QVariant &v2 = QVariant(),
                                           const QVariant &v3 = QVariant(),
                                           const QVariant &v4 = QVariant(),
                                           const QVariant &v5 = QVariant(),
                                           const QVariant &v6 = QVariant(),
                                           const QVariant &v7 = QVariant(),
                                           const QVariant &v8 = QVariant());
    QVariant call(const QString &function, QList<QVariant> &arguments);

    static bool registerEngine(const QString &name, const QString &extension, const QString &code = QString());
    static QString scriptFileFilter();

Q_SIGNALS:
    void error(QAxScript *script, int code, const QString &description, int sourcePosition, const QString &sourceText);

private:
    friend class QAxScript;
    QAxScriptManagerPrivate *d;
};


// QAxScript inlines

inline QString QAxScript::scriptCode() const
{
    return script_code;
}

inline QString QAxScript::scriptName() const
{
    return script_name;
}

inline QAxScriptEngine *QAxScript::scriptEngine() const
{
    return script_engine;
}

// QAxScriptEngine inlines

inline bool QAxScriptEngine::isValid() const
{
    return engine != nullptr;
}

inline QString QAxScriptEngine::scriptLanguage() const
{
    return script_language;
}

// QAxScriptManager inlines

extern QAxBase *qax_create_object_wrapper(QObject*);

inline void QAxScriptManager::addObject(QObject *object)
{
    QAxBase *wrapper = qax_create_object_wrapper(object);
    if (!wrapper) {
        qWarning("QAxScriptMananger::addObject: Class %s not exposed through the QAxFactory",
            object->metaObject()->className());
        Q_ASSERT(wrapper);
    }
    addObject(wrapper);
}

QT_END_NAMESPACE

#endif // QAXSCRIPT_H
