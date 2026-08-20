// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qhttpserverrouter_p.h"

#include <QtHttpServer/qhttpserverrouter.h>
#include <QtHttpServer/qhttpserverrouterrule.h>
#include <QtHttpServer/qhttpserverrequest.h>
#include <QtHttpServer/qhttpserver.h>

#include <private/qhttpserverrouterrule_p.h>

#include <QtCore/qloggingcategory.h>
#include <QtCore/qmetatype.h>

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(lcRouter, "qt.httpserver.router")

using namespace Qt::StringLiterals;

/*!
    \internal
*/
static const QHash<QMetaType, QString> defaultConverters = {
    { QMetaType::fromType<int>(), u"[+-]?\\d+"_s },
    { QMetaType::fromType<long>(), u"[+-]?\\d+"_s },
    { QMetaType::fromType<long long>(), u"[+-]?\\d+"_s },
    { QMetaType::fromType<short>(), u"[+-]?\\d+"_s },

    { QMetaType::fromType<unsigned int>(), u"[+]?\\d+"_s },
    { QMetaType::fromType<unsigned long>(), u"[+]?\\d+"_s },
    { QMetaType::fromType<unsigned long long>(), u"[+]?\\d+"_s },
    { QMetaType::fromType<unsigned short>(), u"[+]?\\d+"_s },

    { QMetaType::fromType<double>(), u"[+-]?(?:[0-9]+(?:[.][0-9]*)?|[.][0-9]+)"_s },
    { QMetaType::fromType<float>(), u"[+-]?(?:[0-9]+(?:[.][0-9]*)?|[.][0-9]+)"_s },

    { QMetaType::fromType<QString>(), u"[^/]+"_s },
    { QMetaType::fromType<QByteArray>(), u"[^/]+"_s },

    { QMetaType::fromType<QUrl>(), u".*"_s },

    { QMetaType::fromType<void>(), u""_s },
};

/*!
    \class QHttpServerRouter
    \since 6.4
    \brief Provides functions to bind a path to a \c ViewHandler.
    \inmodule QtHttpServer

    QHttpServerRouter is a rule-based system for routing HTTP requests to
    their appropriate handlers. You can add QHttpServerRouterRule
    instances, which define a request path and its corresponding handler.

    Variable parts in the route can be specified with placeholders
    (\c{"<arg>"}) in the request path, but it is not needed at the end. The
    handler receives the matched values as a \l QRegularExpressionMatch. The
    arguments can be of any type for which a \l{converters}{converter} is
    available. The handler creation can be simplified with
    \l QHttpServerRouterRule::bindCaptured().


    \note A QHttpServerRouter instance must not be modifed by its rules.
    \note This is a low-level routing API for an HTTP server.

    Minimal example:

    \code
    auto pageView = [] (const quint64 page) {
        qDebug() << "page" << page;
    };
    using ViewHandler = decltype(pageView);

    QHttpServerRouter router;

    // register callback pageView on request "/page/<number>"
    // for example: "/page/10", "/page/15"
    router.addRule<ViewHandler>(
        new QHttpServerRouterRule("/page/", [=] (QRegularExpressionMatch &match,
                                                 const QHttpServerRequest &,
                                                 QHttpServerResponder &&) {
        auto boundView = QHttpServerRouterRule::bindCaptured(pageView, match);

        // it calls pageView
        boundView();
    }));
    \endcode
*/

/*! \fn template <typename Type> bool QHttpServerRouter::addConverter(QAnyStringView regexp)

    Adds a new converter for \e Type that can be parsed with \a regexp, and
    returns \c true if this was successful, otherwise returns \c false. If
    successful, the registered type can be used as argument in handlers for
    \l{QHttpServerRouterRule}. The regular expression will be used to parse
    the path pattern of the rule.

    If there is already a converter of type \e Type, that converter's regexp
    is replaced with \a regexp.

    Custom converters can extend the available type conversions through the
    \l QMetaType system.

    Define a class with a QString constructor:
    \snippet custom-converter/main.cpp Define class

    To use a custom type with the \l{QHttpServer}{HTTP server}, register
    it using this function and define a \l{QHttpServer::}{route} handler
    using the new type:
    \snippet custom-converter/main.cpp Add converter and route

    \sa converters, clearConverters
*/

/*! \fn template <typename ViewHandler, typename ViewTraits = QHttpServerRouterViewTraits<ViewHandler>> QHttpServerRouterRule *QHttpServerRouter::addRule(std::unique_ptr<QHttpServerRouterRule> rule)
    Adds a new \a rule to the router.

    Returns a pointer to the new rule if successful; otherwise returns
    \c nullptr.

    Inside addRule, we determine ViewHandler arguments and generate a list of
    their QMetaType::Type ids. Then we parse the path and replace each
    \c{"<arg>"} with a regexp for its type from the list.

    The provided \a rule must not modify the QHttpServerRouter instance.

    \code
    QHttpServerRouter router;

    using ViewHandler = decltype([] (const QString &page, const quint32 num) { });

    auto rule = std::make_unique<QHttpServerRouterRule>(
        "/<arg>/<arg>/log",
        [] (QRegularExpressionMatch &match,
            const QHttpServerRequest &request,
            QHttpServerResponder &&responder) {
    });

    router.addRule<ViewHandler>(std::move(rule));
    \endcode
*/

