// Copyright (C) 2016 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QQMLCHILDRENPRIVATE_H
#define QQMLCHILDRENPRIVATE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QAbstractState>
#include <QAbstractTransition>
#include <QStateMachine>
#include <QQmlInfo>
#include <QQmlListProperty>
#include <private/qglobal_p.h>

enum class ChildrenMode {
    None              = 0x0,
    State             = 0x1,
    Transition        = 0x2,
    StateOrTransition = State | Transition
};

template<typename T>
static T *parentObject(QQmlListProperty<QObject> *prop) { return static_cast<T *>(prop->object); }

template<class T, ChildrenMode Mode>
struct ParentHandler
{
    static bool unparentItem(QQmlListProperty<QObject> *prop, QObject *oldItem);
    static bool parentItem(QQmlListProperty<QObject> *prop, QObject *item);
};

template<class T>
struct ParentHandler<T, ChildrenMode::None>
{
    static bool unparentItem(QQmlListProperty<QObject> *, QObject *) { return true; }
    static bool parentItem(QQmlListProperty<QObject> *, QObject *) { return true; }
};

template<class T>
struct ParentHandler<T, ChildrenMode::State>
{
    static bool parentItem(QQmlListProperty<QObject> *prop, QObject *item)
    {
        if (QAbstractState *state = qobject_cast<QAbstractState *>(item)) {
            state->setParent(parentObject<T>(prop));
            return true;
        }
        return false;
    }

    static bool unparentItem(QQmlListProperty<QObject> *, QObject *oldItem)
    {
        if (QAbstractState *state = qobject_cast<QAbstractState *>(oldItem)) {
            state->setParent(nullptr);
            return true;
        }
        return false;
    }
};

template<class T>
struct ParentHandler<T, ChildrenMode::Transition>
{
    static bool parentItem(QQmlListProperty<QObject> *prop, QObject *item)
    {
        if (QAbstractTransition *trans = qobject_cast<QAbstractTransition *>(item)) {
            parentObject<T>(prop)->addTransition(trans);
            return true;
        }
        return false;
    }

    static bool unparentItem(QQmlListProperty<QObject> *prop, QObject *oldItem)
    {
        if (QAbstractTransition *trans = qobject_cast<QAbstractTransition *>(oldItem)) {
            parentObject<T>(prop)->removeTransition(trans);
            return true;
        }
        return false;
    }
};

template<class T>
struct ParentHandler<T, ChildrenMode::StateOrTransition>
{
    static bool parentItem(QQmlListProperty<QObject> *prop, QObject *oldItem)
    {
        return ParentHandler<T, ChildrenMode::State>::parentItem(prop, oldItem)
                || ParentHandler<T, ChildrenMode::Transition>::parentItem(prop, oldItem);
    }

    static bool unparentItem(QQmlListProperty<QObject> *prop, QObject *oldItem)
    {
        return ParentHandler<T, ChildrenMode::State>::unparentItem(prop, oldItem)
                || ParentHandler<T, ChildrenMode::Transition>::unparentItem(prop, oldItem);
    }
};

template <class T, ChildrenMode Mode>
class ChildrenPrivate
{
public:
    static void append(QQmlListProperty<QObject> *prop, QObject *item)
    {
        Handler::parentItem(prop, item);
        static_cast<Self *>(prop->data)->children.append(item);
        parentObject<T>(prop)->childrenContentChanged();
    }

    static qsizetype count(QQmlListProperty<QObject> *prop)
    {
        return static_cast<Self *>(prop->data)->children.size();
    }

    static QObject *at(QQmlListProperty<QObject> *prop, qsizetype index)
    {
        return static_cast<Self *>(prop->data)->children.at(index);
    }

    static void clear(QQmlListProperty<QObject> *prop)
    {
        auto &children = static_cast<Self *>(prop->data)->children;
        for (QObject *oldItem : std::as_const(children))
            Handler::unparentItem(prop, oldItem);

        children.clear();
        parentObject<T>(prop)->childrenContentChanged();
    }

    static void replace(QQmlListProperty<QObject> *prop, qsizetype index, QObject *item)
    {
        auto &children = static_cast<Self *>(prop->data)->children;

        Handler::unparentItem(prop, children.at(index));
        Handler::parentItem(prop, item);

        children.replace(index, item);
        parentObject<T>(prop)->childrenContentChanged();
    }

    static void removeLast(QQmlListProperty<QObject> *prop)
    {
        Handler::unparentItem(prop, static_cast<Self *>(prop->data)->children.takeLast());
        parentObject<T>(prop)->childrenContentChanged();
    }

private:
    using Self = ChildrenPrivate<T, Mode>;
    using Handler = ParentHandler<T, Mode>;

    QList<QObject *> children;
};

#endif
