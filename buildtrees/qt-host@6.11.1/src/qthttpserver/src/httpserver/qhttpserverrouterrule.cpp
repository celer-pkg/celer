// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include <QtHttpServer/qhttpserverrouterrule.h>
#include <QtHttpServer/qhttpserverresponder.h>

#include <private/qhttpserverrouterrule_p.h>
#include <private/qhttpserverrequest_p.h>

#include <QtCore/qhash.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qstringbuilder.h>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(lcRouterRule, "qt.httpserver.router.rule")

/*!
    \class QHttpServerRouterRule
    \since 6.4
    \brief The QHttpServerRouterRule is the base class for QHttpServerRouter rules.
    \inmodule QtHttpServer

    QHttpServerRouterRule defines the relationship between a request path, an
    HTTP method, and the corresponding handler callback. A QHttpServerRouter
    is a collection of these rules, executing the appropriate handler when a
    request matches both the path and method. The handler is responsible for
    generating the response.

    \section1 Paths and Patterns

    Each QHttpServerRouterRule includes a path or pattern that determines
    which requests it can handle. Paths may contain placeholders that
    are passed to the handler. The examples below illustrate path patterns
    using the QHttpServer::route() convenience method, though they can also
    be set using the QHttpServerRouterRule constructor.

    In the simplest case the path is a string with a leading \c "/":
    \code
    QHttpServer server;
    server.route("/user", [] () { return "hello user"; } );
    \endcode

    This path pattern defines a rule that directs all requests to \c "/user"
    to the specified handler, which in this case is a simple lambda function.
    (Note that when using QHttpServerRouterRule directly, the handler syntax
    differs—see below.)

    A trailing \c "/" in the path pattern allows the rule to match additional
    paths with arguments after the \c "/". When using the QHttpServer::route()
    convenience method, the argument is automatically passed to the lambda
    function:
    \code
    server.route("/user/", [] ( qint64 id ) { return "hello user"; } );
    \endcode
    This would match request paths such as \c "/user/1", \c "/user/2", and so
    on.

    \section2 Capturing Arguments in the Path

    You can place arguments anywhere in the path pattern using the \c "<arg>"
    placeholders, and multiple of them are supported in the path:
    \code
    server.route("/user/<arg>/history", [] (qint64 id){ return "hello user"; } );
    server.route("/user/<arg>/history/", [] (qint64 id, qint64 page){ return "hello user"; } );
    \endcode
    For example, this would match a request like \c "/user/1/history/2". Any
    data type registered in QHttpServerRouter::converters() can be used in
    both the callback function and the corresponding placeholders in
    the path.

    \section1 Request Method

    The request method corresponds to one of the values in
    QHttpServerRequest::Method. If no method is specified when constructing
    a rule, it will match requests of any known method.

    \section1 Handler Signature

    A handler is a callback function with the following signature:
    \code
    void (*)(const QRegularExpressionMatch &, const QHttpServerRequest &, QHttpServerResponder &);
    \endcode
    \list
        \li The first argument receives any matched capture groups from the
        path.
        \li The second argument contains request details.
        \li The third argument is used to send the response.
    \endlist

    \section1 Adding Rules to QHttpServerRouter

    The example below demonstrates how to create and register a new rule with
    a handler in \l QHttpServerRouter:
    \code
    template<typename ViewHandler>
    void route(const char *path, const QHttpServerRequest::Methods methods, ViewHandler &&viewHandler)
    {
        auto rule = std::make_unique<QHttpServerRouterRule>(
                path, methods, [this, viewHandler = std::forward<ViewHandler>(viewHandler)]
                                                (QRegularExpressionMatch &match,
                                                 const QHttpServerRequest &request,
                                                 QHttpServerResponder &responder) mutable {
            auto boundViewHandler = QHttpServerRouterRule::bindCaptured<ViewHandler>(
                    this, std::move(viewHandler), match);
            boundViewHandler(); // Execute the handler
        });

        // Add rule to the router
        router.addRule<ViewHandler>(std::move(rule));
    }

    // Valid:
    route("/user/", [] (qint64 id) { } );                            // Matches "/user/1", "/user/3", etc.
    route("/user/<arg>/history", [] (qint64 id) { } );               // Matches "/user/1/history", "/user/2/history"
    route("/user/<arg>/history/", [] (qint64 id, qint64 page) { } ); // Matches "/user/1/history/1", "/user/2/history/2"
    \endcode

    \note This is a low-level API. For higher-level alternatives, see
    \l QHttpServer.

    \note Regular expressions are not supported in path patterns, but you can
    use \l QHttpServerRouter::addConverter() to match \c "<arg>" to a
    specific type.
*/