QHttpServerRouterPrivate::QHttpServerRouterPrivate(QAbstractHttpServer *server)
    : converters(defaultConverters), server(server)
{}

/*!
    Creates a QHttpServerRouter object with default converters.

    \sa converters
*/
QHttpServerRouter::QHttpServerRouter(QAbstractHttpServer *server)
    : d_ptr(new QHttpServerRouterPrivate(server))
{}

/*!
    Destroys a QHttpServerRouter.
*/
QHttpServerRouter::~QHttpServerRouter()
{}

/*!
    Adds a new converter for \a metaType that can be parsed with \a regexp.
    Having a converter for a \a metaType enables to use this type in a path
    pattern of a \l QHttpServerRouterRule. The regular expression is used to
    parse parameters of type \a metaType from the path pattern.

    If there is already a converter of type \a metaType, that converter's
    regexp is replaced with \a regexp.

    \sa converters, clearConverters
*/
void QHttpServerRouter::addConverter(QMetaType metaType, QAnyStringView regexp)
{
    Q_D(QHttpServerRouter);
    d->converters[metaType] = regexp.toString();
}

/*!
    Removes the converter for type \a metaType.

    \sa addConverter
*/
void QHttpServerRouter::removeConverter(QMetaType metaType)
{
    Q_D(QHttpServerRouter);
    d->converters.remove(metaType);
}

/*!
    Removes all converters.

    \note clearConverters() does not set up default converters.

    \sa converters, addConverter
*/
void QHttpServerRouter::clearConverters()
{
    Q_D(QHttpServerRouter);
    d->converters.clear();
}

/*!
    \fn const QHash<QMetaType, QString> &QHttpServerRouter::converters() const &
    \fn QHash<QMetaType, QString> QHttpServerRouter::converters() &&

    Returns a map of converter types and regular expressions that are registered
    with this QHttpServerRouter. These are the types that can be used in path
    patterns of \l{QHttpServerRouterRule}{QHttpServerRouterRules}.

    The following converters are available by default:

    \value QMetaType::Int
    \value QMetaType::Long
    \value QMetaType::LongLong
    \value QMetaType::Short
    \value QMetaType::UInt
    \value QMetaType::ULong
    \value QMetaType::ULongLong
    \value QMetaType::UShort
    \value QMetaType::Double
    \value QMetaType::Float
    \value QMetaType::QString
    \value QMetaType::QByteArray
    \value QMetaType::QUrl
    \value QMetaType::Void       An empty converter.

    \sa addConverter, clearConverters
*/
const QHash<QMetaType, QString> &QHttpServerRouter::converters() const &
{
    Q_D(const QHttpServerRouter);
    return d->converters;
}

QHash<QMetaType, QString> QHttpServerRouter::converters() &&
{
    Q_D(QHttpServerRouter);
    return std::move(d->converters);
}

QHttpServerRouterRule *QHttpServerRouter::addRuleImpl(std::unique_ptr<QHttpServerRouterRule> rule,
                                    std::initializer_list<QMetaType> metaTypes)
{
    Q_D(QHttpServerRouter);

    if (!rule->hasValidMethods() || !rule->createPathRegexp(metaTypes, d->converters)) {
        return nullptr;
    }
    if (!d->verifyThreadAffinity(rule->contextObject())) {
        return nullptr;
    }

    return d->rules.emplace_back(std::move(rule)).get();
}

/*!
    Handles each new \a request for the HTTP server using \a responder.

    Iterates through the list of rules to find the first that matches,
    then executes this rule, returning \c true. Returns \c false if no rule
    matches the request.
*/
bool QHttpServerRouter::handleRequest(const QHttpServerRequest &request,
                                      QHttpServerResponder &responder) const
{
    Q_D(const QHttpServerRouter);
    for (const auto &rule : d->rules) {
        if (!rule->contextObject())
            continue;
        if (!d->verifyThreadAffinity(rule->contextObject()))
            continue;
        if (rule->exec(request, responder))
            return true;
    }

    return false;
}

bool QHttpServerRouterPrivate::verifyThreadAffinity(const QObject *contextObject) const
{
    if (contextObject && (contextObject->thread() != server->thread())) {
        qCWarning(lcRouter, "QHttpServerRouter: the context object must reside in the same thread");
        return false;
    }
    return true;
}

QT_END_NAMESPACE
