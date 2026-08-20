// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:significant reason:default


#ifndef QAXFACTORY_H
#define QAXFACTORY_H

#include <QtCore/qcompilerdetection.h>

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Woverloaded-virtual") // gcc complains about QObject::metaObject() being hidden.
QT_WARNING_DISABLE_CLANG("-Woverloaded-virtual") // clang-cl complains about QObject::metaObject() being hidden.

#include <QtCore/qhash.h>
#include <QtCore/quuid.h>
#include <QtCore/qfactoryinterface.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qsettings.h>

struct IUnknown;
struct IDispatch;

QT_BEGIN_NAMESPACE

class QWidget;
class QSettings;

class QAxFactory : public QObject
{
    Q_DISABLE_COPY_MOVE(QAxFactory)
public:
    QAxFactory(const QUuid &libId, const QUuid &appId);
    ~QAxFactory() override;

    virtual QStringList featureList() const = 0;

    virtual QObject *createObject(const QString &key) = 0;
    virtual const QMetaObject *metaObject(const QString &key) const = 0;
    virtual bool createObjectWrapper(QObject *object, IDispatch **wrapper);

    virtual QUuid classID(const QString &key) const;
    virtual QUuid interfaceID(const QString &key) const;
    virtual QUuid eventsID(const QString &key) const;

    virtual QUuid typeLibID() const;
    virtual QUuid appID() const;

    virtual void registerClass(const QString &key, QSettings *) const;
    virtual void unregisterClass(const QString &key, QSettings *) const;

    virtual bool validateLicenseKey(const QString &key, const QString &licenseKey) const;

    virtual QString exposeToSuperClass(const QString &key) const;
    virtual bool stayTopLevel(const QString &key) const;
    virtual bool hasStockEvents(const QString &key) const;
    virtual bool isService() const;

    enum ServerType {
        SingleInstance,
        MultipleInstances
    };

    static bool isServer();
    static QString serverDirPath();
    static QString serverFilePath();
    static bool startServer(ServerType type = MultipleInstances);
    static bool stopServer();

    static bool registerActiveObject(QObject *object);

private:
    QUuid typelib;
    QUuid app;
};

extern QAxFactory *qAxFactory();

extern bool qax_startServer(QAxFactory::ServerType);

inline bool QAxFactory::startServer(ServerType type)
{
    // implementation in qaxservermain.cpp
    return qax_startServer(type);
}

extern bool qax_stopServer();

inline bool QAxFactory::stopServer()
{
    // implementation in qaxservermain.cpp
    return qax_stopServer();
}

#define QAXFACTORY_EXPORT(IMPL, TYPELIB, APPID) \
    QT_BEGIN_NAMESPACE \
    QAxFactory *qax_instantiate()               \
    {                                                   \
        return new IMPL(QUuid(TYPELIB), QUuid(APPID));    \
    } \
    QT_END_NAMESPACE