/*! \fn template <typename Functor, typename ViewTraits = QHttpServerRouterViewTraits<Functor>> static typename ViewTraits::BindableType QHttpServerRouterRule::bindCaptured(QObject *receiver, Functor &&slot, const QRegularExpressionMatch &match) const

    Binds the given \a receiver and \a slot with arguments extracted from the
    URL. The function returns a bound callable that takes any remaining
    arguments required by the handler, supplying them to \a slot after the
    URL-derived values.

    Each captured value from the URL (as a string) is converted to the
    corresponding parameter type in the handler based on its position,
    ensuring it can be passed as \a match.

    \code
    QHttpServerRouter router;

    auto pageView = [] (const QString &page, const quint32 num) {
        qDebug("page: %s, num: %d", qPrintable(page), num);
    };
    using ViewHandler = decltype(pageView);

    auto rule = std::make_unique<QHttpServerRouterRule>(
        "/<arg>/<arg>/log",
        [&router, &pageView] (QRegularExpressionMatch &match,
                              const QHttpServerRequest &request,
                              QHttpServerResponder &&responder) {
        // Bind and call viewHandler with match's captured string and quint32:
        QHttpServerRouterRule::bindCaptured(pageView, match)();
    });

    router.addRule<ViewHandler>(std::move(rule));
    \endcode
*/

/*! \fn template <typename Functor> QHttpServerRouterRule::QHttpServerRouterRule(const QString &pathPattern, const QHttpServerRequest::Methods methods, const QObject *receiver, Functor &&slot)

    Creates a routing rule for \a pathPattern and \a methods, connecting it
    to the specified \a receiver and \a slot.
    \list
        \li The \a slot can be a function pointer, non-mutable lambda, or any
        other copyable callable with \c const call operator.
        \li If \a slot is callable, \a receiver acts as its context object.
        \li The handler remains valid until the \a receiver is destroyed.
    \endlist
    The rule can handle any combination of available HTTP methods.

    \sa QHttpServerRequest::Methods
*/

/*! \fn template <typename Functor> QHttpServerRouterRule::QHttpServerRouterRule(const QString &pathPattern, const QObject *receiver, Functor &&slot)

    \overload

    This overload constructs a routing rule for \a pathPattern and associates
    it with \a receiver and \a slot.
    \list
        \li It defaults to \l QHttpServerRequest::Method::AnyKnown, meaning
        it will match any recognized HTTP method.
        \li The \a slot can be a function pointer, non-mutable lambda, or any
        other copyable callable with \c const call operator.
        \li If \a slot is callable, \a receiver acts as its context object.
        \li The handler remains valid until the \a receiver is destroyed.
    \endlist
*/
QHttpServerRouterRule::QHttpServerRouterRule(const QString &pathPattern,
                                             const QHttpServerRequest::Methods methods,
                                             const QObject *context,
                                             QtPrivate::QSlotObjectBase *slotObjRaw)
    : QHttpServerRouterRule(new QHttpServerRouterRulePrivate{
              pathPattern, methods, QtPrivate::SlotObjUniquePtr(slotObjRaw), QPointer(context), {}})
{
    Q_ASSERT(slotObjRaw);
}

/*!
    \internal
*/
QHttpServerRouterRule::QHttpServerRouterRule(QHttpServerRouterRulePrivate *d)
    : d_ptr(d)
{
}

/*!
    Destroys a QHttpServerRouterRule.
*/
QHttpServerRouterRule::~QHttpServerRouterRule()
{
}