#define QAXFACTORY_DEFAULT(Class, IIDClass, IIDInterface, IIDEvents, IIDTypeLib, IIDApp) \
    QT_BEGIN_NAMESPACE \
    class QAxDefaultFactory : public QAxFactory \
    { \
    public: \
        explicit QAxDefaultFactory(const QUuid &app, const QUuid &lib) \
        : QAxFactory(app, lib), className(QLatin1String(#Class)) {} \
        QStringList featureList() const override \
        { \
            return {className}; \
        } \
        const QMetaObject *metaObject(const QString &key) const override \
        { \
            if (key == className) \
                return &Class::staticMetaObject; \
            return nullptr; \
        } \
        QObject *createObject(const QString &key) override \
        { \
            if (key == className) \
                return new Class(nullptr); \
            return nullptr; \
        } \
        QUuid classID(const QString &key) const override \
        { \
            if (key == className) \
                return QUuid(IIDClass); \
            return QUuid(); \
        } \
        QUuid interfaceID(const QString &key) const override \
        { \
            if (key == className) \
                return QUuid(IIDInterface); \
            return QUuid(); \
        } \
        QUuid eventsID(const QString &key) const override \
        { \
            if (key == className) \
                return QUuid(IIDEvents); \
            return QUuid(); \
        } \
    private: \
        QString className; \
    }; \
    QT_END_NAMESPACE \
    QAXFACTORY_EXPORT(QAxDefaultFactory, IIDTypeLib, IIDApp) \

template<class T>
class QAxClass : public QAxFactory
{
public:
    explicit QAxClass(const QString &libId, const QString &appId)
    : QAxFactory(QUuid(libId), QUuid(appId))
    {}

    const QMetaObject *metaObject(const QString &) const override { return &T::staticMetaObject; }
    QStringList featureList() const override
    {
        return {QLatin1String(T::staticMetaObject.className())};
    }

    QObject *createObject(const QString &key) override
    {
        const QMetaObject &mo = T::staticMetaObject;
        if (key != QLatin1String(mo.className()))
            return nullptr;
        if (!qstrcmp(mo.classInfo(mo.indexOfClassInfo("Creatable")).value(), "no"))
            return nullptr;
        return new T(nullptr);
    }

    void registerClass(const QString &key, QSettings *settings) const override
    {
        const QStringList categories = getImplementedCategories();

        for (const auto &cat : categories) {
            settings->setValue(QLatin1String("/CLSID/") + classID(key).toString()
                               + QLatin1String("/Implemented Categories/") + cat + QLatin1String("/."),
                               QString());
        }
    }

    void unregisterClass(const QString &key, QSettings *settings) const override
    {
        const QStringList categories = getImplementedCategories();

        for (const auto &cat : categories) {
            settings->remove(QLatin1String("/CLSID/") + classID(key).toString()
                             + QLatin1String("/Implemented Categories/") + cat + QLatin1String("/."));
        }
    }

private:
    /*! Retrieve list of comma-separated "Implemented Categories" Q_CLASSINFO UUIDs from T. */
    static QStringList getImplementedCategories()
    {
        const QMetaObject &mo = T::staticMetaObject;
        QString catids = QLatin1String(mo.classInfo(mo.indexOfClassInfo("Implemented Categories")).value());
        return catids.split(QLatin1Char(','));
    }
};

#define QAXFACTORY_BEGIN(IDTypeLib, IDApp) \
    QT_BEGIN_NAMESPACE \
    class QAxFactoryList : public QAxFactory \
    { \
        QStringList factoryKeys; \
        QHash<QString, QAxFactory*> factories; \
        QHash<QString, bool> creatable; \
    public: \
        QAxFactoryList() \
        : QAxFactory(QUuid(IDTypeLib), QUuid(IDApp)) \
        { \
            QAxFactory *factory = nullptr; \
            QStringList keys; \

#define QAXCLASS(Class) \
            factory = new QAxClass<Class>(typeLibID().toString(), appID().toString()); \
            qRegisterMetaType<Class*>(#Class"*"); \
            keys = factory->featureList(); \
            for (const QString &key : std::as_const(keys)) { \
                factoryKeys += key; \
                factories.insert(key, factory); \
                creatable.insert(key, true); \
            }\

#define QAXTYPE(Class) \
            factory = new QAxClass<Class>(typeLibID().toString(), appID().toString()); \
            qRegisterMetaType<Class*>(#Class"*"); \
            keys = factory->featureList(); \
            for (const QString &key : std::as_const(keys)) { \
                factoryKeys += key; \
                factories.insert(key, factory); \
                creatable.insert(key, false); \
            }\

#define QAXFACTORY_END() \
        } \
        ~QAxFactoryList() override { qDeleteAll(factories); } \
        QStringList featureList() const override {  return factoryKeys; } \
        const QMetaObject *metaObject(const QString&key) const override \
        { \
            QAxFactory *f = factories.value(key); \
            return f ? f->metaObject(key) : nullptr; \
        } \
        QObject *createObject(const QString &key) override { \
            if (!creatable.value(key)) \
                return nullptr; \
            QAxFactory *f = factories.value(key); \
            return f ? f->createObject(key) : nullptr; \
        } \
        QUuid classID(const QString &key) const override { \
            QAxFactory *f = factories.value(key); \
            return f ? f->classID(key) : QUuid(); \
        } \
        QUuid interfaceID(const QString &key) const override { \
            QAxFactory *f = factories.value(key); \
            return f ? f->interfaceID(key) : QUuid(); \
        } \
        QUuid eventsID(const QString &key) const override { \
            QAxFactory *f = factories.value(key); \
            return f ? f->eventsID(key) : QUuid(); \
        } \
        void registerClass(const QString &key, QSettings *s) const override { \
            if (QAxFactory *f = factories.value(key)) \
                f->registerClass(key, s); \
        } \
        void unregisterClass(const QString &key, QSettings *s) const override { \
            if (QAxFactory *f = factories.value(key)) \
                f->unregisterClass(key, s); \
        } \
        QString exposeToSuperClass(const QString &key) const override { \
            QAxFactory *f = factories.value(key); \
            return f ? f->exposeToSuperClass(key) : QString(); \
        } \
        bool stayTopLevel(const QString &key) const override { \
            QAxFactory *f = factories.value(key); \
            return f ? f->stayTopLevel(key) : false; \
        } \
        bool hasStockEvents(const QString &key) const override { \
            QAxFactory *f = factories.value(key); \
            return f ? f->hasStockEvents(key) : false; \
        } \
    }; \
    QAxFactory *qax_instantiate()               \
    {                                           \
        return new QAxFactoryList();            \
    } \
    QT_END_NAMESPACE

QT_END_NAMESPACE
QT_WARNING_POP

#ifndef Q_COM_METATYPE_DECLARED
#define Q_COM_METATYPE_DECLARED

Q_DECLARE_OPAQUE_POINTER(IUnknown*)
Q_DECLARE_OPAQUE_POINTER(IDispatch*)

Q_DECLARE_METATYPE(IUnknown*)
Q_DECLARE_METATYPE(IDispatch*)

#endif

#endif // QAXFACTORY_H