/*!
    Retrieves the context object associated with this rule. This object
    serves as the receiver responsible for handling the request.
*/
const QObject *QHttpServerRouterRule::contextObject() const
{
    Q_D(const QHttpServerRouterRule);
    return d->context;
}

/*!
    Validates the Request Method. Returns \c true if the specified HTTP
    method is valid.
*/
bool QHttpServerRouterRule::hasValidMethods() const
{
    Q_D(const QHttpServerRouterRule);
    return d->methods & QHttpServerRequest::Method::AnyKnown;
}

/*!
    Executes the rule. Processes the given \a request by checking if it
    matches this rule.
    \list
        \li This function is called by QHttpServerRouter when a new
        \a request is received.
        \li If the \a request matches the rule, it handles the request by
        sending a response through the provided \a responder and returns
        \c true.
        \li If there is no match, it returns \c false.
    \endlist
*/
bool QHttpServerRouterRule::exec(const QHttpServerRequest &request,
                                 QHttpServerResponder &responder) const
{
    Q_D(const QHttpServerRouterRule);
    if (!d->routerHandler)
        return false;

    QRegularExpressionMatch match;
    if (!matches(request, &match))
        return false;

    void *args[] = { nullptr, &match, const_cast<QHttpServerRequest *>(&request), &responder };
    Q_ASSERT(d->routerHandler);
    d->routerHandler->call(nullptr, args);
    return true;
}

/*!
    Determines whether the provided \a request meets the conditions of this
    rule.
    \list
        \li This virtual function is called by \l exec() to evaluate the
        \a request.
        \li If the request matches, the details are stored in \a match
        (which \e{must not} be \c nullptr), and the function returns \c true.
        \li Otherwise, it returns \c false.
    \endlist
*/
bool QHttpServerRouterRule::matches(const QHttpServerRequest &request,
                                    QRegularExpressionMatch *match) const
{
    Q_D(const QHttpServerRouterRule);

    if (d->methods && !(d->methods & request.method()))
        return false;

    *match = d->pathRegexp.match(request.url().path());
    return (match->hasMatch() && d->pathRegexp.captureCount() == match->lastCapturedIndex());
}

/*!
    \internal
*/
bool QHttpServerRouterRule::createPathRegexp(std::initializer_list<QMetaType> metaTypes,
                                             const QHash<QMetaType, QString> &converters)
{
    Q_D(QHttpServerRouterRule);

    QString pathRegexp = d->pathPattern;
    const QLatin1StringView arg("<arg>");
    for (auto metaType : metaTypes) {
        if (metaType.id() >= QMetaType::User
            && !QMetaType::hasRegisteredConverterFunction(QMetaType::fromType<QString>(), metaType)) {
            qCWarning(lcRouterRule,
                      "%s has not registered a converter to QString. "
                      "Use QHttpServerRouter::addConveter<Type>(converter).",
                      metaType.name());
            return false;
        }

        auto it = converters.constFind(metaType);
        if (it == converters.end()) {
            qCWarning(lcRouterRule, "Can not find converter for type: %s", metaType.name());
            return false;
        }

        if (it->isEmpty())
            continue;

        const auto index = pathRegexp.indexOf(arg);
        const QString &regexp = QLatin1Char('(') % *it % QLatin1Char(')');
        if (index == -1)
            pathRegexp.append(regexp);
        else
            pathRegexp.replace(index, arg.size(), regexp);
    }

    if (pathRegexp.indexOf(arg) != -1) {
        qCWarning(lcRouterRule) << "not enough types or one of the types is not supported, regexp:"
                                << pathRegexp
                                << ", pattern:" << d->pathPattern
                                << ", types:" << metaTypes;
        return false;
    }

    if (!pathRegexp.startsWith(QLatin1Char('^')))
        pathRegexp = QLatin1Char('^') % pathRegexp;
    if (!pathRegexp.endsWith(QLatin1Char('$')))
        pathRegexp += u'$';

    qCDebug(lcRouterRule) << "url pathRegexp:" << pathRegexp;

    d->pathRegexp.setPattern(pathRegexp);
    d->pathRegexp.optimize();
    return true;
}

QT_END_NAMESPACE
